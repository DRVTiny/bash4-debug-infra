rpad () {
 (( $# == 2 )) || return 1
 local s="$1" pad="$2" 
 if (( ${#pad}<=${#s} )); then
  echo "${s:0:${#pad}}"
 else
  echo "${s}${pad:${#s}:$((${#pad}-${#s}))}"
 fi
 return 0
}

zt () {
 echo ${1##*(0)}
}

zp () {
 printf "%0${1}g" $2
}

lpop () {
 (( $#<2 || $#>3 )) && return 1
 local lst="$1" ITEM="$2" sep="${3:- }"
 [[ $lst =~ (^|.+${sep})$ITEM(${sep}.+|$) ]] && \
  echo -n "${BASH_REMATCH[1]%$sep}${BASH_REMATCH[1]:+${BASH_REMATCH[2]:+$sep}}${BASH_REMATCH[2]#$sep}" || \
  echo -n "$lst"
 return 0
}

lpush () {
 (( $#<2 || $#>3 )) && return 1 
 local lst="$1" ITEM="$2" sep="${3:- }" 
 lst+="${lst:+$sep}$ITEM"
 echo -n "$lst"
}

join2 () {
 (( $# == 2 || $# == 3 )) || return 1
 local delim=$1
 local arrName=$2
 local join2var=$3
 local v
 for v in ${join2var:+join2var} arrName; do
  [[ ${!v} =~ ^[_0-9A-Za-z]+$ ]] || return 2
 done
 [[ $join2var ]] || join2var='v'
 source <(
        cat <<SOURCE
         printf -v $join2var "%s${delim}" "\${$arrName[@]}" || return \$?
         [[ \$delim ]] && $join2var=\${$join2var:0:-${#delim}}
SOURCE
 )
 (( $# == 2 )) && echo "$v"
 return 0
}

split2 () {
  local arrName=$1
  [[ $arrName =~ ^[0-9_A-Za-z]+$ ]] || return 1
  local delim=$2
  shift 2
  local args
  while (( $# )); do
   args+=${args:+$delim}$1
   shift
  done  
  readarray -t $arrName < <(echo -n "${args//$delim/$'\x0a'}")
  return $?
}
