/*
 * @Author       : cwd
 * @Date         : 2021-5-16
 * @Place  : hust
 * 基于共享内存，作为ethercat主站
 *      1。给电机使能
 *      2。循环读取电机的过程数据
 *      3。将过程数据写道共享内存里，把共享内存里的数据发给从站
 * 
 */
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>              /* For O_* constants */
#include <unistd.h>
#include "shm_ect.h"
#include "log/log.h"
#include "sig.h"
#include "motor.h"
#include "init.h"
#include "ecmk.h"
using namespace std;

//#define LOG
;
bool running;
void close_sig_handler(int sig){
    running = false;
}

int main(int argc, char const *argv[])
{
    /*
    addsig(SIGINT,close_sig_handler);//ctrl+c的函数设置

    //共享内存初始化设置
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT ,SHM_MODE);
    motor = (struct MOTOR* )mmap(NULL,1024*sizeof(struct MOTOR),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
    ftruncate(shmfd,1024*sizeof(struct MOTOR));
    close(shmfd);
    */

    //若开启log，那么开始记录
    #ifdef LOG    
        //log的初始化设置
        int logLevel = 0;
        int logQueSize =5;
        Log::Instance()->Init(logLevel,"./logfile","server_control.log",logQueSize);
        Log_Info("========== Server init ==========");
        Log_Info("LogSys level: %d", logLevel);
        Log::Instance()->flush();
        int count = 0;      //LOG计数用,避免打出太多log
    #endif
    
    running = true;
    printf("*It's working now*\n");


    //创建ethercat主站master
    ec_master_t * master = Ecrt_request_master(0);
    ec_master_info_t *master_info= new ec_master_info_t;
    printf("ecrt_request_master done\n");
    ecrt_master(master,master_info);//获取主站信息
    int slave_count = master_info->slave_count;//从站个数
    printf("there are %d slaves\n",slave_count);
    vector<SLAVE*> slaves(slave_count,nullptr);

    init_EtherCAT_slaves(master,slaves);
    printf("ethercat master init successed!\n");
    int powerup=false;
    while (running){
        usleep(1000);
        //接收过程数据
        ecrt_master_receive(master);
        for (int i=0;i< slave_count;i++){
            //接收过程数据
            ecrt_domain_process(slaves[i]->domain);

            //检查过程数据状态（可选）
            check_domain_state(slaves[i]->domain, &slaves[i]->domain_state);
            
            //检查从站配置状态
            check_slave_config_states(slaves[i]->slave, &slaves[i]->slave_state);

            //slaves[i]->read_data();

            #ifdef LOG
                count++;
                if (count>1000){
                    count=0;
                    Log_Info(Status_Check_char(motor->status));
                    Log_Info_All(*motor);

                }
                if (Is_Serevr_On(motor) && !powerup)
                {
                    powerup = true;
                    Log_Info("========== Server Power up ==========")
                }
            #endif

            //slaves[i]->send_data();
            
            //发送过程数据
            ecrt_domain_queue(slaves[i]->domain);
        }
        ecrt_master_send(master);
    }
    
    //ecrt_master_deactivate(motor->master);
    ecrt_release_master(master);
    printf("========== SERVER CLOSED ==========\n");
    #ifdef LOG
        Log_Info("========== SERVER CLOSED ==========");
    #endif
    return 0;
}



  