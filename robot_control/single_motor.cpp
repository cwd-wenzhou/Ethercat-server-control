/*
 * @Author: your name
 * @Date: 2021-12-11 16:03:54
 * @LastEditTime: 2021-12-16 19:57:35
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /py2cpp/home/lqx/src/sdk_c/single_motor.cpp
 */
#include <stdio.h>
#include "fiber_head.h"

int main(int argc, char **argv)
{

    struct MOTOR fiber_motor[FIBER_MOTOR_NUM];
    init_EtherCAT_fiber_master(fiber_motor);
    printf("*It's working now*\n");

    for (int i = 0; i < 6; i++)
    {
        fiber_motor[i].targetPosition = 0;
        fiber_motor[i].opModeSet = 8; //位置模式
        fiber_motor[i].homeBusy = true;
        fiber_motor[i].powerBusy = true;
    }
    enable_fiber_motor(fiber_motor);
    int count = 0;
    long total = 0;
    printf("电机当前速度 当前位置 当前转距 目标位置\n");
    while (count < 10)
    {
        count++;
        //total+=SinglePosition(fiber_motor,0,ENCODER_RESOLUTION,200);
        //total+=SinglePosition(fiber_motor,0,0,200);

        total += SinglePosition_planded(fiber_motor, 2, 5*ENCODER_RESOLUTION, 500, 500);
        total += SinglePosition_planded(fiber_motor, 2, 0, 500, 500);
    }
    printf("total stepnum%ld\n", total);

    //SinglePosition(fiber_motor,0,0,100);
    //SinglePosition(fiber_motor,0,0,100);
    //SinglePosition(fiber_motor,0,0,100);
    ecrt_master_deactivate(fiber_motor[0].master);
}
