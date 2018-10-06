#!/bin/bash
#获取本机内存信息
Time_now=`date +"%Y-%m-%d_%H:%M:%S"` 

#内存总量（$1）
#内存剩余量（$1-$2）= 内存总量($1) - 内存使用量($2)
#当前内存占用量($2 * 100 / $1) = 内存使用量($2) / 内存总量($1)
#平均值(a * 0.3 + 0.7 * b) = 上一次的内存占用量(a) * 0.3 + 0.7 * 当前内存占用量($2 * 100 / $1)
mem_information=`free -m | tr -s -c "0-9" " " | awk -v a=$1 '{printf("%dM\t%dM\t%d%%\t%d%%"), $1, $1-$2, $2*100/$1, a*0.3+0.7*($2*100/$1)}'`
#因为free是实时性的，所以只能free一次

#(( last = $total - $used )) #内存剩余量 = 内存总量 - 内存使用量 
#(( Occupy_now = $used * 100 / $total )) #当前内存占用量 = 内存使用量 / 内存总量 
#Dynamic_average=`echo | awk -v a=$1 -v b=$Occupy_now '{printf a * 0.3 + 0.7 * b}'` # 平均值

#输出：时间  内存总量  内存剩余量  当前内存占用量  平均值
echo -e "$Time_now\t$mem_information" >> ./log/Mem.log

