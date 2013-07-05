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
