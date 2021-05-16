/*
 * @Author       : cwd
 * @Date         : 2021-5-16
 * @Place  : hust
 * 基于共享内存，作为应用层，读共享内存里的ethercat过程数据，然后基于业务逻辑，操作数据并写入。
 * 现在业务逻辑只实现了position一种，其他尚待完善。
 * 调用方法：
 *      ./share_men_cli  2000
 *  其中2000表示期望运动到的位置。
 */
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>              /* For O_* constants */
#include <unistd.h>
#include "shm_ect.h"
#include "header.h"
using namespace std;
int main(int argc, char const *argv[])
{
    struct MOTOR  *motor;
    int shmfd = shm_open(SHM_NAME, O_RDWR  ,SHM_MODE);
    motor = (struct MOTOR* )mmap(NULL,sizeof(struct MOTOR),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
    close(shmfd);
    //printf("that't all\n");
    if (argc<2){
        printf("put in position(0~%d)\n",ENCODER_RESOLUTION);
        return 1;
    }
    int position_=atoi(argv[1]);
    printf("*It's working now*\n");


    while (true){
        usleep(1000);
        if (!Is_Serevr_On)                     
            continue;
        Position(motor,position_);
    }
    return 0;
}
