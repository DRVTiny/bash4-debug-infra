opt_dscr=':'
for opt_k in ${!opts[@]}; do
        opt_dscr+=${opt_k}
        v=${opts[${opt_k}]}
        [[ ${v:0:2} == 'fl' || ( ${v:0:1} == '&' && ${v/\$OPTARG/} == $v ) ]] || opt_dscr+=':'
        [[ $v =~ ^[^\&].+= ]] && eval "$v"
done
keys_used=''
while getopts $opt_dscr k; do
        if [[ ! ${opts[$k]} ]]; then
                if [[ $k == '?' ]]; then
                        err_ "No such option '-$k'"
                else
                        err_ "'-$k' not implemented yet"
                fi
                exit 1
        fi
        v=${opts[$k]}
        if [[ ${v:0:1} == '&' ]]; then
                eval "${v:1}"
        elif [[ $opt_dscr =~ ${k}: ]]; then
                eval "${v%%=*}=\$OPTARG"
        else
                eval "${v%%=*}=1"
        fi
        keys_used+=$k
done
