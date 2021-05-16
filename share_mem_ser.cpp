#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>              /* For O_* constants */
#include <unistd.h>
#include "shm_ect.h"
#include "log/log.h"
#include "sig.h"
#include "header.h"
#include "init.h"
using namespace std;

struct MOTOR  *motor;
void close_sig_handler(int sig){
    
    ecrt_master_deactivate(motor->master);
    printf("========== SERVER CLOSED ==========\n");
    Log_Info("========== SERVER CLOSED ==========");
    exit(0);
}

int main(int argc, char const *argv[])
{
    addsig(SIGINT,close_sig_handler);//ctrl+c的函数设置

    //共享内存初始化设置
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT ,SHM_MODE);
    motor = (struct MOTOR* )mmap(NULL,sizeof(struct MOTOR),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
    ftruncate(shmfd,sizeof(struct MOTOR));
    close(shmfd);

    //log的初始化设置
    int openLog = 1;
    int logLevel = 0;
    int logQueSize =5;
    //若开启log，那么开始记录
    if (openLog){
            Log::Instance()->Init(logLevel,"./logfile","server_control.log",logQueSize);
            Log_Info("========== Server init ==========");
            Log_Info("LogSys level: %d", logLevel);
    }
    Log::Instance()->flush();

    printf("*It's working now*\n");
    motor->powerBusy=1;
    motor->opModeSet = 8;         //位置模式
    init_EtherCAT_master(motor);

    int count = 0;      //控制台计数用
    int powerup=false;
    while (true){
        usleep(1000);
        //接收过程数据
        ecrt_master_receive(motor->master);
        ecrt_domain_process(motor->domain);

        //检查过程数据状态（可选）
        check_domain_state(motor->domain, &motor->domain_state);
        //检查主站状态
        check_master_state(motor->master, &motor->master_state);
        //检查从站配置状态
        check_slave_config_states(motor->maxsine_EP3E, &motor->maxsine_EP3E_state);
        
        //读取数据
        motor->status =
            EC_READ_U16(motor->domain_pd + motor->drive_variables.status_word);  //状态字
        motor->opmode =
            EC_READ_U8(motor->domain_pd + motor->drive_variables.mode_display);  //运行模式
        motor->currentVelocity = EC_READ_S32(
            motor->domain_pd + motor->drive_variables.current_velocity);  //当前速度
        motor->currentPosition = EC_READ_S32(
            motor->domain_pd + motor->drive_variables.current_postion);  //当前位置

        Status_Check(motor->status,&motor->driveState);//状态机模式将state置到operationEnable
        
        State_Machine(motor);//使能状态机   

        count++;
        if (count>1000){
            count=0;
            Status_Check_printf(motor->status);
            Log_Info_All(*motor);
        }
        if (Is_Serevr_On(motor) && !powerup)
        {
            powerup = true;
            Log_Info("========== Server Power up ==========")
        }
        if (Is_Serevr_On(motor))
        {
            EC_WRITE_S32(motor->domain_pd + motor->drive_variables.target_postion,
                motor->targetPosition);

            EC_WRITE_S32(motor->domain_pd + motor->drive_variables.target_velocity,
                motor->targetVelocity);

        }
        
        //发送过程数据
        ecrt_domain_queue(motor->domain);
        ecrt_master_send(motor->master);
    }
    
    
    return 0;
}


