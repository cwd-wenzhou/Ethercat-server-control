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
/*CiA 402数据对象(Data Object)*/
#define CTRL_WORD 0x6040         //控制字的数据对象
#define OPERATION_MODE 0x6060    //设定运行模式的数据对象
#define TARGET_VELOCITY 0x60FF   //目标速度的数据对象
#define TARGET_POSITION 0x607A   //目标位置的数据对象
#define STATUS_WORD 0x6041       //状态字的数据对象
#define MODE_DISPLAY 0x6061      //当前运行模式的数据对象
#define CURRENT_VELOCITY 0x606C  //当前速度的数据对象
#define CURRENT_POSITION 0x6064  //当前位置的数据对象


enum DRIVERSTATE {
    dsNotReadyToSwitchOn = 0,  //初始化 未完成状态
    dsSwitchOnDisabled,        //初始化 完成状态
    dsReadyToSwitchOn,         //主电路电源OFF状态
    dsSwitchedOn,              //伺服OFF/伺服准备
    dsOperationEnabled,        //伺服ON
    dsQuickStopActive,         //即停止
    dsFaultReactionActive,     //异常（报警）判断
    dsFault                    //异常（报警）状态
};

//迈信伺服驱动器里PDO入口的偏移量
/*我们需要定义一些变量去关联需要用到的从站的PD0对象*/
struct DRIVERVARIABLE {
    unsigned int operation_mode;   //设定运行模式
    unsigned int ctrl_word;        //控制字
    unsigned int target_velocity;  //目标速度 （pulse/s)
    unsigned int target_postion;   //目标位置 （pulse）

    unsigned int status_word;       //状态字
    unsigned int mode_display;      //实际运行模式
    unsigned int current_velocity;  //当前速度 （pulse/s）
    unsigned int current_postion;   //当前位置 （pulse）
};

//迈信伺服电机结构体
struct MOTOR {
    //关于ethercat master
    ec_master_t *master;             //主站
    ec_master_state_t master_state;  //主站状态

    ec_domain_t *domain;             //域
    ec_domain_state_t domain_state;  //域状态

    ec_slave_config_t *maxsine_EP3E;  //从站配置，这里只有一台迈信伺服
    ec_slave_config_state_t maxsine_EP3E_state;  //从站配置状态

    uint8_t *domain_pd;                     // Process Data
    struct DRIVERVARIABLE drive_variables;  //从站驱动器变量

    int32_t targetPosition;   //电机的目标位置
    int32_t targetVelocity;  //电机当前运行速度
    int8_t opModeSet;         //电机运行模式的设定值,默认位置模式
    int8_t opmode;            //驱动器当前运行模式
    int32_t currentVelocity;  //电机当前运行速度
    int32_t currentPosition;  //电机当前位置
    uint16_t status;          //驱动器状态字

    enum DRIVERSTATE driveState;  //驱动器状态

    bool powerBusy;       //使能标志位
    bool resetBusy;       //复位标志位
    bool quickStopBusy;   //急停标志位
    bool homeBusy;        //回零标志位
    bool positionMoving;  //位置模式下运动
};

struct MOTOR_DATA{
    //关于电机控制的私有变量
    bool powerBusy;       //使能标志位
    bool resetBusy;       //复位标志位
    bool quickStopBusy;   //急停标志位
    bool homeBusy;        //回零标志位
    bool positionMoving;  //位置模式下运动
    int32_t currentVelocity;  //电机当前运行速度
    int32_t currentPosition;  //电机当前位置
    int32_t targetPosition;   //电机的目标位置
};

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
bool Is_Serevr_On(struct MOTOR *motor){
    if  (motor->driveState == dsOperationEnabled && motor->powerBusy==false)
        return true;
    return false;
}
void Homing(struct MOTOR *motor){
    if(motor->driveState == dsOperationEnabled && motor->resetBusy == 0 &&
        motor->powerBusy == 0 && motor->quickStopBusy == 0) {
        if(motor->homeBusy == true) {  //开始回零
            // printf("电机实时位置%d\t,电机目标位置%d\n",motor->currentPosition,motor->targetPosition);
            if(motor->currentPosition > 0) {
                motor->targetPosition = motor->currentPosition - HOME_STEP;
                if(motor->targetPosition < 0) {
                    motor->targetPosition = 0;
                }
            } else if(motor->currentPosition < 0) {
                motor->targetPosition = motor->currentPosition + HOME_STEP;
                if(motor->targetPosition > 0) {
                    motor->targetPosition = 0;
                }
            } else if(motor->currentPosition == 0) {
                motor->positionMoving=false;
            }
            EC_WRITE_S32(motor->domain_pd + motor->drive_variables.target_postion,
                            motor->targetPosition);
        }
    } 
}

//速度模式，规定velocity单位为 rad/s
void Velocity(struct MOTOR *motor,int velocity){
    motor->targetVelocity=ENCODER_RESOLUTION/2/Pi*velocity;
    //motor->targetVelocity=velocity;
    if(motor->driveState == dsOperationEnabled && motor->resetBusy == 0 &&
        motor->powerBusy == 0 && motor->quickStopBusy == 0 ) {
        if(motor->opmode == 9) {  
            //本来就是速度模式
            EC_WRITE_S32(motor->domain_pd + motor->drive_variables.target_velocity,
                            motor->targetVelocity);
        }
        else {
            /*
                todo
                进到这里说明已经是位置模式了
                应该要先把电机停下来，然后再置成速度模式
            */
        }
    }
}

//位置模式，将电机移动到指定位置
void Position(struct MOTOR *motor,int position_){
    if(motor->driveState == dsOperationEnabled && motor->resetBusy == 0 &&
        motor->powerBusy == 0 && motor->quickStopBusy == 0) {
        //本来就是位置模式    
        if(motor->opmode == 8) {  
            if(motor->currentPosition > position_) {
                motor->targetPosition = motor->currentPosition - HOME_STEP;
                if(motor->targetPosition < position_) {
                    motor->targetPosition = position_;
                }
            } else if(motor->currentPosition < position_) {
                motor->targetPosition = motor->currentPosition + HOME_STEP;
                if(motor->targetPosition > position_) {
                    motor->targetPosition = position_;
                }
            } else if(motor->currentPosition == position_) {
                motor->homeBusy = false;  
            }
            // EC_WRITE_S32(motor->domain_pd + motor->drive_variables.target_postion,
            //                 motor->targetPosition);
        }
        else {
            /*
                todo
                进到这里说明已经是速度模式了
                应该要先把电机停下来，然后再置成位置模式
            */
        }
    } 
}
#endif
