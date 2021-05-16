/*
* @Author       : cwd
* @Date         : 2021-5-16
* @Place  : hust
* 伺服控制函数
*/
#ifndef SERVER_H
#define SERVER_H

#include "header.h"

//回零函数
void Homing(struct MOTOR *motor){
    //motor->positionMoving 表示电机是否在运动的标志位
    motor->positionMoving = true;//开始运动
    while(motor->positionMoving)
    {
        usleep(1000);
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
                    //已经回0，停止运动
                    motor->positionMoving=false;
                }
                // EC_WRITE_S32(motor->domain_pd + motor->drive_variables.target_postion,
                //                 motor->targetPosition);
            }
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
    motor->positionMoving = true;//开始运动
    while(motor->positionMoving)
    {
        usleep(1000);
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
                    motor->positionMoving = false;  
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
}

#endif //SERVERH