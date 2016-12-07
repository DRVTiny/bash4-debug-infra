#!/bin/bash
#
# (C) by Andrey A. Konovalov, NSPK, 2016
#
[[ $1 == '-x' ]] && { set -x; export TRACE=1; }

declare -A err2msg=(
	['ERR_INVALID_OPTION']='Invalid option passed to me'
	['ERR_TMP_DIR_FAIL']='Failed to create and use temporary directory'
	['ERR_WEB_CON_REFUSED']='Connection to web-server was refused'
	['ERR_CANT_GET_CRL']='Cant download CRL'
	['ERR_CANT_GET_CA_CERT']='Failed to download CA certificate'
	['ERR_CMN_SSL_ERR']='OpenSSL-related issue'
	['ERR_ZBX_SND_FAIL']='Zabbix sender failed'
)

source '../erc_handle.inc'

# Defaults ->
# Default (fallback) CA certificate file. If the valid path to the CA cert should be found in the downloaded server certificate, this variable will be replaced.
pemCACert='/opt/Zabbix/x509/sca.cer'
agentConf='/etc/zabbix/zabbix_agentd.conf'
# <- Defaults

#source /etc/zabbix/zabbix_agentd.conf
err_ () {
	echo "ERROR: $0: @$(date +%s): $@" >&2
}

declare -A host=(
	['CertName']='support.nspk.ru'
	['ZbxHost']='msk1-vm-redmine01.unix.nspk.ru'
)

declare -A opts=(
	['s']='host["CertName"]'
	['z']='host["ZbxHost"]'
	['x']='flTrace'
	['D']='flDontRemoveTmp'
)

source '..//getopts_helper.inc'

[[ $flTrace && !$TRACE ]] && { set -x; export TRACE=1; }
[[ $keys_used =~ [sz]  ]] && {
	[[ ${keys_used/s/} == $keys_used ]]; flUseZbxH=$?
	[[ ${keys_used/z/} == $keys_used ]]; flUseCertH=$?
	if ! [[ $flUseZbxH -ne 0 && $flUseCertH -ne 0 ]]; then
		if [[ $flUseZbxH -eq 0 ]]; then
			host['CertName']=${host[ZbxHost]}
		else
			host['ZbxHost']=${host[CertName]}
		fi
	fi
}

hook_on_exit () {
	local erc=$1 msg=$2
	
	if (( erc )); then
	# Dont try to use zabbix_sender if we are here because of its failure!
		(( (erc&255) == ERR_ZBX_SND_FAIL )) && return 0
		zabbix_sender -c "$agentConf" -vv -s ${host[ZbxHost]} -i - <<ITEMS
- check.cert.script_status $erc
- check.cert.last_err_txt "$msg"
ITEMS
	else
		zabbix_sender -c "$agentConf" -vv -s ${host[ZbxHost]} -o 0 -k 'check.cert.script_status'
	fi
	return 0
}

httpsHost=${host[CertName]}
pemCert="${httpsHost}_cert.pem"
derCRL="${httpsHost}_crl.der"
pemCRL=${derCRL/.der/.pem}

tmpDir=$(mktemp -d /tmp/XXXXXXXXXX)
echo "Using temporary directory: $tmpDir" >&2
try $ERR_TMP_DIR_FAIL cd "$tmpDir" 4>/dev/null

[[ $flDontRemoveTmp ]] || \
	cleanOnExit "$tmpDir"
	
set -o pipefail
httpsPort=$(getent services https | sed -nr 's%^.*\s([0-9]+).*$%\1%p')
zbxHost=${host[ZbxHost]}
try -e '[[ $retc>0 && $retc<100 ]]' $ERR_WEB_CON_REFUSED timeout 0.1 telnet $httpsHost $httpsPort 4>/dev/null
#if [[ $?>0 && $?<100 ]]; then
#	err_ "Cant connect to $httpsHost:$httpsPort, connection refused"
#	exit $ERR_CONNECT_REFUSED
#fi
openssl s_client -showcerts -servername $httpsHost -connect $httpsHost:$httpsPort 2>/dev/null | \
	openssl x509 -inform pem -text > "$pemCert"
urlCRL=$(sed -nr '/CRL Distribution Points:/,${ s%^\s+URI:(http:.+)$%\1%p; T l; q; :l }' "$pemCert")
urlCACrt=$(sed -nr 's%^\s*CA Issuers.+URI:(http:.+)\s*$%\1%pI; T l; q; :l' "$pemCert")
if [[ $? == 0 && $urlCACrt ]]; then
	derCACrt=${urlCACrt##*/}
	try $ERR_CANT_GET_CA_CERT timeout 4 wget -q "$urlCACrt" -O "$derCACrt"
	if [[ $derCACrt =~ \.pem$ ]]; then
		pemCACrt=$derCACrt
	else
		pemCACrt="${derCACrt%.*}.pem"
		try $ERR_CMN_SSL_ERR openssl x509 -in "$derCACrt" -inform DER -out "$pemCACrt"
	fi
fi
try $ERR_CANT_GET_CRL timeout 4 wget -q "$urlCRL" -O "$derCRL"
try $ERR_CMN_SSL_ERR openssl crl -in "$derCRL" -inform DER -out "$pemCRL"

openssl verify -CAfile "$pemCACrt" -CRLfile "$pemCRL" "$pemCert"
checkCRLStatus=$?

tsCertValidTill=$(date -d "$(sed -nr 's%^\s+Not After\s+:\s+%%p' $pemCert)" +%s)
secsBeforeCertExpire=$((tsCertValidTill-$(date +%s)))

try $ERR_ZBX_SND_FAIL zabbix_sender -c '/etc/zabbix/zabbix_agentd.conf' -vv -s ${host[ZbxHost]} -i - <<ITEMS
- check.cert.time_before_expiration $secsBeforeCertExpire
- check.cert.revoke_status $checkCRLStatus
ITEMS
