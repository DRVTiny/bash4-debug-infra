PerlDcl_Hash () {
  [[ $1 ]] || return 1
  declare -p $1 | \
   sed -r "s&declare -A ([^=]+)='&%\1=&; s%\[\"?%'%g; s%\"?\]%'%g; s%=([^(])%=>\1%g" | \
    sed -r -e s%\"\\s\'%\",\ \'%g -e s%\\\)\'$%\)\;\ %g
  return ${PIPESTATUS[0]}
}
