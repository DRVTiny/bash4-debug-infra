shopt -s extglob
declare -f zt &>/dev/null || \
 source /opt/scripts/functions/strings.inc

YYYY () {
 local year=$(printf '%02g' $1)
 echo $(( ${#year}==2?20${year}:${year} ))
}

dinv () {
  local dblock date1 date2 y YY YYYY m MM d DD
  declare -a dblocks
  declare -a dmy
  
  IFS='; ' read -a dblocks <<<"$1"
  local sep="${2:0:1}"

  for dblock in ${dblocks[@]}; do 
   IFS='.' read -a dmy <<<"$dblock"
   if [[ ${#dmy[@]} -gt 3 && $dblock =~ \- ]]; then
    date1="$(YYYY ${dmy[2]%%-*})/${dmy[1]}/${dmy[0]}"
    date2="$(YYYY ${dmy[4]})/${dmy[3]}/${dmy[2]##*-}"

    date1=$(date -d "$date1" +%s) || {
     echo 'Invalid first date in interval!' >&2
     return 1
    }
     
    date2=$(date -d "$date2" +%s) || {
     echo 'Invalid last date in interval!' >&2
     return 1
    }
    
    if (( date1 > date2 )); then
     m=$date1
     date1=$date2
     date2=$m
    fi
    for ((d=date1; d<=date2; d+=86400)); do
     date -d "@${d}" +"%Y${sep}%m${sep}%d" || return 1
    done 
   else
    for ((y=$(zt ${dmy[2]%%-*}); y<=$(zt ${dmy[2]##*-}); y++)); do
     year=$(YYYY $y)
     for ((m=$(zt ${dmy[1]%%-*}); m<=$(zt ${dmy[1]##*-}); m++)); do
      month=$(zp 2 $m)
      for ((d=$(zt ${dmy[0]%%-*}); d<=$(zt ${dmy[0]##*-}); d++)); do
       day=$(zp 2 $d)
       echo "${year}${sep}${month}${sep}${day}"
      done
     done
    done
   fi
  done
}
