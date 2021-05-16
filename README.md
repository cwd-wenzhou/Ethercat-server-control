EtherCAT 主站IGH
控制伺服电机的代码


开启主站命令
sudo /etc/init.d/ethercat start

成功开启则如下：
imc@es1:~$ sudo /etc/init.d/ethercat start
[sudo] password for imc: 
Starting EtherCAT master 1.5.2  done


编译语句
gcc -O2  xxxx.c   /opt/etherlab/lib/libethercat.a -lrt -lm  -lpthread -D_GNU_SOURCE -o xxx


empty.c
空程序，并不传输数据，只从ethercat读取数据，并进行周期性通讯使得ethercat 从站进入op.
每隔一秒在控制台输出一次当前位置和速度。

operation.c
将状态置到operationenabled

homing.c
电机归零

position.c
位置模式

velocity.c
速度模式.

ethercat slaves
查看op状态









