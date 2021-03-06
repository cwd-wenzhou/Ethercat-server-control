/*
 * @Author       : cwd
 * @Date         : 2021-5-10
 * @Place  : hust
 */
#ifndef  HEADER_H
#define HEADER_H

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <ecrt.h>
#include <math.h>
#include <stdio.h>

#define TASK_FREQUENCY 4000        //*Hz* 任务周期
#define ENCODER_RESOLUTION 131072  //编码器分辨率
#define HOME_VECOLITY 5            // r/s，回零速度
#define HOME_STEP HOME_VECOLITY *ENCODER_RESOLUTION / TASK_FREQUENCY  // pulse 回零步长
#define POSITION_STEP 1 / TASK_FREQUENCY  //位置模式下步长
#define Pi 3.141592654                    //圆周率

//从站配置所用的参数
#define EP3ESLAVEPOS 0, 0               //迈信伺服EP3E在ethercat总线上的位置
#define MAXSINE 0x000007DD, 0x00000001  // EP3E的厂家标识和产品标识

// CoE对象字典
#define RXPDO 0x1600
#define TXPDO 0x1A00


static void
check_domain_state(ec_domain_t *domain, ec_domain_state_t *domain_state) {
    ec_domain_state_t ds;
    ecrt_domain_state(domain, &ds);
    if(ds.working_counter != domain_state->working_counter) {
        // printf("Domain: WC %u.\n", ds.working_counter);
    }
    if(ds.wc_state != domain_state->wc_state) {
        // printf("Domain: State %u.\n", ds.wc_state);
    }
    *domain_state = ds;
}

static void
check_master_state(ec_master_t *master, ec_master_state_t *master_state) {
    ec_master_state_t ms;
    ecrt_master_state(master, &ms);
    if(ms.slaves_responding != master_state->slaves_responding) {
        // printf("%u slave(s).\n", ms.slaves_responding);
    }
    if(ms.al_states != master_state->al_states) {
        // printf("AL states: 0x%02X.\n", ms.al_states);
    }
    if(ms.link_up != master_state->link_up) {
        // printf("Link is %s.\n", ms.link_up ? "up" : "down");
    }
    *master_state = ms;
}

static void
check_slave_config_states(ec_slave_config_t *slave,
                          ec_slave_config_state_t *slave_state) {
    ec_slave_config_state_t s;
    ecrt_slave_config_state(slave, &s);
    if(s.al_state != slave_state->al_state) {
        //  printf("EP3E: State 0x%02X.\n", s.al_state);
    }
    if(s.online != slave_state->online) {
        // printf("EP3E: %s.\n", s.online ? "online" : "offline");
    }
    if(s.operational != slave_state->operational) {
        // printf("EP3E: %soperational.\n", s.operational ? "" : "Not ");
    }
    *slave_state = s;
}

//获取当前驱动器的驱动状态
void Status_Check_printf(uint16_t motor_status_){
    
    if((motor_status_ & 0x004F) == 0x0000)
        printf("motor.driveState = dsNotReadyToSwitchOn; 初始化 未完成状态\n");
    else if((motor_status_ & 0x004F) == 0x0040)
        printf("motor.driveState = dsSwitchOnDisabled;  初始化 完成状态\n");
    else if((motor_status_ & 0x006F) == 0x0021)
        printf("motor.driveState = dsReadyToSwitchOn; 主电路电源OFF状态\n");
    else if((motor_status_ & 0x006F) == 0x0023)
        printf("motor.driveState = dsSwitchedOn; 伺服OFF/伺服准备\n");
    else if((motor_status_ & 0x006F) == 0x0027)
        printf("motor.driveState = dsOperationEnabled; 伺服ON\n");
    else if((motor_status_ & 0x006F) == 0x0007)
        printf("motor.driveState = dsQuickStopActive; 即停止\n");
    else if((motor_status_ & 0x004F) == 0x000F)
        printf("motor.driveState = dsFaultReactionActive;异常（报警）判断\n");
    else if((motor_status_ & 0x004F) == 0x0008)
        printf("motor.driveState = dsFault;异常（报警）状态\n");
    else 
        printf("unknow state\n");
        
}

