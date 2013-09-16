#!/bin/bash
pg_grant_all () {
local flExecute flDebug
 while [[ ${1:0:1} == '-' ]]; do
  case ${1:1} in
    e) flExecute=1 ;;
    d) shift; DBNAME="$1" ;;
    u) shift; USER="$1" ;;
    x) flDebug=1; set -x ;;
    *) return 1 ;;
  esac
  shift
 done
 [[ $DBNAME && $USER ]] || return 1

 psql <<<'\t\l' | egrep -q "^[[:space:]]*$DBNAME[[:space:]]*|" || return 1

 if [[ $flExecute ]]; then
  cmd="psql $DBNAME"
 else
  cmd='cat -'
 fi 

 {
 for SCHEMA in $(psql $DBNAME <<<'\dn' | sed -nr -e '1,3d' -e 's%^\s*(\w+)\s*\|.*$%\1%p'); do
  for TABLE in $(psql $DBNAME <<<"SELECT relname FROM pg_stat_all_tables WHERE schemaname='$SCHEMA';" | fgrep -v 'pg_' | grep '^ '); do
   $cmd <<<"GRANT SELECT,UPDATE,DELETE,INSERT ON TABLE $SCHEMA.$TABLE to $USER;"
  done
  for GRANT in $(psql $DBNAME <<<"select n.nspname||'.'||c.relname from pg_class c, pg_namespace n where n.oid = c.relnamespace and c.relkind in ('S') and n.nspname in ('$SCHEMA');" | grep "^ $SCHEMA"); do
   $cmd <<<"GRANT ALL ON $GRANT TO $USER";
  done
 done
 }
 local ret_=$?
 [[ $flDebug ]] && set +x
 return ${ret_}
}
