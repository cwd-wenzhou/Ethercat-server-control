
#ifndef FIBER_MOTOR
#define FIBER_MOTOR
#endif

#include <stdlib.h>
#include <ecrt.h>
#include <math.h>
#include <unistd.h>
#include <time.h> //nanosleep
#define SERVER_NUM 6
#define FIBER_MOTOR_NUM 6
// ethercat主站应用程序所用的参数
#define TASK_FREQUENCY 1000                                          //*Hz* 任务周期
#define ENCODER_RESOLUTION 131072                                    //编码器分辨率
#define HOME_VECOLITY 80                                             // r/s，回零速度
#define HOME_STEP HOME_VECOLITY *ENCODER_RESOLUTION / TASK_FREQUENCY // pulse 回零步长

#define RUN_VECOLITY 10                                            // r/s，速度
#define RUN_STEP RUN_VECOLITY *ENCODER_RESOLUTION / TASK_FREQUENCY // pulse 回零步长

#define POSITION_STEP 1 / TASK_FREQUENCY //位置模式下步长
#define Pi 3.141592654                   //圆周率

//从站配置所用的参数
#define EP3ESLAVEPOS 0                 //迈信伺服EP3E在ethercat总线上的位置
#define MAXSINE 0x000007DD, 0x00000001 // EP3E的厂家标识和产品标识

// CoE对象字典
#define RXPDO 0x1600
#define TXPDO 0x1A00
/*CiA 402数据对象(Data Object)*/
#define CTRL_WORD 0x6040        //控制字的数据对象
#define OPERATION_MODE 0x6060   //设定运行模式的数据对象
#define TARGET_VELOCITY 0x60FF  //目标速度的数据对象
#define TARGET_POSITION 0x607A  //目标位置的数据对象
#define STATUS_WORD 0x6041      //状态字的数据对象
#define MODE_DISPLAY 0x6061     //当前运行模式的数据对象
#define CURRENT_VELOCITY 0x606C //当前速度的数据对象
#define CURRENT_POSITION 0x6064 //当前位置的数据对象
#define CURRENT_TORQUE 0x6077   //当前转距的数据对象
#define POSITION_OFFSET 0x60b0  // 位置偏差
#define VELOCITY_OFFSET 0x60b1  // 位置偏差
#define TORQUE_OFFSET 0x60b2    // 位置偏差

// DS402 CANOpen over EtherCAT 驱动器状态
enum DRIVERSTATE
{
    dsNotReadyToSwitchOn = 0, //初始化 未完成状态
    dsSwitchOnDisabled,       //初始化 完成状态
    dsReadyToSwitchOn,        //主电路电源OFF状态
    dsSwitchedOn,             //伺服OFF/伺服准备
    dsOperationEnabled,       //伺服ON
    dsQuickStopActive,        //即停止
    dsFaultReactionActive,    //异常（报警）判断
    dsFault                   //异常（报警）状态
};

//迈信伺服驱动器里PDO入口的偏移量
/*我们需要定义一些变量去关联需要用到的从站的PD0对象*/
struct DRIVERVARIABLE
{
    unsigned int operation_mode;  //设定运行模式
    unsigned int ctrl_word;       //控制字
    unsigned int target_velocity; //目标速度 （pulse/s)
    unsigned int target_postion;  //目标位置 （pulse）
    unsigned int target_torque;   //目标位置 （pulse）

    unsigned int status_word;      //状态字
    unsigned int mode_display;     //实际运行模式
    unsigned int current_velocity; //当前速度 （pulse/s）
    unsigned int current_postion;  //当前位置 （pulse）
    unsigned int current_torque;   //当前位置 （pulse）

    unsigned int position_offset; //速度偏差
    unsigned int velocity_offset; //速度偏差
    unsigned int torque_offset;   //速度偏差
};

//迈信伺服电机结构体
struct MOTOR
{
    //关于ethercat master
    ec_master_t *master;            //主站
    ec_master_state_t master_state; //主站状态

    ec_domain_t *domain;            //域
    ec_domain_state_t domain_state; //域状态

    ec_slave_config_t *maxsine_EP3E;            //从站配置，这里只有一台迈信伺服
    ec_slave_config_state_t maxsine_EP3E_state; //从站配置状态

    uint8_t *domain_pd;                    // Process Data
    struct DRIVERVARIABLE drive_variables; //从站驱动器变量

    int32_t targetPosition;  //电机的目标位置
    int8_t opModeSet;        //电机运行模式的设定值,默认位置模式
    int8_t opmode;           //驱动器当前运行模式
    int32_t currentVelocity; //电机当前运行速度
    int32_t currentPosition; //电机当前位置
    int16_t currentTorque;   //电机当前转距

    int32_t position_offset; //速度偏差
    int32_t velocity_offset; //速度偏差
    int16_t torque_offset;   //速度偏差

    uint16_t status; //驱动器状态字

    enum DRIVERSTATE driveState; //驱动器状态

    //关于电机控制的私有变量
    bool powerBusy;      //使能标志位
    bool disable;        //使能标志位
    bool resetBusy;      //复位标志位
    bool quickStopBusy;  //急停标志位
    bool homeBusy;       //回零标志位
    bool positionMoving; //位置模式下运动
};