//获取当前驱动器的驱动状态
const char* Status_Check_char(uint16_t motor_status_){
    
    if((motor_status_ & 0x004F) == 0x0000)
        return "motor.driveState = dsNotReadyToSwitchOn; 初始化 未完成状态\n";
    else if((motor_status_ & 0x004F) == 0x0040)
        return "motor.driveState = dsSwitchOnDisabled;  初始化 完成状态\n";
    else if((motor_status_ & 0x006F) == 0x0021)
        return "motor.driveState = dsReadyToSwitchOn; 主电路电源OFF状态\n";
    else if((motor_status_ & 0x006F) == 0x0023)
        return "motor.driveState = dsSwitchedOn; 伺服OFF/伺服准备\n";
    else if((motor_status_ & 0x006F) == 0x0027)
        return "motor.driveState = dsOperationEnabled; 伺服ON\n";
    else if((motor_status_ & 0x006F) == 0x0007)
        return "motor.driveState = dsQuickStopActive; 即停止\n";
    else if((motor_status_ & 0x004F) == 0x000F)
        return "motor.driveState = dsFaultReactionActive;异常（报警）判断\n";
    else if((motor_status_ & 0x004F) == 0x0008)
        return "motor.driveState = dsFault;异常（报警）状态\n";
    else 
        return "unknow state\n";
        
}

//获取当前驱动器的驱动状态,并修改driverstate
void Status_Check(uint16_t motor_status_,enum DRIVERSTATE *motor_driveState_){
    
    if((motor_status_ & 0x004F) == 0x0000)
        *motor_driveState_ = dsNotReadyToSwitchOn;  //初始化 未完成状态
    else if((motor_status_ & 0x004F) == 0x0040)
        *motor_driveState_ = dsSwitchOnDisabled;  //初始化 完成状态
    else if((motor_status_ & 0x006F) == 0x0021)
        *motor_driveState_ = dsReadyToSwitchOn;  //主电路电源OFF状态
    else if((motor_status_ & 0x006F) == 0x0023)
        *motor_driveState_ = dsSwitchedOn;  //伺服OFF/伺服准备
    else if((motor_status_ & 0x006F) == 0x0027)
        *motor_driveState_ = dsOperationEnabled;  //伺服ON
    else if((motor_status_ & 0x006F) == 0x0007)
        *motor_driveState_ = dsQuickStopActive;  //即停止
    else if((motor_status_ & 0x004F) == 0x000F)
        *motor_driveState_ = dsFaultReactionActive;  //异常（报警）判断
    else if((motor_status_ & 0x004F) == 0x0008)
        *motor_driveState_ = dsFault;  //异常（报警）状态
}

//状态机，把伺服使能
void State_Machine(struct MOTOR *motor){
    if(motor->powerBusy == true) {
            switch(motor->driveState) {
                case dsNotReadyToSwitchOn:
                    break;

                case dsSwitchOnDisabled:
                    //设置运行模式和控制位，否则会异常
                    EC_WRITE_S8(motor->domain_pd + motor->drive_variables.operation_mode,
                                motor->opModeSet);
                    EC_WRITE_U16(motor->domain_pd + motor->drive_variables.ctrl_word,
                                    0x0006);
                    break;

                case dsReadyToSwitchOn:
                    EC_WRITE_U16(motor->domain_pd + motor->drive_variables.ctrl_word,
                                    0x0007);
                    break;

                case dsSwitchedOn:
                    EC_WRITE_U16(motor->domain_pd + motor->drive_variables.ctrl_word,
                                    0x000f);  // enable operation
                    motor->targetPosition =motor->currentPosition;  //将当前位置复制给目标位置，防止使能后电机震动
                    EC_WRITE_S32(motor->domain_pd + motor->drive_variables.target_postion,
                                    motor->targetPosition);
                    break;
                default:
                    motor->powerBusy = false;
            }
        }
}

//判断伺服是否处于使能状态
bool Is_Serevr_On(struct MOTOR *motor){
    if  (motor->driveState == dsOperationEnabled && motor->powerBusy==false)
        return true;
    return false;
}


#endif
