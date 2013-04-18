set +H
declare -i CPU_KERNELS=$(fgrep processor /proc/cpuinfo | wc -l)
declare -i MAX_TASKS=${MAX_TASKS-$CPU_KERNELS}

declare -A TASKS_QUEUE=()

unset wait4_cpu_free push_task

rotate_tq () {
 declare -i taskPID flTaskGone=0 
 for taskPID in ${!TASKS_QUEUE[@]}; do
  [[ -f /proc/$taskPID/cmdline && ${TASKS_QUEUE[$taskPID]} == "$(</proc/$taskPID/cmdline)" ]] 2>/dev/null || {
   flTaskGone=1
   unset TASKS_QUEUE[$taskPID]
  }
 done
 return $flTaskGone 
}

wait4_cpu_free () {
 while :; do
  rotate_tq || break
  sleep 0.01
 done
 return 0
}

push_task () {
 local taskPID
# echo "PID_QUEUE=${!TASKS_QUEUE[@]}"

 if (( ${#TASKS_QUEUE[@]} )); then  
  rotate_tq  
  (( ${#TASKS_QUEUE[@]} >= MAX_TASKS )) && wait4_cpu_free
 fi

 eval "$(cat -)" &
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
