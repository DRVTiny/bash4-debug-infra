doSSHLogin_AttachScreen () {
 ret=1
 if [[ $PS1 && $(</proc/$PPID/cmdline) =~ ^sshd ]] && which screen &>/dev/null; then
   screenSessMaxPID=$(screen -ls | sed -nr 's%^\s+([0-9]+)\..*\(Detached\)$%\1%p' | sort -n | tail -1) && \
    { screen -dr $screenSessMaxPID; ret=$?; }
 fi
 return $ret
}