static void
check_domain_state(ec_domain_t *domain, ec_domain_state_t *domain_state)
{
    ec_domain_state_t ds;
    ecrt_domain_state(domain, &ds);
    if (ds.working_counter != domain_state->working_counter)
    {
        printf("Domain: WC %u.\n", ds.working_counter);
    }
    if (ds.wc_state != domain_state->wc_state)
    {
        printf("Domain: State %u.\n", ds.wc_state);
    }
    *domain_state = ds;
}

static void
check_master_state(ec_master_t *master, ec_master_state_t *master_state)
{
    ec_master_state_t ms;
    ecrt_master_state(master, &ms);
    if (ms.slaves_responding != master_state->slaves_responding)
    {
        printf("%u slave(s).\n", ms.slaves_responding);
    }
    if (ms.al_states != master_state->al_states)
    {
        printf("AL states: 0x%02X.\n", ms.al_states);
    }
    if (ms.link_up != master_state->link_up)
    {
        printf("Link is %s.\n", ms.link_up ? "up" : "down");
    }
    *master_state = ms;
}

static void
check_slave_config_states(ec_slave_config_t *slave,
                          ec_slave_config_state_t *slave_state)
{
    ec_slave_config_state_t s;
    ecrt_slave_config_state(slave, &s);
    if (s.al_state != slave_state->al_state)
    {
        printf("EP3E: State 0x%02X.\n", s.al_state);
    }
    if (s.online != slave_state->online)
    {
        printf("EP3E: %s.\n", s.online ? "online" : "offline");
    }
    if (s.operational != slave_state->operational)
    {
        printf("EP3E: %soperational.\n", s.operational ? "" : "Not ");
    }
    *slave_state = s;
}

