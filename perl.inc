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
