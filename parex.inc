set +H
declare -i CPU_KERNELS=$(fgrep processor /proc/cpuinfo | wc -l)
declare -i MAX_TASKS=${MAX_TASKS:-$CPU_KERNELS}

FL_CLEVER_TASK=0
declare -A TASKS_QUEUE=()

unset wait4_cpu_free push_task

rotate_tq () {
 declare -i taskPID flTaskGone=0 
 for taskPID in ${!TASKS_QUEUE[@]}; do
  [[ -f /proc/$taskPID/cmdline ]] 2>/dev/null || {
   flTaskGone=1
   unset TASKS_QUEUE[$taskPID]
  }
 done
 return $flTaskGone 
}

wait4_cpu_free () {
 if (( FL_CLEVER_TASK )); then
  wait ${!TASKS_QUEUE[@]}
 else
  while :; do
   rotate_tq || break
   sleep 0.01
  done
 fi
 return 0
}

push_task () {
 local taskPID pthInclude flUseEval=0 flCleverTask=0
 while [[ $@ ]]; do
  case $1 in
   -e) flUseEval=1 ;;
   -c) flCleverTask=1 ;;
   -f) shift; pthInclude="$1"
       [[ -r $pthInclude ]] || { echo 'Invalid include file' >&2; return 101; }
    ;;
   *) echo 'Unknown parameter specified' >&2; return 1 ;;
  esac
  shift
 done
# echo "PID_QUEUE=${!TASKS_QUEUE[@]}"

 if (( ${#TASKS_QUEUE[@]} )); then  
  rotate_tq  
  if (( ${#TASKS_QUEUE[@]} >= MAX_TASKS )); then
   if (( flCleverTask )); then
    wait
   else
    wait4_cpu_free
   fi
  fi
 fi
 if (( flUseEval )); then
  eval "$(cat -)" &
 elif [[ $pthInclude ]]; then
  source "$pthInclude" &
 else  
  source <(cat -) &
 fi
 taskPID=$!
 [[ -f /proc/$taskPID/cmdline ]] && \
  TASKS_QUEUE[$taskPID]="$(</proc/$taskPID/cmdline)"
 return $?
}

wait4_all_gone () {
 declare -i taskPID errc=0
 (( ${#TASKS_QUEUE[@]} )) || return 0
 for taskPID in ${!TASKS_QUEUE[@]}; do
  wait $taskPID
  errc+=$?
  unset TASKS_QUEUE[$taskPID]
 done
 return $errc
}
