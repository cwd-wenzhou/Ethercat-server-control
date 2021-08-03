#include "header.h"
#include "init.h"

int main(int argc, char **argv) 
{
    struct MOTOR motor; 
    printf("*It's working now*\n");
    init_EtherCAT_master(&motor);

    int count = 0;      //控制台计数用
    while (true){
        usleep(1000);
        //接收过程数据
        ecrt_master_receive(motor.master);
        ecrt_domain_process(motor.domain);

        //检查过程数据状态（可选）
        check_domain_state(motor.domain, &motor.domain_state);
        //检查主站状态
        check_master_state(motor.master, &motor.master_state);
        //检查从站配置状态
        check_slave_config_states(motor.maxsine_EP3E, &motor.maxsine_EP3E_state);
        
        //读取数据
        motor.status =
            EC_READ_U16(motor.domain_pd + motor.drive_variables.status_word);  //状态字
        motor.opmode =
            EC_READ_U8(motor.domain_pd + motor.drive_variables.mode_display);  //运行模式
        motor.currentVelocity = EC_READ_S32(
            motor.domain_pd + motor.drive_variables.current_velocity);  //当前速度
        motor.currentPosition = EC_READ_S32(
            motor.domain_pd + motor.drive_variables.current_postion);  //当前位置

        count++;
        if (count>1000){
            count=0;
            Status_Check_printf(motor.status);
            printf("电机当前速度%d||||当前位置%d\n",motor.currentVelocity,motor.currentPosition);
        }
        
        //发送过程数据
        ecrt_domain_queue(motor.domain);
        ecrt_master_send(motor.master);
    }
    
    ecrt_master_deactivate(motor.master);
}