#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>              /* For O_* constants */
#include <unistd.h>
#include "shm_ect.h"

using namespace std;
int main(int argc, char const *argv[])
{
    struct shm_ect_data  *ect_data_ptr_;
    int shmfd = shm_open(SHM_NAME, O_RDWR  ,SHM_MODE);
    ect_data_ptr_ = (struct shm_ect_data* )mmap(NULL,sizeof(struct shm_ect_data),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
    close(shmfd);

    for (;;){
        sleep(1);
        cin>>ect_data_ptr_->targetPosition>>ect_data_ptr_->targetVelocity;
    }
    return 0;
}
