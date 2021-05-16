#include "header.h"
#include "init.h"

int main(int argc, char **argv) 
{
    struct MOTOR motor; 
    if (argc<2){
        printf("put in velocity(rad/s)\n");
        return 1;
    }
    int velocity_=atoi(argv[1]);
    
    printf("velocity=%d\n",velocity_);
    printf("*It's working now*\n");
    motor.powerBusy=1;
    motor.opModeSet = 9;         //位置模式
    init_EtherCAT_master(&motor);
    int count = 1;

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
 
        Status_Check(motor.status,&motor.driveState);//状态机模式将state置到operationEnable
        
        State_Machine(&motor);//使能状态机    

        Velocity(&motor,velocity_);
        count++;
        if (count>100){
            count=0;
            Status_Check_printf(motor.status);
            printf("电机当前速度%d||||当前位置%d\n",motor.currentVelocity,motor.currentPosition);
        }

        Homing(&motor);
        //发送过程数据
        ecrt_domain_queue(motor.domain);
        ecrt_master_send(motor.master);
    }
    
    ecrt_master_deactivate(motor.master);
}