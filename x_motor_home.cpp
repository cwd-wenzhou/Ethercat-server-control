#include <stdio.h>
#include <stdlib.h>
#include <ecrt.h>
#include <math.h>
#include <unistd.h>
#include "motor.h"

#define SERVER_NUM 1
// ethercat主站应用程序所用的参数
#define TASK_FREQUENCY 4000        //*Hz* 任务周期
#define ENCODER_RESOLUTION 131072  //编码器分辨率
#define HOME_VECOLITY 5            // r/s，回零速度
#define HOME_STEP HOME_VECOLITY *ENCODER_RESOLUTION / TASK_FREQUENCY  // pulse 回零步长
#define POSITION_STEP 1 / TASK_FREQUENCY  //位置模式下步长
#define Pi 3.141592654                    //圆周率

//从站配置所用的参数
#define EP3ESLAVEPOS 0               //迈信伺服EP3E在ethercat总线上的位置
#define MAXSINE 0x000007DD, 0x00000001  // EP3E的厂家标识和产品标识

// CoE对象字典
#define RXPDO 0x1600
#define TXPDO 0x1A00

static void
check_domain_state(ec_domain_t *domain, ec_domain_state_t *domain_state) {
    ec_domain_state_t ds;
    ecrt_domain_state(domain, &ds);
    if(ds.working_counter != domain_state->working_counter) {
         printf("Domain: WC %u.\n", ds.working_counter);
    }
    if(ds.wc_state != domain_state->wc_state) {
         printf("Domain: State %u.\n", ds.wc_state);
    }
    *domain_state = ds;
}

static void
check_master_state(ec_master_t *master, ec_master_state_t *master_state) {
    ec_master_state_t ms;
    ecrt_master_state(master, &ms);
    if(ms.slaves_responding != master_state->slaves_responding) {
         printf("%u slave(s).\n", ms.slaves_responding);
    }
    if(ms.al_states != master_state->al_states) {
         printf("AL states: 0x%02X.\n", ms.al_states);
    }
    if(ms.link_up != master_state->link_up) {
         printf("Link is %s.\n", ms.link_up ? "up" : "down");
    }
    *master_state = ms;
}

static void
check_slave_config_states(ec_slave_config_t *slave,
                          ec_slave_config_state_t *slave_state) {
    ec_slave_config_state_t s;
    ecrt_slave_config_state(slave, &s);
    if(s.al_state != slave_state->al_state) {
        printf("EP3E: State 0x%02X.\n", s.al_state);
    }
    if(s.online != slave_state->online) {
        printf("EP3E: %s.\n", s.online ? "online" : "offline");
    }
    if(s.operational != slave_state->operational) {
        printf("EP3E: %soperational.\n", s.operational ? "" : "Not ");
    }
    *slave_state = s;
}

