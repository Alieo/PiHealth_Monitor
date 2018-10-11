#!/bin/bash
#恶意进程
#执行ps, 如果找到有cpu%或mem%大于50, 暂停5s, 在重新执行一次ps, 如果这个大于50的数仍存在, 把它的进程名, PID, 用户, CPU%, MEM%输出出来
#如果没找到有大于50的, 则退出脚本
Timenow=`date +"%Y-%m-%d__%H:%M:%S"`
Mem=`ps -aux -h | awk '{ if ($3 > 50 || $4 > 50) {printf("%d", $2);} }'`
if [[ $Mem ]];then
    sleep 5
else exit 0
fi
Process=`ps -aux -h | awk -v time=$Timenow '{if ($3 > 50 || $4 > 50){ printf("%s %s %d %s %d%% %d%%"), time, $11, $2, $1, $3, $4}}'`
#输出: 时间  进程名  PID  用户  CPU%  MEM%
echo $Process