//初始化EtherCAT主站函数
static void init_EtherCAT_fiber_master(struct MOTOR *fiber_motor)
{
    //创建ethercat主站master
    fiber_motor->master = ecrt_request_master(0);
    if (!fiber_motor->master)
    {
        printf("Failed to create ethercat master!\n");
        exit(EXIT_FAILURE); //创建失败，退出线程
    }
    //num为电机个数

    for (uint16_t i = 0; i < FIBER_MOTOR_NUM; i++)
    {
        // uint16_t a = 0;
        // a = i;
        //变量与对应PDO数据对象关联
        /*
        ec_pdo_entry_reg_t domain_regs[] = {
            {a,EP3ESLAVEPOS,MAXSINE, CTRL_WORD, 0, &(motor+i)->drive_variables.ctrl_word},
            {a,EP3ESLAVEPOS,MAXSINE, OPERATION_MODE, 0,
            &(motor+i)->drive_variables.operation_mode},
            {a,EP3ESLAVEPOS,MAXSINE, TARGET_VELOCITY, 0,
            &(motor+i)->drive_variables.target_velocity},
            {a,EP3ESLAVEPOS,MAXSINE, TARGET_POSITION, 0,
            &(motor+i)->drive_variables.target_postion},
            {a,EP3ESLAVEPOS,MAXSINE, STATUS_WORD, 0, &(motor+i)->drive_variables.status_word},
            {a,EP3ESLAVEPOS, MAXSINE, MODE_DISPLAY, 0, &(motor+i)->drive_variables.mode_display},
            {a,EP3ESLAVEPOS,MAXSINE, CURRENT_VELOCITY, 0,
            &(motor+i)->drive_variables.current_velocity},
            {a,EP3ESLAVEPOS, MAXSINE, CURRENT_POSITION, 0,
            &(motor+i)->drive_variables.current_postion},
            {}};
            
*/
        ec_pdo_entry_reg_t domain_regs[] = {
            {EP3ESLAVEPOS, i, MAXSINE, CTRL_WORD, 0, &(fiber_motor + i)->drive_variables.ctrl_word},
            {EP3ESLAVEPOS, i, MAXSINE, OPERATION_MODE, 0, &(fiber_motor + i)->drive_variables.operation_mode},
            {EP3ESLAVEPOS, i, MAXSINE, TARGET_VELOCITY, 0, &(fiber_motor + i)->drive_variables.target_velocity},
            {EP3ESLAVEPOS, i, MAXSINE, TARGET_POSITION, 0, &(fiber_motor + i)->drive_variables.target_postion},
            {EP3ESLAVEPOS, i, MAXSINE, TARGET_POSITION, 0, &(fiber_motor + i)->drive_variables.target_postion},

            {EP3ESLAVEPOS, i, MAXSINE, STATUS_WORD, 0, &(fiber_motor + i)->drive_variables.status_word},
            {EP3ESLAVEPOS, i, MAXSINE, MODE_DISPLAY, 0, &(fiber_motor + i)->drive_variables.mode_display},
            {EP3ESLAVEPOS, i, MAXSINE, CURRENT_VELOCITY, 0, &(fiber_motor + i)->drive_variables.current_velocity},
            {EP3ESLAVEPOS, i, MAXSINE, CURRENT_POSITION, 0, &(fiber_motor + i)->drive_variables.current_postion},
            {EP3ESLAVEPOS, i, MAXSINE, CURRENT_TORQUE, 0, &(fiber_motor + i)->drive_variables.current_torque},
            {EP3ESLAVEPOS, i, MAXSINE, POSITION_OFFSET, 0, &(fiber_motor + i)->drive_variables.position_offset},
            {EP3ESLAVEPOS, i, MAXSINE, VELOCITY_OFFSET, 0, &(fiber_motor + i)->drive_variables.velocity_offset},
            {EP3ESLAVEPOS, i, MAXSINE, TORQUE_OFFSET, 0, &(fiber_motor + i)->drive_variables.torque_offset},
            {}};

        //填充相关PDOS信息
        ec_pdo_entry_info_t EP3E_pdo_entries[] = {
            /*RxPdo 0x1600*/
            {CTRL_WORD, 0x00, 16},
            {OPERATION_MODE, 0x00, 8},
            {TARGET_VELOCITY, 0x00, 32},
            {TARGET_POSITION, 0x00, 32},
            {POSITION_OFFSET, 0x00, 32},
            {VELOCITY_OFFSET, 0x00, 32},
            {TORQUE_OFFSET, 0x00, 16},
            /*TxPdo 0x1A00*/
            {STATUS_WORD, 0x00, 16},
            {MODE_DISPLAY, 0x00, 8},
            {CURRENT_VELOCITY, 0x00, 32},
            {CURRENT_POSITION, 0x00, 32},
            {CURRENT_TORQUE, 0x00, 16},

        };
        ec_pdo_info_t EP3E_pdos[] = {// RxPdo
                                     {RXPDO, 7, EP3E_pdo_entries + 0},
                                     // TxPdo
                                     {TXPDO, 8, EP3E_pdo_entries + 7}};
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
        (fiber_motor + i)->domain = ecrt_master_create_domain(fiber_motor->master);
        if (!(fiber_motor + i)->domain)
        {
            printf("Failed to create master domain!\n");
            exit(EXIT_FAILURE); //创建失败，退出线程
        }

        //配置从站
        if (!((fiber_motor + i)->maxsine_EP3E = ecrt_master_slave_config((fiber_motor->master), EP3ESLAVEPOS, i, MAXSINE)))
        {
            printf("Failed to get slave configuration for EP3E!\n");
            exit(EXIT_FAILURE); //配置失败，退出线程
        }

        //配置PDOs
        printf("Configuring PDOs...\n");
        if (ecrt_slave_config_pdos((fiber_motor + i)->maxsine_EP3E, EC_END, EP3E_syncs))
        {
            printf("Failed to configure EP3E PDOs!\n");
            exit(EXIT_FAILURE); //配置失败，退出线程
        }
        else
        {
            printf("*Success to configuring EP3E PDOs*\n"); //配置成功
        }

        //注册PDO entry
        if (ecrt_domain_reg_pdo_entry_list((fiber_motor + i)->domain, domain_regs))
        {
            printf("PDO entry registration failed!\n");
            exit(EXIT_FAILURE); //注册失败，退出线程
        }
        else
        {
            printf("*Success to configuring EP3E PDO entry*\n"); //注册成功
        }
    }
    //激活主站master
    printf("Activating master...\n");
    if (ecrt_master_activate(fiber_motor->master))
    {
        exit(EXIT_FAILURE); //激活失败，退出线程
    }
    else
    {
        printf("*Master activated*\n"); //激活成功
    }
    for (uint16_t i = 0; i < FIBER_MOTOR_NUM; i++)
    {
        if (!((fiber_motor + i)->domain_pd = ecrt_domain_data((fiber_motor + i)->domain)))
        {
            exit(EXIT_FAILURE);
        }
    }
}

