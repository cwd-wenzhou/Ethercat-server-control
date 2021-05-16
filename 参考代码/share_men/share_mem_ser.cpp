#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>              /* For O_* constants */
#include <unistd.h>
#include "shm_ect.h"
#include "log/log.h"
#include "sig.h"
using namespace std;
int main(int argc, char const *argv[])
{
    int openLog = 1;
    int logLevel = 0;
    int logQueSize =5;
    addsig(SIGINT,close_sig_handler);
    struct shm_ect_data  *ect_data_ptr_;
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT ,SHM_MODE);
    ect_data_ptr_ = (struct shm_ect_data* )mmap(NULL,sizeof(struct shm_ect_data),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
    ftruncate(shmfd,sizeof(struct shm_ect_data));
    close(shmfd);

    //若开启log，那么开始记录
    if (openLog){
            Log::Instance()->Init(logLevel,"./logfile","server_control.log",5);
            Log_Info("========== Server init ==========");
            Log_Info("LogSys level: %d", logLevel);
    }
    Log::Instance()->flush();
    for (;;){
        //sleep(1);
        //Log_Info_All(*ect_data_ptr_);
        ect_data_ptr_->targetPosition++;
        if (ect_data_ptr_->targetPosition>1000)
            ect_data_ptr_->targetPosition=0;
    }
    return 0;
}


