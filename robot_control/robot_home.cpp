/*
 * @Author: your name
 * @Date: 2021-12-14 19:06:55
 * @LastEditTime: 2021-12-17 15:55:55
 * @LastEditors: Please set LastEditors
 * @FilePath: /sdk_c/home/lqx/src/code/demo.cpp
 * g++ robot_home.cpp -o robothome  /opt/etherlab/lib/libethercat.a
 */
#include <iostream>
#include <stdio.h>
#include <string.h> //strerror
#include "fiber_head.h"
using namespace std;
int main(int argc, char **argv)
{
    // int cpus = 0;
    // cpu_set_t mask;
    // cpu_set_t get;

    // cpus = sysconf(_SC_NPROCESSORS_ONLN);
    // printf("cpus: %d\n", cpus);

    // CPU_ZERO(&mask); /* 初始化set集，将set置为空*/
    // /*将本进程绑定到CPU2上*/
    // CPU_SET(2, &mask);
    // if (sched_setaffinity(0, sizeof(mask), &mask) == -1)
    // {
    //     printf("Set CPU affinity failue, ERROR:%s\n", strerror(errno));
    //     return -1;
    // }
    // else
    //     printf("绑定成功");

    //初始化电机参数
    struct MOTOR fiber_motor[FIBER_MOTOR_NUM];
    for (int i = 0; i < 6; i++)
    {
        fiber_motor[i].targetPosition = 0;
        fiber_motor[i].opModeSet = 8; //位置模式
        fiber_motor[i].homeBusy = true;
        fiber_motor[i].powerBusy = true;
    }
    init_EtherCAT_fiber_master(fiber_motor);
    for (int i = 0; i < 6; i++)
        fiber_motor[i].powerBusy = true;
    enable_fiber_motor(fiber_motor);
    Position_planed(fiber_motor, +80000, 90000, 0, 70000, 70000, 100000, 500);
    //Position_planed(fiber_motor, 200000, 200000, 200000, 200000, 200000, 200000, 500);

    cout << "所有规划的电机运动" << endl;
    cout << endl;
    ecrt_release_master(fiber_motor[0].master);
}