static void enable_fiber_motor(struct MOTOR *fiber_motor)
{

    while (fiber_motor[0].powerBusy == true ||
           fiber_motor[1].powerBusy == true ||
           fiber_motor[2].powerBusy == true ||
           fiber_motor[3].powerBusy == true ||
           fiber_motor[4].powerBusy == true ||
           fiber_motor[5].powerBusy == true)
    {
        usleep(250);
        ecrt_master_receive(fiber_motor[0].master);
        //检查主站状态
        //check_master_state(motor[0].master, &motor[0].master_state);

        for (int i = 0; i < FIBER_MOTOR_NUM; i++)
        {
            //接收过程数据
            ecrt_domain_process(fiber_motor[i].domain);

            //检查过程数据状态（可选）
            check_domain_state(fiber_motor[i].domain, &fiber_motor[i].domain_state);

            //检查从站配置状态
            check_slave_config_states(fiber_motor[i].maxsine_EP3E, &fiber_motor[i].maxsine_EP3E_state);

            //读取数据
            fiber_motor[i].status = EC_READ_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.status_word);               //状态字
            fiber_motor[i].opmode = EC_READ_U8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.mode_display);               //运行模式
            fiber_motor[i].currentVelocity = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_velocity); //当前速度
            fiber_motor[i].currentPosition = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_postion);  //当前位置
            fiber_motor[i].currentTorque = EC_READ_S16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_torque);     //当前转距
            //printf("%d号电机当前速度%d||||当前位置%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition);

            // DS402 CANOpen over EtherCAT status machine
            //获取当前驱动器的驱动状态
            if ((fiber_motor[i].status & 0x004F) == 0x0000)
                fiber_motor[i].driveState = dsNotReadyToSwitchOn; //初始化 未完成状态
            else if ((fiber_motor[i].status & 0x004F) == 0x0040)
                fiber_motor[i].driveState = dsSwitchOnDisabled; //初始化 完成状态
            else if ((fiber_motor[i].status & 0x006F) == 0x0021)
                fiber_motor[i].driveState = dsReadyToSwitchOn; //主电路电源OFF状态
            else if ((fiber_motor[i].status & 0x006F) == 0x0023)
                fiber_motor[i].driveState = dsSwitchedOn; //伺服OFF/伺服准备
            else if ((fiber_motor[i].status & 0x006F) == 0x0027)
                fiber_motor[i].driveState = dsOperationEnabled; //伺服ON
            else if ((fiber_motor[i].status & 0x006F) == 0x0007)
                fiber_motor[i].driveState = dsQuickStopActive; //即停止
            else if ((fiber_motor[i].status & 0x004F) == 0x000F)
                fiber_motor[i].driveState = dsFaultReactionActive; //异常（报警）判断
            else if ((fiber_motor[i].status & 0x004F) == 0x0008)
                fiber_motor[i].driveState = dsFault; //异常（报警）状态
                                                     //printf("EP3E:  currentPosition = %d  ,status = 0x%x , "
                                                     // "drivestate = %d , opmode = 0x%d\n",
                                                     // motor[i].currentPosition , motor[i].status,
                                                     // motor[i].driveState, motor[i].opmode);
            if (fiber_motor[i].powerBusy == true)
            {
                switch (fiber_motor[i].driveState)
                {
                case dsNotReadyToSwitchOn:
                    break;

                case dsSwitchOnDisabled:
                    //设置运行模式为位置模式
                    EC_WRITE_S8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.operation_mode, fiber_motor[i].opModeSet);
                    EC_WRITE_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.ctrl_word, 0x0006);
                    break;

                case dsReadyToSwitchOn:
                    EC_WRITE_S8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.operation_mode, fiber_motor[i].opModeSet);
                    EC_WRITE_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.ctrl_word, 0x0007);
                    break;

                case dsSwitchedOn:
                    EC_WRITE_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.ctrl_word, 0x000f); // enable operation
                    fiber_motor[i].targetPosition = fiber_motor[i].currentPosition;                            //将当前位置复制给目标位置，防止使能后电机震动
                    EC_WRITE_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.target_postion, fiber_motor[i].targetPosition);
                    break;
                default:
                    fiber_motor[i].powerBusy = false;
                }
            }
            ecrt_domain_queue(fiber_motor[i].domain);
        }
        ecrt_master_send(fiber_motor[0].master);
    }
}

//纳秒转换函数
void nanoSecondFieldConversion(struct timespec &t)
{
    if (t.tv_nsec > 999999999)
    {
        t.tv_sec += 1;
        t.tv_nsec -= 1000000000;
    }
}

/**
 * @description: 位置模式，将电机移动到指定位置
 * @param {MOTOR} *fiber_motor 电机结构体数组头指针
 * @param {int} index1  控制电机的位置，从1开始
 * @param {int} position_1 期望控制电机的位置 单位 units
 * @param {int} time: 期望执行周期个数，默认为毫秒数。以此来规划路径
 * @param {int} interval 控制周期 单位 us 默认ms
 * @param {int} error_allowed 允许在目标位置绝对值error_allowed误差内完成
 * @return {int} 完成运动所花周期数
 */
