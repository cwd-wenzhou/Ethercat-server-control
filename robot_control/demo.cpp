/*
 * @Author: your name
 * @Date: 2021-12-14 19:06:55
 * @LastEditTime: 2021-12-17 15:57:35
 * @LastEditors: Please set LastEditors
 * @FilePath: /sdk_c/home/lqx/src/code/demo.cpp
 * g++ demo.cpp -o demo  /opt/etherlab/lib/libethercat.a
 */
#include <iostream>
#include <stdio.h>
#include <string.h> //strerror
#include "fiber_head.h"
using namespace std;
int main(int argc, char **argv)
{
    int cpus = 0;
    cpu_set_t mask;
    cpu_set_t get;

    cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("cpus: %d\n", cpus);

    CPU_ZERO(&mask); /* 初始化set集，将set置为空*/
    /*将本进程绑定到CPU2上*/
    CPU_SET(2, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1)
    {
        printf("Set CPU affinity failue, ERROR:%s\n", strerror(errno));
        return -1;
    }
    else
        printf("绑定成功");
    
    //初始化电机参数
    struct MOTOR fiber_motor[FIBER_MOTOR_NUM];
    for (int i = 0; i < 6; i++)
    {
        fiber_motor[i].targetPosition = 0;
        fiber_motor[i].opModeSet = 8; //位置模式
        fiber_motor[i].homeBusy = true;
        fiber_motor[i].powerBusy = true;
    }
    int num;
    while (1)
    {

        cout << "0: 动轴0" << endl;
        cout << "1: 动轴1" << endl;
        cout << "2: 动轴2" << endl;
        cout << "3: 动轴3" << endl;
        cout << "4: 动轴4" << endl;
        cout << "5: 动轴5" << endl;
        //10～15为线性规划后的
        cout << "10: 线性规划后的动轴0" << endl;
        cout << "11: 线性规划后的动轴1" << endl;
        cout << "12: 线性规划后的动轴2" << endl;
        cout << "13: 线性规划后的动轴3" << endl;
        cout << "14: 线性规划后的动轴4" << endl;
        cout << "15: 线性规划后的动轴5" << endl;
        cout << "100:全部动" << endl;
        cout << "200: 线性规划后的全部动" << endl;
        cout << "1000:读取所有电机当前位置值 " << endl;
        cin >> num;
        switch (num)
        {
        case 1000:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            for (int i = 0; i < FIBER_MOTOR_NUM; i++)
            {
                fiber_motor[i].powerBusy = true;
            }
            enable_fiber_motor(fiber_motor);
            Position(fiber_motor, 0, 0, 0, 0, 0, 0, 0);
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
            //ecrt_master_deactivate(fiber_motor[0].master);
        }
        break;
        case 0:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[0].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            SinglePosition(fiber_motor, 0, 0, 50);
            SinglePosition(fiber_motor, 0, 100000, 50);
            cout << "轴0运动完成" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 1:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[1].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            SinglePosition(fiber_motor, 1, 0, 50);
            SinglePosition(fiber_motor, 1, 100000, 50);
            cout << "轴1运动完成" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 2:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[2].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            SinglePosition(fiber_motor, 2, 0, 50);
            SinglePosition(fiber_motor, 2, 100000, 50);
            cout << "轴2运动完成" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 3:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[3].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            SinglePosition(fiber_motor, 3, 0, 50);
            SinglePosition(fiber_motor, 3, 100000, 50);
            cout << "轴3运动完成" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 4:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[4].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            SinglePosition(fiber_motor, 4, 0, 50);
            SinglePosition(fiber_motor, 4, 100000, 50);
            cout << "轴4运动完成" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 5:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[5].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            SinglePosition(fiber_motor, 5, 0, 50);
            SinglePosition(fiber_motor, 5, 100000, 50);
            cout << "轴5运动完成" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;

        case 10:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[0].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            //SinglePosition(fiber_motor,1,0,50);
            SinglePosition_planded(fiber_motor, 0, 0, 500);
            //SinglePosition_planded(fiber_motor, 0, 100000, 500);
            cout << "电机0运动结束" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 11:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[1].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            //SinglePosition(fiber_motor,1,0,50);
            SinglePosition_planded(fiber_motor, 1, 0, 500);
            SinglePosition_planded(fiber_motor, 1, 100000, 500);
            cout << "电机1运动结束" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 12:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[2].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            //SinglePosition(fiber_motor,1,0,50);
            SinglePosition_planded(fiber_motor, 2, 40000, 500);
            //SinglePosition_planded(fiber_motor, 2, 100000, 500);
            cout << "电机2运动结束" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 13:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[3].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            //SinglePosition(fiber_motor,1,0,50);
            SinglePosition_planded(fiber_motor, 3,-20000, 500);
            //SinglePosition_planded(fiber_motor, 3, 100000, 500);
            cout << "电机3运动结束" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 14:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[4].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            //SinglePosition(fiber_motor,1,0,50);
            SinglePosition_planded(fiber_motor, 4, 0, 500);
            //SinglePosition_planded(fiber_motor, 4, 300000, 500);
            cout << "电机4运动结束" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 15:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            fiber_motor[5].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            //SinglePosition(fiber_motor,1,0,50);
            SinglePosition_planded(fiber_motor, 5, 90000, 500);
            //SinglePosition_planded(fiber_motor, 5, 100000, 500);
            cout << "电机5运动结束" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 100:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            for (int i = 0; i < 6; i++)
                fiber_motor[i].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            Position(fiber_motor, 0, 0, 0, 0, 0, 0, 50);
            //Position(fiber_motor, 100000, 100000, 100000, 100000, 100000, 100000, 50);

            cout << "所有电机运动" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;
        case 200:
        {
            init_EtherCAT_fiber_master(fiber_motor);
            for (int i = 0; i < 6; i++)
                fiber_motor[i].powerBusy = true;
            enable_fiber_motor(fiber_motor);
            Position_planed(fiber_motor, +80000, 90000, 0, 70000, 70000, 100000, 500);
            Position_planed(fiber_motor, 500000, 500000, 500000, 500000, 500000, 500000, 500);

            cout << "所有规划的电机运动" << endl;
            cout << endl;
            ecrt_release_master(fiber_motor[0].master);
        }
        break;

        default:
            break;
        }
    }
    // enable_fiber_motor(fiber_motor);

    // Position(fiber_motor, 30000, 30000, 30000, 30000, 30000, 30000, 5);
    // Position_planed(fiber_motor, 0, 0, 0, 0, 0, 0, 5);

    // ecrt_master_deactivate(fiber_motor[0].master);
    // usleep(1000000);
    // //SinglePosition(fiber_motor,3,100000,5);
    // //SinglePosition(fiber_motor,0,0,100);
    // //SinglePosition(fiber_motor,0,0,100);
    // //SinglePosition(fiber_motor,0,0,100);
    // ecrt_release_master(fiber_motor[0].master);
}
