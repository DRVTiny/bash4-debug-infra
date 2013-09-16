PerlDcl_Hash () {
declare -i errc=0
  (( $# )) || return 1
  while (( $# )); do
   declare -p $1 | \
    sed -r "s&declare -A ([^=]+)='&%\1=&; s%\[\"?%'%g; s%\"?\]%'%g; s%=([^(])%=>\1%g" | \
     sed -r -e s%\"\\s\'%\",\ \'%g -e s%\\\)\'$%\)\;\ %g
   errc+=${PIPESTATUS[0]}
   shift
  done
  return $errc
}
