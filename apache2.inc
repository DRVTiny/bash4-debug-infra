get_apache2_port () {
 local portN=$(fgrep -rH NameVirtualHost /etc/apache2 2>/dev/null | head -1 | sed -nr 's%^.*:[^#]*NameVirtualHost\s+(\*|[0-9.]+):([0-9]+).*$%\2%p')
 echo -n "$portN"
 [[ $portN ]]
 return $?
}

get_apache2_user () {
 local portN=${1:-$(get_apache2_port)} 
 which lsof &>/dev/null || return 1
 local _lsof="$(lsof -iTCP:${portN} 2>/dev/null)"
 if [[ ${_lsof} ]]; then
  local www_user=$(awk 'NR != 1 && $3 != "root" {print $3; exit; }' <<<"${_lsof}")
  [[ ${www_user=root} ]]
 elif [[ -f /etc/apache2/envvars ]]; then
  eval "$(sed -nr '/RUN_(USER|GROUP)/p' /etc/apache2/envvars)"
  www_user=$APACHE_RUN_USER
 fi
 echo -n "$www_user"
 [[ $www_user ]]
 return $?
}