int SinglePosition_planded(struct MOTOR *fiber_motor, int index1, int position_1, int time, int interval = 1000,int error_allowed=10)
{
    bool flag[6] = {true, true, true, true, true, true}; //开始运动
    int position[6] = {0, 0, 0, 0, 0, 0};
    position[index1] = position_1;
    int tf = time;
    double a0, a1, a2, a3; //三次插补参数 f=a0+a1*t+a2*t*t+s3*t*t*t;
    double k, b;           //线性插补参数  f=k*t+b;

    //int step = speed * ENCODER_RESOLUTION / TASK_FREQUENCY;
    int i = index1;
    int count = 0;

    /* Get current time and compute the next nanosleeptime */
    struct timespec nextnanosleeptime;
    clock_gettime(CLOCK_REALTIME, &nextnanosleeptime);
    /* Variable to nano Sleep until SECONDS_SLEEP second boundary */
    nextnanosleeptime.tv_nsec += 250000;
    nanoSecondFieldConversion(nextnanosleeptime);

    //printf("电机当前速度 当前位置 当前转距 目标位置\n");
    while (flag[index1])
    {
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &nextnanosleeptime, NULL);
        //clock_gettime(CLOCK_TAI, &nextnanosleeptime);
        /* Variable to nano Sleep until SECONDS_SLEEP second boundary */
        nextnanosleeptime.tv_nsec += interval * 1000;
        nanoSecondFieldConversion(nextnanosleeptime);

        //usleep(interval);

        count++;
        ecrt_master_receive(fiber_motor[0].master);
        for (int i = 0; i < FIBER_MOTOR_NUM; i++)
        {
            if (i == index1)
            {
                ecrt_domain_process(fiber_motor[i].domain);

                //读取数据
                fiber_motor[i].status = EC_READ_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.status_word);               //状态字
                fiber_motor[i].opmode = EC_READ_U8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.mode_display);               //运行模式
                fiber_motor[i].currentVelocity = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_velocity); //当前速度
                fiber_motor[i].currentPosition = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_postion);  //当前位置
                fiber_motor[i].currentTorque = EC_READ_S16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_torque);     //当前转距

                if (count == 1)
                {

                    a0 = fiber_motor[i].currentPosition;
                    a1 = 0;
                    a2 = 3 * ((double)position_1 - fiber_motor[i].currentPosition) / (double)(tf * tf);
                    a3 = -2 * ((double)position_1 - fiber_motor[i].currentPosition) / (double)(tf * tf);
                    a3 = a3 / (double)tf;
                    //printf("a0 a1 a2 a3  %f  %  f  %f  %f\n", a0, a1, a2, a3);
                    //计算线性插补参数
                    k = double(position_1 - fiber_motor[i].currentPosition) / tf;
                    b = fiber_motor[i].currentPosition;
                }
                if (fiber_motor[i].driveState == dsOperationEnabled &&
                    fiber_motor[i].powerBusy == 0 && fiber_motor[i].opmode == 8)
                {
                    if (abs(fiber_motor[i].currentPosition - position_1) < error_allowed)
                        //与目标误差在十个脉冲之间认为完成
                        flag[i] = false;
                    else
                    {
                        if (count < time)
                            fiber_motor[i].targetPosition = a0 + a1 * count + a2 * count * count + a3 * count * count * count; // 根据三次插补计算
                            //fiber_motor[i].targetPosition = k * count + b; // 根据线性插补计算
                        else
                            fiber_motor[i].targetPosition = position_1;
                    }
                    EC_WRITE_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.target_postion, fiber_motor[i].targetPosition);
                }
                // fiber_motor[i].position_offset =EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.position_offset); //位置偏差
                // fiber_motor[i].velocity_offset =EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.velocity_offset); //速度偏差
                // fiber_motor[i].torque_offset =EC_READ_S16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.torque_offset); //转距偏差
                printf("%d号电机当前速度%d||||当前位置%d||||当前转距%d||||目标位置%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition, fiber_motor[i].currentTorque, fiber_motor[i].targetPosition);
                //printf("%d %d %d %d\n", fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition, fiber_motor[i].currentTorque, fiber_motor[i].targetPosition);
                ecrt_domain_queue(fiber_motor[i].domain);
            }
            else
            {
                //接收过程数据
                ecrt_domain_process(fiber_motor[i].domain);
                ecrt_domain_queue(fiber_motor[i].domain);
            }
        }
        ecrt_master_send(fiber_motor[0].master);
    }
    //printf("\n总共周期个数%lld\n", count);
    return count;
}

/**
 * @description: 位置指令控制电机运动到指定位置。
 * @param {MOTOR} *fiber_motor
 * @param {int} position_1～6  电机1～6期望运动位置
 * @param {int} time: 期望执行周期个数，默认为毫秒数。以此来规划路径
 * @param {int} interval 控制周期 单位 us 默认ms
 * @param {int} error_allowed 允许在目标位置绝对值error_allowed误差内完成
 * @return {int} 完成运动所花周期个数
 */
