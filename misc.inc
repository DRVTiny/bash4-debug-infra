doSSHLogin_AttachScreen () {
 ret=1
 if [[ $PS1 && $(</proc/$PPID/cmdline) =~ ^sshd ]] && which screen &>/dev/null; then
   screenSessMaxPID=$(screen -ls | sed -nr 's%^\s+([0-9]+)\..*\(Detached\)$%\1%p' | sort -n | tail -1)
   screen ${screenSessMaxPID:+-dr $screenSessMaxPID}
   ret=$?
 fi
 return $ret
}


getMySQLUseFiles () {
 find /proc -maxdepth 1 -mindepth 1 -exec bash -c '[[ {} =~ /proc/[0-9]+ ]]' \; -exec grep -q 'bin/mysql' {}/cmdline \; -print | \
  sed 's%^/proc/%%' | parallel lsof -p'{}' | awk '$9 ~ /^\// {print $9}' | sort | uniq
 return $?
}

showProcInfo () {
local PROC_NAME=$1

local pid key i
local -a arr   
local -A procInfo uid2name
 shopt -s extglob
 while IFS=':' read -a arr; do
  uid2name[${arr[2]}]=${arr[0]}
 done < <(getent passwd)
 while read pid; do
  while IFS=':'$'\x09' read -a arr; do
   arr[1]=${arr[1]##+([[:space:]])}   
   if (( ${#arr[@]}>2 )); then
    key=${arr[0],,}    
    for ((i=1; i<${#arr[@]}; i++)); do
     procInfo["${key}__${i}"]=${arr[$i]}
    done   
   else    
    procInfo[${arr[0],,}]=${arr[1]}
   fi
  done </proc/$pid/status
  printf "[pid=%s,ppid=%s(%s),user=%s] %s\n" \
            $pid \
            ${procInfo[ppid]} \
            "$(</proc/${procInfo[ppid]}/comm)" \
            "${uid2name[${procInfo[uid__1]}]}" \
            "$(xargs -0 </proc/$pid/cmdline)"   
 done < <(pgrep $PROC_NAME)
}
