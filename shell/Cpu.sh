#!/bin/bash
#cpu信息获取
Time_now=`date +"%Y-%m-%d__%H:%M:%S"`
#负载
Loadavg=`cat /proc/loadavg | awk '{printf $1" "$2" "$3}'`
#cpu占用率
a=`cat /proc/stat`
arr=($a)
sum1=0
idle1=`echo $a | head -n 1  | awk '{print $5}'`
for i in `seq 1 10`;do
    (( sum1 += ${arr[i]} ))
done

sleep 0.5 

b=`cat /proc/stat`
arrb=($b)
sum2=0
idle2=`echo $b | head -n 1  | awk '{print $5}'`
for i in `seq 1 10`;do
    (( sum2 += ${arrb[i]} ))
done
rate=`echo  | awk -v sum2=$sum2 -v sum1=$sum1 -v idle2=$idle2 -v idle1=$idle1 '{printf("%.2f", 100 - (idle2 - idle1) * 100.0 / (sum2 - sum1))}'`
#(( total = $sum2 - $sum1 ))
#(( idle = $idle2 - $idle1 ))
#(( rate = ($total - $idle) * 10000 / $total ))

#当前温度
Temp_num=`cat /sys/class/thermal/thermal_zone0/temp`
Temp=`echo $Temp_num | awk '{printf("%.2f", $1 * 1.0 / 1000)}'`
#cpu温度报警级别
Temp_level=`echo $Temp_num | awk '{t = $1 / 1000; if (t >= 0 && t < 50) printf("normal"); else if (t >= 50 && t < 70) printf("note"); else printf("warning");}'`

#输出：时间  负载  cpu占用率  当前cpu温度  cpu温度警报级别
echo "$Time_now  $Loadavg  $rate  $Temp℃   $Temp_level" >> ./log/Cpu.log