//初始化EtherCAT主站函数
static ec_master_t *
init_EtherCAT_master(struct MOTOR *motor,ec_master_t *master) {
    //创建ethercat主站master
    master=ecrt_request_master(0);
    if(!master) {
        printf("Failed to create ethercat master!\n");
        exit(EXIT_FAILURE);  //创建失败，退出线程
    }
    else
        printf("ethercat master create seccessed\n");

    //num为电机个数
    for (uint16_t i=0;i< SERVER_NUM;i++){

            //变量与对应PDO数据对象关联
        ec_pdo_entry_reg_t domain_regs[] = {
            {EP3ESLAVEPOS,i,MAXSINE, CTRL_WORD, 0, &(motor+i)->drive_variables.ctrl_word},
            {EP3ESLAVEPOS,i,MAXSINE, OPERATION_MODE, 0,&(motor+i)->drive_variables.operation_mode},
            {EP3ESLAVEPOS,i,MAXSINE, TARGET_VELOCITY, 0,&(motor+i)->drive_variables.target_velocity},
            {EP3ESLAVEPOS,i,MAXSINE, TARGET_POSITION, 0,&(motor+i)->drive_variables.target_postion},
            {EP3ESLAVEPOS,i,MAXSINE, STATUS_WORD, 0, &(motor+i)->drive_variables.status_word},
            {EP3ESLAVEPOS,i, MAXSINE, MODE_DISPLAY, 0, &(motor+i)->drive_variables.mode_display},
            {EP3ESLAVEPOS,i,MAXSINE, CURRENT_VELOCITY, 0,&(motor+i)->drive_variables.current_velocity},
            {EP3ESLAVEPOS,i, MAXSINE, CURRENT_POSITION, 0,&(motor+i)->drive_variables.current_postion},
            {}};

        //填充相关PDOS信息
        ec_pdo_entry_info_t EP3E_pdo_entries[] = {/*RxPdo 0x1600*/
                                                {CTRL_WORD, 0x00, 16},
                                                {OPERATION_MODE, 0x00, 8},
                                                {TARGET_VELOCITY, 0x00, 32},
                                                {TARGET_POSITION, 0x00, 32},
                                                /*TxPdo 0x1A00*/
                                                {STATUS_WORD, 0x00, 16},
                                                {MODE_DISPLAY, 0x00, 8},
                                                {CURRENT_VELOCITY, 0x00, 32},
                                                {CURRENT_POSITION, 0x00, 32}};
        ec_pdo_info_t EP3E_pdos[] = {// RxPdo
                                    {RXPDO, 4, EP3E_pdo_entries + 0},
                                    // TxPdo
                                    {TXPDO, 4, EP3E_pdo_entries + 4}};
        ec_sync_info_t EP3E_syncs[] = {{0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
                                    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
                                    {2, EC_DIR_OUTPUT, 1, EP3E_pdos + 0, EC_WD_DISABLE},
                                    {3, EC_DIR_INPUT, 1, EP3E_pdos + 1, EC_WD_DISABLE},
                                    {0xFF}};
        // if (i==0){
        //     //创建ethercat主站master
        //     motor[i]->master = ecrt_request_master(0);
        //     if(!motor->master) {
        //         printf("Failed to create ethercat master!\n");
        //         exit(EXIT_FAILURE);  //创建失败，退出线程
        //     }
        // }
        

        //创建域domain
        (motor+i)->domain = ecrt_master_create_domain(master);
        if(!(motor+i)->domain) {
            printf("Failed to create master domain!\n");
            exit(EXIT_FAILURE);  //创建失败，退出线程
        }

         //配置从站
        if(!((motor+i)->maxsine_EP3E =
                ecrt_master_slave_config((master), EP3ESLAVEPOS,i, MAXSINE))) {
            printf("Failed to get slave configuration for EP3E!\n");
            exit(EXIT_FAILURE);  //配置失败，退出线程
        }

        //配置PDOs
        printf("Configuring PDOs...\n");
        if(ecrt_slave_config_pdos((motor+i)->maxsine_EP3E, EC_END, EP3E_syncs)) {
            printf("Failed to configure EP3E PDOs!\n");
            exit(EXIT_FAILURE);  //配置失败，退出线程
        } else {
            printf("*Success to configuring EP3E PDOs*\n");  //配置成功
        }

        //注册PDO entry
        if(ecrt_domain_reg_pdo_entry_list((motor+i)->domain, domain_regs)) {
            printf("PDO entry registration failed!\n");
            exit(EXIT_FAILURE);  //注册失败，退出线程
        } else {
            printf("*Success to configuring EP3E PDO entry*\n");  //注册成功
        }  
    }
    //激活主站master
    printf("Activating master...\n");
    if(ecrt_master_activate(master)) {
        exit(EXIT_FAILURE);  //激活失败，退出线程
    } else {
        printf("*Master activated*\n");  //激活成功
    }
    for (uint16_t i=0;i< SERVER_NUM;i++){
        if(!((motor+i)->domain_pd = ecrt_domain_data((motor+i)->domain))) {
        exit(EXIT_FAILURE);
        }
    }
    return master;
}


int main(int argc, char **argv) 
{
	struct MOTOR motor[2];
    ec_master_t *master;
    master=init_EtherCAT_master(motor,master);
    if (!master) {
        printf("master null\n");
        exit;
    }
    printf("*It's working now*\n");
    motor[0].targetPosition = 0;
    motor[0].opModeSet = 8;     //位置模式
    motor[0].homeBusy = true;
	motor[0].powerBusy = true;

    motor[1].targetPosition = 0;
    motor[1].opModeSet = 8;     //位置模式
    motor[1].homeBusy = true;
	motor[1].powerBusy = true;
    int8_t reset_count = 0;  //复位的时候计数用
	int16_t position_count = 0;  //位置模式下计数
	double t = 0.00;
	int count = 0;
	while (true){
		usleep(250);
        ecrt_master_receive(master);
        //检查主站状态
        //check_master_state(motor[0].master, &motor[0].master_state);

		for (int i=0;i< SERVER_NUM;i++){
                //接收过程数据
            ecrt_domain_process(motor[i].domain);

            //检查过程数据状态（可选）
            check_domain_state(motor[i].domain, &motor[i].domain_state);
            
            //检查从站配置状态
            check_slave_config_states(motor[i].maxsine_EP3E, &motor[i].maxsine_EP3E_state);

            //读取数据
            motor[i].status =
                EC_READ_U16(motor[i].domain_pd + motor[i].drive_variables.status_word);  //状态字
            motor[i].opmode =
                EC_READ_U8(motor[i].domain_pd + motor[i].drive_variables.mode_display);  //运行模式
            motor[i].currentVelocity = EC_READ_S32(
                motor[i].domain_pd + motor[i].drive_variables.current_velocity);  //当前速度
            motor[i].currentPosition = EC_READ_S32(
                motor[i].domain_pd + motor[i].drive_variables.current_postion);  //当前位置
    

            printf("%d号电机当前速度%d||||当前位置%d\n",i,motor[i].currentVelocity,motor[i].currentPosition);

            // DS402 CANOpen over EtherCAT status machine
            //获取当前驱动器的驱动状态
            if((motor[i].status & 0x004F) == 0x0000)
                motor[i].driveState = dsNotReadyToSwitchOn;  //初始化 未完成状态
            else if((motor[i].status & 0x004F) == 0x0040)
                motor[i].driveState = dsSwitchOnDisabled;  //初始化 完成状态
            else if((motor[i].status & 0x006F) == 0x0021)
                motor[i].driveState = dsReadyToSwitchOn;  //主电路电源OFF状态
            else if((motor[i].status & 0x006F) == 0x0023)
                motor[i].driveState = dsSwitchedOn;  //伺服OFF/伺服准备
            else if((motor[i].status & 0x006F) == 0x0027)
                motor[i].driveState = dsOperationEnabled;  //伺服ON
            else if((motor[i].status & 0x006F) == 0x0007)
                motor[i].driveState = dsQuickStopActive;  //即停止
            else if((motor[i].status & 0x004F) == 0x000F)
                motor[i].driveState = dsFaultReactionActive;  //异常（报警）判断
            else if((motor[i].status & 0x004F) == 0x0008)
                motor[i].driveState = dsFault;  //异常（报警）状态
            
            if(motor[i].powerBusy == true) {
                switch(motor[i].driveState) {
                    case dsNotReadyToSwitchOn:
                        break;

                    case dsSwitchOnDisabled:
                        //设置运行模式为位置模式
                        EC_WRITE_S8(motor[i].domain_pd + motor[i].drive_variables.operation_mode,
                                    motor[i].opModeSet);
                        EC_WRITE_U16(motor[i].domain_pd + motor[i].drive_variables.ctrl_word,
                                    0x0006);
                        break;

                    case dsReadyToSwitchOn:
                        EC_WRITE_U16(motor[i].domain_pd + motor[i].drive_variables.ctrl_word,
                                    0x0007);
                        break;

                    case dsSwitchedOn:
                        EC_WRITE_U16(motor[i].domain_pd + motor[i].drive_variables.ctrl_word,
                                    0x000f);  // enable operation
                        motor[i].targetPosition =
                            motor[i].currentPosition;  //将当前位置复制给目标位置，防止使能后电机震动
                        EC_WRITE_S32(motor[i].domain_pd + motor[i].drive_variables.target_postion,
                                    motor[i].targetPosition);
                        break;

                    default:
                        motor[i].powerBusy = false;
                }
            }

            if(motor[i].driveState == dsOperationEnabled && motor[i].resetBusy == 0 &&
           motor[i].powerBusy == 0 && motor[i].quickStopBusy == 0) {
                if(motor[i].opmode == 8) {           //位置模式
                    if(motor[i].homeBusy == true) {  //开始回零
                        if(motor[i].currentPosition > 0) {
                            motor[i].targetPosition = motor[i].currentPosition - HOME_STEP;
                            if(motor[i].targetPosition < 0) {
                                motor[i].targetPosition = 0;
                            }
                        } else if(motor[i].currentPosition < 0) {
                            motor[i].targetPosition = motor[i].currentPosition + HOME_STEP;
                            if(motor[i].targetPosition > 0) {
                                motor[i].targetPosition = 0;
                            }
                        } else if(motor[i].currentPosition == 0) {
                            motor[i].homeBusy = false;  //回零结束
                            motor[i].positionMoving = true;
                            t = 0;
                        }
                        EC_WRITE_S32(motor[i].domain_pd + motor[i].drive_variables.target_postion,
                                    motor[i].targetPosition);
                    } else {
                        if(motor[i].positionMoving == true) {  //开始运动
                            if(position_count == 8000) {
                                position_count = 0;
                                t = 0;
                            }
                            motor[i].targetPosition =
                                (int32_t)(5*ENCODER_RESOLUTION * sin(Pi * t)/2 );  //位置模式时传送位置信息
                            //printf("电机实时位置%d\t,电机目标位置%d\n",motor.currentPosition,motor.targetPosition);
                            // EC_WRITE_S32(motor.domain_pd +
                            //                  motor.drive_variables.target_postion, motor.targetPosition);
                            t = t + (double)POSITION_STEP;
                            position_count = position_count + 1;
                            
                            EC_WRITE_S32(motor[i].domain_pd +
                                            motor[i].drive_variables.target_postion,
                                        motor[i].targetPosition);
                        } else {
                            motor[i].targetPosition = motor[i].currentPosition;
                            EC_WRITE_S32(motor[i].domain_pd +
                                            motor[i].drive_variables.target_postion,
                                        motor[i].targetPosition);
                        }
                    }
                }
                EC_WRITE_U16(motor[i].domain_pd + motor[i].drive_variables.ctrl_word, 0x001f);
            }


            // count++;
            // if (count>100){
            //     count=0;
            //     //Status_Check_printf(motor[i].status);
            //     printf("%d号电机当前速度%d||||当前位置%d\n",i,motor[i].currentVelocity,motor[i].currentPosition);
            // }
            ecrt_domain_queue(motor[i].domain);
	    }
        ecrt_master_send(master);
    }
	ecrt_master_deactivate(master);
}