int Position_planed(struct MOTOR *fiber_motor, int position_1, int position_2, int position_3, int position_4, int position_5, int position_6, int time, int interval = 1000,int error_allowed=10)
{

    int tf = time;
    double a0[6], a1[6], a2[6], a3[6]; //三次插补参数 f=a0+a1*t+a2*t*t+s3*t*t*t;
    double k[6], b[6];                 //线性插补参数  f=k*t+b;
    int count = 0;

    bool flag[6] = {true, true, true, true, true, true}; //开始运动
    int position[6] = {position_1, position_2, position_3, position_4, position_5, position_6};

    /* Get current time and compute the next nanosleeptime */
    struct timespec nextnanosleeptime;
    clock_gettime(CLOCK_REALTIME, &nextnanosleeptime);
    /* Variable to nano Sleep until SECONDS_SLEEP second boundary */
    nextnanosleeptime.tv_nsec += 250000;
    nanoSecondFieldConversion(nextnanosleeptime);

    while (flag[0] || flag[1] || flag[2] || flag[3] || flag[4] || flag[5])
    {
        count++;
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &nextnanosleeptime, NULL);
        //clock_gettime(CLOCK_TAI, &nextnanosleeptime);
        /* Variable to nano Sleep until SECONDS_SLEEP second boundary */
        nextnanosleeptime.tv_nsec += interval * 1000;
        nanoSecondFieldConversion(nextnanosleeptime);
        //usleep(1000);
        ecrt_master_receive(fiber_motor[0].master);
        for (int i = 0; i < FIBER_MOTOR_NUM; i++)
        {
            //接收过程数据
            ecrt_domain_process(fiber_motor[i].domain);

            //读取数据
            fiber_motor[i].status = EC_READ_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.status_word);               //状态字
            fiber_motor[i].opmode = EC_READ_U8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.mode_display);               //运行模式
            fiber_motor[i].currentVelocity = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_velocity); //当前速度
            fiber_motor[i].currentPosition = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_postion);  //当前位置
            fiber_motor[i].currentTorque = EC_READ_S16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_torque);     //当前转距
            //printf("%d号电机当前速度%d||||当前位置%d当前转距%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition, fiber_motor[i].currentTorque);
            if (count == 1)
            {
                a0[i] = fiber_motor[i].currentPosition;
                a1[i] = 0;
                a2[i] = 3 * ((double)position[i] - fiber_motor[i].currentPosition) / (double)(tf * tf);
                //分两次执行除法，更准确
                a3[i] = -2 * ((double)position[i] - fiber_motor[i].currentPosition) / (double)(tf * tf);
                a3[i] = a3[i] / (double)tf;
                //计算线性插补参数
                k[i] = double(position[i] - fiber_motor[i].currentPosition) / tf;
                b[i] = fiber_motor[i].currentPosition;
            }
            if (fiber_motor[i].driveState == dsOperationEnabled &&
                fiber_motor[i].powerBusy == 0 && fiber_motor[i].opmode == 8)
            {
                if (abs(fiber_motor[i].currentPosition - position[i]) < 10)
                    flag[i] = false;
                else
                {
                    if (count < time)
                        //fiber_motor[i].targetPosition = a0[i] + a1[i] * count + a2[i] * count * count + a3[i] * count * count * count; // 根据三次插补计算
                        fiber_motor[i].targetPosition = k[i] * count + b[i]; // 根据线性插补计算
                    //fiber_motor[i].targetPosition = a0 + a1 * 1 + a2 * 1 * 1 + a3 * 1 * 1 * 1;
                    else
                        fiber_motor[i].targetPosition = position[i];
                }
                EC_WRITE_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.target_postion, fiber_motor[i].targetPosition);
            }
            printf("%d号电机当前速度%d||||当前位置%d||||当前转距%d||||目标位置%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition, fiber_motor[i].currentTorque, fiber_motor[i].targetPosition);
            ecrt_domain_queue(fiber_motor[i].domain);
        }

        ecrt_master_send(fiber_motor[0].master);
    }
    return count;
}

//位置模式，将电机移动到指定位置
/**
 * @description: 位置指令控制电机运动到指定位置。
 * @param {MOTOR} *fiber_motor
 * @param {int} position_1～2 电机1～2期望运动位置
 * @param {int} time: 期望执行周期个数，默认为毫秒数。以此来规划路径
 * @param {int} interval 控制周期 单位 us 默认ms
 * @param {int} error_allowed 允许在目标位置绝对值error_allowed误差内完成
 * @return {int} 完成运动所花周期个数
 */
