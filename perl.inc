PerlDcl_Arr () {
local ArrDef ArrName
declare -i errc=0
  (( $# )) || return 1
  while (( $# )); do
   ArrDef=$(declare -p $1) || { errc+=$?; continue; }
   ArrName=$(sed -r "s%^declare -[aA] %%; s%='\(.+$%%" <<<"$ArrDef")
   if [[ $ArrDef =~ ^declare\ -A ]]; then
    sed -r "s&^declare -A ${ArrName}='&%${ArrName}=&; s%\[\"?%'%g; s%\"?\]%'%g; s%=([^(])%=>\1%g" <<<"$ArrDef" | \
     sed -r -e s%\"\\s\'%\",\ \'%g -e s%\\\)\'$%\)\;\ %g
   else
    sed -r "s%^declare -a ${ArrName}='\(%\\@${ArrName}=();\n%; s%\)'$%%; s%(\[[0-9]+\]=\"[^\"]+\")\s*%\$${ArrName}\1;\n%g " <<<"$ArrDef"
   fi
   shift
  done
  return $errc
}

PerlSrch_Pkg () {
  local -a lstPkgNames lstPerlINC
  local arg rxPkgNames
  
  [[ $@ ]] || return 1
  while (($#)); do
    arg=$1; shift
    if [[ $arg == '--' ]]; then
      break
    else
      lstPkgNames+=("${arg//::/\/}")
    fi
  done
  rxPkgNames="("$(declare -p lstPkgNames | sed -r -e 's%^.+\[0\]="%[0]="%' -e 's%\s*\[[0-9]+\]="([^"]+)"%\1|%g' -e "s%\)'$%%" -e 's%\|$%%')")"
  
  readarray -t lstPerlINC < <(perl -e 'print join(qq|\n|  => grep { !/^\.$/ and -d $_ } @INC),"\n"')
  
  find "${lstPerlINC[@]}" \
  	-maxdepth 2 \
  	-type f \
  	-regextype posix-extended -regex ".*/${rxPkgNames}\.pm" \
  	$((($#)) && echo "-exec $@ {} ;" || echo '')
}
