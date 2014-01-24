doSSHLogin_AttachScreen () {
 ret=1
 if [[ $PS1 && $(</proc/$PPID/cmdline) =~ ^sshd ]] && which screen &>/dev/null; then
   screenSessMaxPID=$(screen -ls | sed -nr 's%^\s+([0-9]+)\..*\(Detached\)$%\1%p' | sort -n | tail -1) && \
    { screen -dr $screenSessMaxPID; ret=$?; }
 fi
 return $ret
}

getMySQLUseFiles () {
 find /proc -maxdepth 1 -mindepth 1 -exec bash -c '[[ {} =~ /proc/[0-9]+ ]]' \; -exec grep -q 'bin/mysql' {}/cmdline \; -print | \
  sed 's%^/proc/%%' | parallel lsof -p'{}' | awk '$9 ~ /^\// {print $9}' | sort | uniq
 return $?
}
