#!/bin/bash
#系统运行概况
Time_now=`date +"%Y-%m-%d__%H:%M:%S"`

#主机名
pi_name=`uname -n` 

#OS版本
Os_version=`cat /etc/issue | awk '{print $1"_"$2"_"$3}'`

#内核版本
Kernel_version=`uname -r`

#运行时间
Run_time=`uptime -p | tr " " "_"`

#平均负载
load_average=`cat /proc/loadavg | awk '{print $1" "$2" "$3}'`

#磁盘总量、磁盘已用%
Disk_infm=` df -m | grep "^/dev/" | awk '{a+=$2;b+=$3}END{printf("%dM %d%%", a, b * 100 / a)}'`

#内存大小、内存已用%
Mem_all=`free -m | tr -s -c "0-9" " "` #查看关于内存的各个参数
Mem_infm=`echo $Mem_all | awk '{printf("%dM %d%%", $1, $2*100/$1)}'`

#cpu温度
Temp_num=`cat /sys/class/thermal/thermal_zone0/temp`
Temp=`echo $Temp_num | awk '{printf("%.2f", $1 / 1000)}'`

#磁盘已用报警级别
Disk_level=`df -m | grep "^/dev/" | awk '{a += $2; b += $3; c = b*100/a;} END {if (c >= 0 && c < 80) printf ("normal "); else if (c >= 80 && c < 90) printf("note "); else printf("warning ");}'`

#内存已用警报级别
Mem_level=`echo $Mem_all | awk '{a = $2*100/$1; if (a >= 0 && a < 70) printf("normal "); else if (a >= 70 && a < 80) printf("note "); else printf("warning ")}'`

#cpu温度报警级别
Temp_level=`echo $Temp_num | awk '{t = $1 / 1000; if (t >= 0 && t < 50) printf("normal"); else if (t >= 50 && t < 70) printf("note"); else printf("warning");}'`

#输出：时间 主机名 os版本 内核版本 运行时间 平均负载 磁盘总量 磁盘已用% 内存大小 内存已用%  cpu温度 磁盘警报级别 内存警报级别  cpu温度警报级别
echo "$Time_now $pi_name $Os_version $Kernel_version $Run_time $load_average $Disk_infm $Mem_infm $Temp $Disk_level $Mem_level $Temp_level"
