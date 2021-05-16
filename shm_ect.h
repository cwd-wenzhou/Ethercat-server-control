/*
 * @Author       : cwd
 * @Date         : 2021-5-13
 * @Place  : hust
 */
#include <unistd.h>
#include <stdint.h>
#include "log/log.h"
#include "header.h"

// struct shm_ect_data {		/* struct stored in shared memory */

//     int32_t targetPosition;   //电机的目标位置
//     int32_t targetVelocity;  //电机当前运行速度
//     int8_t opModeSet;         //电机运行模式的设定值,默认位置模式
//     int8_t opmode;            //驱动器当前运行模式
//     int32_t currentVelocity;  //电机当前运行速度
//     int32_t currentPosition;  //电机当前位置
//     uint16_t status;          //驱动器状态字

//     bool powerBusy;       //使能标志位
//     bool resetBusy;       //复位标志位
//     bool quickStopBusy;   //急停标志位
//     bool homeBusy;        //回零标志位
//     bool positionMoving;  //位置模式下运动
// };

#define SHM_NAME "ethercat_shm"
#define SHM_MODE  0666 //means that all users can read and write but cannot execute the file or floder

//print all the info in motor struct
void Print_All(struct MOTOR & shm){
    printf("targetPosition= %d\ntargetVelocity= %d \nopModeSet= %d\nopmode=%d\ncurrentVelocit=%d\ncurrentPosition=%d\nstatus=%d\npowerBusy=%d\nresetBusy=%d\nquickStopBusy=%d\nhomeBusy=%d\npositionMoving=%d\n",
    shm.targetPosition,shm.targetVelocity,shm.opModeSet,shm.opmode,shm.currentVelocity,shm.currentPosition,shm.status,shm.powerBusy,shm.resetBusy,shm.quickStopBusy,
    shm.homeBusy,shm.positionMoving);
}

//Log all the info in motor struct
void Log_Info_All(struct MOTOR & shm){
    Log_Info("\ntargetPosition= %d\ntargetVelocity= %d \nopModeSet= %d\nopmode=%d\ncurrentVelocit=%d\ncurrentPosition=%d\nstatus=%d\npowerBusy=%d\nresetBusy=%d\nquickStopBusy=%d\nhomeBusy=%d\npositionMoving=%d\n",
    shm.targetPosition,shm.targetVelocity,shm.opModeSet,shm.opmode,shm.currentVelocity,shm.currentPosition,shm.status,shm.powerBusy,shm.resetBusy,shm.quickStopBusy,
    shm.homeBusy,shm.positionMoving);
}