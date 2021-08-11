#include "motor.h"

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

ec_sync_info_t* MOTOR::get_ec_sync_info_t_(){
    return EP3E_syncs;
}

void MOTOR::read_data(){
    this->status = EC_READ_U16(this->domain_pd + this->drive_variables.status_word);  //状态字
    this->opmode = EC_READ_U8(this->domain_pd + this->drive_variables.mode_display);  //运行模式
    this->currentVelocity = EC_READ_S32(this->domain_pd + this->drive_variables.current_velocity);  //当前速度
    this->currentPosition = EC_READ_S32(this->domain_pd + this->drive_variables.current_postion);  //当前位置
}

void MOTOR::send_data(){
    Status_Check(this);
    State_Machine(this);
    if (Is_Serevr_On(this))
        {
            EC_WRITE_S32(this->domain_pd + this->drive_variables.target_postion,
                this->targetPosition);

            EC_WRITE_S32(this->domain_pd + this->drive_variables.target_velocity,
                this->targetVelocity);

        }
}

ec_pdo_entry_reg_t* MOTOR::Domain_regs(uint16_t position){
    ec_pdo_entry_reg_t*  ans= new ec_pdo_entry_reg_t[9]{
        {0,position,VenderID,ProductCode, CTRL_WORD, 0, &this->drive_variables.ctrl_word},
        {0,position,VenderID,ProductCode, OPERATION_MODE, 0,&this->drive_variables.operation_mode},
        {0,position,VenderID,ProductCode, TARGET_VELOCITY, 0,&this->drive_variables.target_velocity},
        {0,position,VenderID,ProductCode, TARGET_POSITION, 0,&this->drive_variables.target_postion},
        {0,position,VenderID,ProductCode, STATUS_WORD, 0, &this->drive_variables.status_word},
        {0,position,VenderID,ProductCode, MODE_DISPLAY, 0, &this->drive_variables.mode_display},
        {0,position,VenderID,ProductCode, CURRENT_VELOCITY, 0,&this->drive_variables.current_velocity},
        {0,position,VenderID,ProductCode, CURRENT_POSITION, 0,&this->drive_variables.current_postion},
        {}
    };
    return ans;
}


//驱动器状态打印函数
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
void Status_Check(MOTOR *motor){
    uint16_t motor_status_ = motor->status;

    if((motor_status_ & 0x004F) == 0x0000)
        motor->driveState = dsNotReadyToSwitchOn;  //初始化 未完成状态
    else if((motor_status_ & 0x004F) == 0x0040)
        motor->driveState = dsSwitchOnDisabled;  //初始化 完成状态
    else if((motor_status_ & 0x006F) == 0x0021)
        motor->driveState = dsReadyToSwitchOn;  //主电路电源OFF状态
    else if((motor_status_ & 0x006F) == 0x0023)
        motor->driveState = dsSwitchedOn;  //伺服OFF/伺服准备
    else if((motor_status_ & 0x006F) == 0x0027)
        motor->driveState = dsOperationEnabled;  //伺服ON
    else if((motor_status_ & 0x006F) == 0x0007)
        motor->driveState = dsQuickStopActive;  //即停止
    else if((motor_status_ & 0x004F) == 0x000F)
        motor->driveState = dsFaultReactionActive;  //异常（报警）判断
    else //if((motor_status_ & 0x004F) == 0x0008)
        motor->driveState = dsFault;  //异常（报警）状态

}

//状态机，把伺服使能
void State_Machine(MOTOR *motor){
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
bool Is_Serevr_On(MOTOR *motor){
    if  (motor->driveState == dsOperationEnabled && motor->powerBusy==false)
        return true;
    return false;
}

