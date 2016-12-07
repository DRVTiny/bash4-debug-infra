err2msg_def=$(declare -p err2msg 2>/dev/null) || exit 101
[[ ${err2msg_def:0:10} == 'declare -A' ]] || exit 102
unset err2msg_def
declare -a lstFiles2Clean
declare -a erc2msg
c=0
for erc in ${!err2msg[@]}; do
	msg=${err2msg[$erc]}
	if [[ $msg =~ ^([0-9]+)::(.+)$ ]]; then
		ern=${BASH_REMATCH[1]}
		msg=${BASH_REMATCH[2]}
		err2msg[$erc]=$msg
	else
		ern=$((++c))
	fi
	eval "$erc=$ern"
	erc2msg[$ern]=$msg
done

exitHandler () {
	local exitCode=${1:-$?}
	local softRetC=$(( exitCode>>8 ))

	[[ $(caller 0 | cut -d' ' -f2) == 'exitHandler' ]] && \
		exit $exitCode
	if (( ${exitCode:=0} )); then
          exitCode=$(( exitCode&255 ))
          local msg=${erc2msg[$exitCode]}
          if [[ $msg ]]; then
	    local ert=$(printf '[%s]: ERROR: %s' "$(date +'%Y-%m-%d %H:%M:%S')" "$msg")
            (( softRetC )) && ert+="; Return code: $softRetC"
	    echo "$ert" >&2
          fi
	  declare -F hook_on_exit >/dev/null && {
		hook_on_exit $(( exitCode | (softRetC<<8) )) "$msg"
	  }
	else
	  hook_on_exit 0
	fi
	if (( ${#lstFiles2Clean[@]} )); then
          local i
          for ((i=0; i<${#lstFiles2Clean[@]}; i++)); do
                  rm -rf "${lstFiles2Clean[i]}"
          done
	fi
	exit $exitCode
}

cleanOnExit () {
	while (( $# )); do
		lstFiles2Clean+=($1)
		shift
	done
}

exec 4<&2
try () {
	local o eval_retc flDontUseHook
	while [[ ${1:0:1} == '-' ]]; do
		case $1 in
			-e) shift; eval_retc=$1 ;;
			-H) flDontUseHook=1 ;;
			*) break ;;
		esac
		shift
	done
	local ercIfFail=$1; shift

	source <(cat <<"EOSOURCE"
$@	
EOSOURCE) 2>&4

	local -i retc=$? flIsErr=0
	if [[ $eval_retc ]]; then
		! eval "$eval_retc"
		flIsErr=$?
	else
		flIsErr=$retc
	fi
	(( flIsErr )) && \
		exitHandler $(( ercIfFail | (retc<<8) ))

	return 0
}

trap 'exitHandler' INT TERM EXIT
