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