int DoublePosition_planed(struct MOTOR *fiber_motor, int index1, int index2, int position_1, int position_2, int time, int interval = 1000,int error_allowed=10)
{
    bool flag[6] = {true, true, true, true, true, true}; //开始运动
    int position[6] = {0, 0, 0, 0, 0, 0};
    int step[6] = {0, 0, 0, 0, 0, 0};
    position[index1] = position_1;
    position[index2] = position_2;
    int tf = time;

    double a0[6], a1[6], a2[6], a3[6]; //三次插补参数 f=a0+a1*t+a2*t*t+s3*t*t*t;
    double k[6], b[6];                 //线性插补参数  f=k*t+b;
    int count = 0;

    /* Get current time and compute the next nanosleeptime */
    struct timespec nextnanosleeptime;
    clock_gettime(CLOCK_REALTIME, &nextnanosleeptime);
    /* Variable to nano Sleep until SECONDS_SLEEP second boundary */
    nextnanosleeptime.tv_nsec += 250000;
    nanoSecondFieldConversion(nextnanosleeptime);

    while (flag[index1] || flag[index2])
    {
        //usleep(1000);
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &nextnanosleeptime, NULL);
        //clock_gettime(CLOCK_TAI, &nextnanosleeptime);
        /* Variable to nano Sleep until SECONDS_SLEEP second boundary */
        nextnanosleeptime.tv_nsec += interval * 1000;
        nanoSecondFieldConversion(nextnanosleeptime);
        count++;
        ecrt_master_receive(fiber_motor[0].master);
        for (int i = 0; i < 6; i++)
        {
            if (i == index1 || i == index2)
            {

                //接收过程数据
                ecrt_domain_process(fiber_motor[i].domain);

                //读取数据
                fiber_motor[i].status = EC_READ_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.status_word);               //状态字
                fiber_motor[i].opmode = EC_READ_U8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.mode_display);               //运行模式
                fiber_motor[i].currentVelocity = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_velocity); //当前速度
                fiber_motor[i].currentPosition = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_postion);  //当前位置
                fiber_motor[i].currentTorque = EC_READ_S16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_torque);     //当前转距
                printf("%d号电机当前速度%d||||当前位置%d当前转距%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition, fiber_motor[i].currentTorque);
                if (count == 1)
                {
                    a0[i] = fiber_motor[i].currentPosition;
                    a1[i] = 0;
                    a2[i] = 3 * ((double)position[i] - fiber_motor[i].currentPosition) / (double)(tf * tf);
                    a3[i] = -2 * ((double)position[i] - fiber_motor[i].currentPosition) / (double)(tf * tf * tf);
                    //计算线性插补参数
                    k[i] = double(position[i] - fiber_motor[i].currentPosition) / tf;
                    b[i] = fiber_motor[i].currentPosition;
                }
                if (fiber_motor[i].driveState == dsOperationEnabled &&
                    fiber_motor[i].powerBusy == 0 && fiber_motor[i].opmode == 8)
                {
                    if (abs(fiber_motor[i].currentPosition - position[i]) < 10)
                        flag[i] = false;
                    else
                    {
                        if (count < time)
                            //fiber_motor[i].targetPosition = a0[i] + a1[i] * count + a2[i] * count * count + a3[i] * count * count * count; // 根据三次插补计算
                            fiber_motor[i].targetPosition = k[i] * count + b[i]; // 根据线性插补计算
                        //fiber_motor[i].targetPosition = a0 + a1 * 1 + a2 * 1 * 1 + a3 * 1 * 1 * 1;
                        else
                            fiber_motor[i].targetPosition = position_1;
                    }
                    EC_WRITE_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.target_postion, fiber_motor[i].targetPosition);
                }
                ecrt_domain_queue(fiber_motor[i].domain);
            }
            else
            {
                //接收过程数据
                ecrt_domain_process(fiber_motor[i].domain);
                ecrt_domain_queue(fiber_motor[i].domain);
            }
        }
        ecrt_master_send(fiber_motor[0].master);
    }
    return count;
}

//位置模式，将电机移动到指定位置
void Position(struct MOTOR *fiber_motor, int position_1, int position_2, int position_3, int position_4, int position_5, int position_6, int speed)
{

    bool flag[6] = {true, true, true, true, true, true}; //开始运动
    int position[6] = {position_1, position_2, position_3, position_4, position_5, position_6};
    int step = speed * ENCODER_RESOLUTION / TASK_FREQUENCY;

    while (flag[0] || flag[1] || flag[2] || flag[3] || flag[4] || flag[5])
    {

        usleep(1000);
        ecrt_master_receive(fiber_motor[0].master);
        for (int i = 0; i < FIBER_MOTOR_NUM; i++)
        {
            //接收过程数据
            ecrt_domain_process(fiber_motor[i].domain);

            //读取数据
            fiber_motor[i].status = EC_READ_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.status_word);               //状态字
            fiber_motor[i].opmode = EC_READ_U8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.mode_display);               //运行模式
            fiber_motor[i].currentVelocity = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_velocity); //当前速度
            fiber_motor[i].currentPosition = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_postion);  //当前位置
            fiber_motor[i].currentTorque = EC_READ_S16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_torque);     //当前转距
            printf("%d号电机当前速度%d||||当前位置%d当前转距%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition, fiber_motor[i].currentTorque);

            if (fiber_motor[i].driveState == dsOperationEnabled &&
                fiber_motor[i].powerBusy == 0 && fiber_motor[i].opmode == 8)
            {

                if (fiber_motor[i].currentPosition > position[i] + 10)
                {
                    fiber_motor[i].targetPosition = fiber_motor[i].currentPosition - step;
                    if (fiber_motor[i].targetPosition < position[i])
                    {
                        fiber_motor[i].targetPosition = position[i];
                    }
                }
                else if (fiber_motor[i].currentPosition < position[i] - 10)
                {
                    fiber_motor[i].targetPosition = fiber_motor[i].currentPosition + step;
                    if (fiber_motor[i].targetPosition > position[i])
                    {
                        fiber_motor[i].targetPosition = position[i];
                    }
                }
                else
                //if(fiber_motor[i].currentPosition ==  position[i])
                {
                    flag[i] = false;
                }
                EC_WRITE_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.target_postion, fiber_motor[i].targetPosition);
            }
            ecrt_domain_queue(fiber_motor[i].domain);
        }
        ecrt_master_send(fiber_motor[0].master);
    }
}

