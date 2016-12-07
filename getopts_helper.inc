opt_dscr=':'
for opt_k in ${!opts[@]}; do
	opt_dscr+=${opt_k}
	[[ ${opts[${opt_k}]:0:2} == 'fl' ]] || \
		opt_dscr+=':'
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
	if [[ $opt_dscr =~ ${k}: ]]; then
		eval "${opts[$k]}=\$OPTARG"
	else
		eval "${opts[$k]}=1"
	fi
	keys_used+=$k
done