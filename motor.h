/*
 * @Author       : cwd
 * @Date         : 2021-7-28
 * @Place  : hust
 * 该头文件建立符合cia402行规的电机模型MOTOR
 * 
 */
#ifndef MOTOR_def
#define MOTOR_def

#include <ecrt.h>
#include "slave.h"
//电机控制基本参数
#define TASK_FREQUENCY 4000        //*Hz* 任务周期
#define ENCODER_RESOLUTION 131072  //编码器分辨率
#define HOME_VECOLITY 5            // r/s，回零速度
#define HOME_STEP HOME_VECOLITY *ENCODER_RESOLUTION / TASK_FREQUENCY  // pulse 回零步长
#define POSITION_STEP 1 / TASK_FREQUENCY  //位置模式下步长
#define Pi 3.141592654                    //圆周率

/*CiA 402数据对象(Data Object)*/
//RxPDO 
//1600h映射
#define CTRL_WORD 0x6040         //控制字的数据对象
#define OPERATION_MODE 0x6060    //设定运行模式的数据对象
#define TARGET_POSITION 0x607A   //目标位置的数据对象
#define TARGET_VELOCITY 0x60FF   //目标速度的数据对象
#define TARGET_TORQUE 0x6071   //目标力矩的数据对象
//1602映射

//TxPDO 1600h映射
#define STATUS_WORD 0x6041       //状态字的数据对象
#define MODE_DISPLAY 0x6061      //当前运行模式的数据对象
#define CURRENT_POSITION 0x6064  //当前位置的数据对象
#define CURRENT_VELOCITY 0x606C  //当前速度的数据对象
#define CURRENT_TORQUE 0x6077  //当前力矩的数据对象

#define MAX_TORQUE  0x6072   //最大力矩的数据对象

//从站配置所用的参数
#define EP3ESLAVEPOS 0              //迈信伺服EP3E在ethercat总线上的位置
//#define MAXSINE 0x000007DD, 0x00000001  // EP3E的厂家标识和产品标识

// CoE对象字典
#define RXPDO 0x1600
#define TXPDO 0x1A00

//伺服驱动器里PDO入口的偏移量
/*我们需要定义一些变量去关联需要用到的从站的PD0对象*/
struct DRIVERVARIABLE {
    unsigned int operation_mode;   //设定运行模式
    unsigned int ctrl_word;        //控制字
    unsigned int target_torque;  //目标力矩（)
    unsigned int target_velocity;  //目标速度 （pulse/s)
    unsigned int target_postion;   //目标位置 （pulse）
    unsigned int max_torque; 

    unsigned int status_word;       //状态字
    unsigned int mode_display;      //实际运行模式
    unsigned int current_torque;  //当前力矩 （）
    unsigned int current_velocity;  //当前速度 （pulse/s）
    unsigned int current_postion;   //当前位置 （pulse）
};

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

//迈信伺服电机结构体
class MOTOR:public SLAVE {
public:
    MOTOR(){
        targetPosition = 0;
        opModeSet = 8;     //位置模式
        homeBusy = true;
        powerBusy = true;
    }
    MOTOR(unsigned int venderID,unsigned int productCode){
        VenderID = venderID;
        ProductCode = productCode;
        targetPosition = 0;
        opModeSet = 8;     //位置模式
        homeBusy = true;
        powerBusy = true;
    }
    struct DRIVERVARIABLE drive_variables;  //从站驱动器变量

    int32_t targetPosition;   //电机的目标位置
    int32_t targetVelocity;   //电机的目标速度
    int32_t targettorque;   //电机的目标力矩
    int8_t opModeSet;         //电机运行模式的设定值,默认位置模式
    int8_t opmode;            //驱动器当前运行模式
    int32_t currenttorque;  //电机当前运行力矩
    int32_t currentVelocity;  //电机当前运行速度
    int32_t currentPosition;  //电机当前位置
    uint16_t status;          //驱动器状态字

    enum DRIVERSTATE driveState;  //驱动器状态

    //关于电机控制的私有变量
    bool powerBusy;       //使能标志位
    bool resetBusy;       //复位标志位
    bool quickStopBusy;   //急停标志位
    bool homeBusy;        //回零标志位
    bool positionMoving;  //位置模式下运动

    unsigned int VenderID;
    unsigned int ProductCode;
    void read_data();
    void send_data();
    void print();
    ec_pdo_entry_reg_t *Domain_regs(uint16_t position);
    ec_sync_info_t* get_ec_sync_info_t_();

    ~MOTOR(){};
};



//获取当前驱动器的驱动状态
const char* Status_Check_char(uint16_t motor_status_);

//获取当前驱动器的驱动状态,并修改driverstate
void Status_Check( MOTOR *motor);

//状态机，把伺服使能
void State_Machine( MOTOR *motor);

//判断伺服是否处于使能状态
bool Is_Serevr_On(MOTOR *motor);

//填充相关PDOS信息
extern ec_pdo_entry_info_t EP3E_pdo_entries[9];
extern ec_pdo_info_t EP3E_pdos[2];
extern ec_sync_info_t EP3E_syncs[5];


#endif //MOTOR_def