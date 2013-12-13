declare -a DEBLIKE=(LinuxMint Ubuntu Debian UnitedLinux)
declare -a RHLIKE=(CentOS RHEL RHAS RedHat Fedora)
declare -A APACHE_CD=(['Debian']='/etc/apache2' ['RedHat']='/etc/httpd')
[[ $APACHE_CONF_DIR && -d $APACHE_CONF_DIR ]] || {
 if [[ -f /etc/redhat-release ]]; then
  distrType='Debian'
 elif [[ -f /etc/debian_version ]]; then
  distrType='RedHat'
 elif which lsb_release &>/dev/null; then
  rxDebLike="^($(echo ${DEBLIKE[@]} | tr ' ' '|'))\$"
  rxRHLike="^($(echo ${RHLIKE[@]} | tr ' ' '|'))\$"
  distrID=$(lsb_release -is)
  if [[ $distrID =~ $rxDebLike ]]; then
   distrType='Debian'
  elif [[ $distrID =~ $rxRHLike ]]; then
   distrType='RedHat'
  fi
 fi
 if [[ $distrType =~ ^(Debian|RedHat)$ ]]; then
  APACHE_CONF_DIR=${APACHE_CD[$distrType]}
 else
  for d in ${APACHE_CD[@]}; do
   [[ -d $d ]] && {
    APACHE_CONF_DIR=$d
    break
   }
  done
 fi
}

[[ $APACHE_CONF_DIR && -d $APACHE_CONF_DIR ]] || {
 echo 'Cant determine where Apache2 config directory is located. Failed when trying to source apache2 functions' >&2
 exit 1
}

get_apache2_port () {
 local portN=$(fgrep -rH NameVirtualHost $APACHE_CONF_DIR 2>/dev/null | head -1 | sed -nr 's%^.*:[^#]*NameVirtualHost\s+(\*|[0-9.]+):([0-9]+).*$%\2%p')
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
 elif [[ -f $APACHE_CONF_DIR/envvars ]]; then
  eval "$(sed -nr '/RUN_(USER|GROUP)/p' $APACHE_CONF_DIR/envvars)"
  www_user=$APACHE_RUN_USER
 fi
 echo -n "$www_user"
 [[ $www_user ]]
 return $?
}
