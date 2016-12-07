#!/bin/bash
[[ $1 == '-x' ]] && { set -x; export TRACE=1; }
declare -A err2msg=(
	['ERR_INVALID_FILE']='Invalid input file specified'
	['ERR_DL_TIMEOUT']='Download aborted: timeout'
	['ERR_DL_INVALID_URL']='Cant proceed download: invalid URL'
	['ERR_SSL_SRV_CERT_EXPIRED']='Server certificate expired'
	['ERR_INT_BY_KBRD']='130::Interrupted by keyboard'
)

source 'bash4-debug-infra/erc_handle.inc'

mkdir -p a/b/c/d
touch a/b/c/d/1
cleanOnExit a

hook_on_exit () {
	local erc=$1 msg=$2
	echo 'hook_on_exit()' >&2
	caller 0
	printf 'local error code: %s\nsoft error code: %s\nerror message: %s\n' $(( erc&255 )) $(( erc>>8 )) "$msg" >&2
}

try $ERR_DL_INVALID_URL wget http://yyy1.ru 4>/dev/null
#try -e '[[ $retc>1 ]]' $ERR_DL_INVALID_URL wget http://yyy1.ru 4>/dev/null

#while :; do
#	sleep 0.1
#done

