declare -A slf=(
		[NAME]="${0##*/}"
		[PATH]="${0%/*}"
		[REAL_PATH]=$(readlink -e "$0")
)

slf[REAL_NAME]=$(basename "${slf[REAL_PATH]}")
slf[NAME_NO_EXT]=${slf[NAME]%%.*}
slf[REAL_NAME_NO_EXT]=${slf[REAL_NAME]%%.*}
