/*
 * @Author       : cwd
 * @Date         : 2021-5-16
 * @Place  : hust
 * 基于共享内存，作为应用层，读共享内存里的ethercat过程数据，然后基于业务逻辑，操作数据并写入。
 * 现在业务逻辑只实现了position一种，其他尚待完善。
 * 调用方法：
 *      ./share_men_cli  <command> <arg>
 * -h 回零
 * -p <arg> 位置模式，运动到arg处
 * -v <arg> 速度模式，以arg(rad/s)速度运行
 * --todo
 * 
 * 例如
 *  ./share_men_cli -h 0//表示命令电机回零
 *  ./share_men_cli -p 1000  //表示命令电机运动到位置 1000
 *  其中2000表示期望运动到的位置。
 */
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>              /* For O_* constants */
#include <unistd.h>
#include "shm_ect.h"
#include "header.h"
#include "server.h"
using namespace std;
int main(int argc, char *argv[])
{
    struct MOTOR  *motor;
    int shmfd = shm_open(SHM_NAME, O_RDWR  ,SHM_MODE);
    motor = (struct MOTOR* )mmap(NULL,sizeof(struct MOTOR),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
    close(shmfd);

    if (argc<2){
        printf("./share_men_cli  <command> <arg>");
        return 1;
    }

    printf("waiting server on\n");
    while (!Is_Serevr_On){}
    printf("*It's working now*\n");

    int c = getopt(argc,argv,"h:p:v:");
    int position_,velocity_;
    switch (c)
    {
    case 'h':
        Homing(motor);
        break;
    case 'p':
        if (argc<3)
        {
            printf("put in position(0~%d)\n",ENCODER_RESOLUTION);
            break;
        }
        position_=atoi(argv[2]);    
        Position(motor,position_);
        break;
    case 'v':
        if (argc<3)
        {
            printf("put in velocity(rad/s)\n");
            break;
        }
        velocity_=atoi(argv[2]); 
        Velocity(motor,velocity_);
        break;
    default:
        printf("./share_men_cli  <command> <arg>\n");
        break;
    }

    printf("done\n");
    return 0;
}
