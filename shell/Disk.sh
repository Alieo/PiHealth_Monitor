#!/bin/bash
#磁盘信息获取
Time_now=`date +"%Y-%m-%d__%H:%M:%S"`

#标识（0表示整个磁盘，1表示分区）

#for循环输出分区
IFS=$'\n' #让for循环只以\n作为分隔符
for i in `df -m | grep "^/dev/"`;do 
    #echo $i
    message1=`echo $i | awk '{printf("1\t%s\t%dM\t%dM\t%d%%\n", $6, $2, $4, $3 * 100 / $2 )}'`
    echo "$Time_now  $message1"
done

#输出整个磁盘信息
message0=`df -m | grep "^/dev/" | awk '{a+=$2;b+=$4;c+=$3}END{printf("0\tDisk\t%dM\t%dM\t%d%%\n", a, b, c * 100 / a) }'`
echo "$Time_now  $message0" >> ./log/Disk.log