//位置模式，将电机移动到指定位置
void DoublePosition(struct MOTOR *fiber_motor, int index1, int index2, int position_1, int position_2, int speed1, int speed2)
{
    bool flag[6] = {true, true, true, true, true, true}; //开始运动
    int position[6] = {0, 0, 0, 0, 0, 0};
    int step[6] = {0, 0, 0, 0, 0, 0};
    position[index1] = position_1;
    position[index2] = position_2;
    int step1 = speed1 * ENCODER_RESOLUTION / TASK_FREQUENCY;
    int step2 = speed2 * ENCODER_RESOLUTION / TASK_FREQUENCY;
    step[index1] = step1;
    step[index2] = step2;
    while (flag[index1] || flag[index2])
    {

        usleep(1000);
        ecrt_master_receive(fiber_motor[0].master);
        for (int i = 0; i < 6; i++)
        {
            if (i == index1 || i == index2)
            {
                //接收过程数据
                ecrt_domain_process(fiber_motor[i].domain);

                //读取数据
                fiber_motor[i].status =
                    EC_READ_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.status_word); //状态字
                fiber_motor[i].opmode =
                    EC_READ_U8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.mode_display); //运行模式
                fiber_motor[i].currentVelocity =
                    EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_velocity); //当前速度
                fiber_motor[i].currentPosition =
                    EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_postion); //当前位置
                printf("%d号电机当前速度%d||||当前位置%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition);

                if (fiber_motor[i].driveState == dsOperationEnabled &&
                    fiber_motor[i].powerBusy == 0 && fiber_motor[i].opmode == 8)
                {

                    if (fiber_motor[i].currentPosition > position[i] + 5)
                    {
                        fiber_motor[i].targetPosition = fiber_motor[i].currentPosition - step[i];
                        if (fiber_motor[i].targetPosition < position[i])
                        {
                            fiber_motor[i].targetPosition = position[i];
                        }
                    }
                    else if (fiber_motor[i].currentPosition < position[i] - 5)
                    {
                        fiber_motor[i].targetPosition = fiber_motor[i].currentPosition + step[i];
                        if (fiber_motor[i].targetPosition > position[i])
                        {
                            fiber_motor[i].targetPosition = position[i];
                        }
                    }
                    else
                    //if(fiber_motor[i].currentPosition ==  position[i])
                    {
                        flag[i] = false;
                    }
                    EC_WRITE_S32(fiber_motor[i].domain_pd +
                                     fiber_motor[i].drive_variables.target_postion,
                                 fiber_motor[i].targetPosition);
                }
                ecrt_domain_queue(fiber_motor[i].domain);
            }
            else
            {
                //接收过程数据
                ecrt_domain_process(fiber_motor[i].domain);
                ecrt_domain_queue(fiber_motor[i].domain);
            }
        }
        ecrt_master_send(fiber_motor[0].master);
    }
}

//位置模式，将电机移动到指定位置
void SinglePosition(struct MOTOR *fiber_motor, int index1, int position_1, int speed)
{
    bool flag[6] = {true, true, true, true, true, true}; //开始运动
    int position[6] = {0, 0, 0, 0, 0, 0};
    position[index1] = position_1;
    int step = speed * ENCODER_RESOLUTION / TASK_FREQUENCY;

    while (flag[index1])
    {

        usleep(1000);
        ecrt_master_receive(fiber_motor[0].master);
        for (int i = 0; i < 6; i++)
        {
            if (i == index1)
            {
                //接收过程数据
                ecrt_domain_process(fiber_motor[i].domain);

                //读取数据
                fiber_motor[i].status = EC_READ_U16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.status_word);               //状态字
                fiber_motor[i].opmode = EC_READ_U8(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.mode_display);               //运行模式
                fiber_motor[i].currentVelocity = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_velocity); //当前速度
                fiber_motor[i].currentPosition = EC_READ_S32(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_postion);  //当前位置
                fiber_motor[i].currentTorque = EC_READ_S16(fiber_motor[i].domain_pd + fiber_motor[i].drive_variables.current_torque);     //当前转距
                printf("%d号电机当前速度%d||||当前位置%d当前转距%d\n", i, fiber_motor[i].currentVelocity, fiber_motor[i].currentPosition, fiber_motor[i].currentTorque);

                if (fiber_motor[i].driveState == dsOperationEnabled &&
                    fiber_motor[i].powerBusy == 0 && fiber_motor[i].opmode == 8)
                {

                    if (fiber_motor[i].currentPosition > position[i] + 5)
                    {
                        fiber_motor[i].targetPosition = fiber_motor[i].currentPosition - step;
                        if (fiber_motor[i].targetPosition < position[i])
                        {
                            fiber_motor[i].targetPosition = position[i];
                        }
                    }
                    else if (fiber_motor[i].currentPosition < position[i] - 5)
                    {
                        fiber_motor[i].targetPosition = fiber_motor[i].currentPosition + step;
                        if (fiber_motor[i].targetPosition > position[i])
                        {
                            fiber_motor[i].targetPosition = position[i];
                        }
                    }
                    else
                    //if(fiber_motor[i].currentPosition ==  position[i])
                    {
                        flag[i] = false;
                    }
                    EC_WRITE_S32(fiber_motor[i].domain_pd +
                                     fiber_motor[i].drive_variables.target_postion,
                                 fiber_motor[i].targetPosition);
                }
                ecrt_domain_queue(fiber_motor[i].domain);
            }
            else
            {
                //接收过程数据
                ecrt_domain_process(fiber_motor[i].domain);
                ecrt_domain_queue(fiber_motor[i].domain);
            }
        }
        ecrt_master_send(fiber_motor[0].master);
    }
}
