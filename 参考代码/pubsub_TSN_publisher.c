/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

/**
 * **Trace point setup**
 *
 *            +--------------+                        +----------------+
 *         T1 | OPCUA PubSub |  T8                 T5 | OPCUA loopback |  T4
 *         |  |  Application |  ^                  |  |  Application   |  ^
 *         |  +--------------+  |                  |  +----------------+  |
 *  User   |  |              |  |                  |  |                |  |
 *  Space  |  |              |  |                  |  |                |  |
 *         |  |              |  |                  |  |                |  |
 *------------|--------------|------------------------|----------------|--------
 *         |  |    Node 1    |  |                  |  |     Node 2     |  |
 *  Kernel |  |              |  |                  |  |                |  |
 *  Space  |  |              |  |                  |  |                |  |
 *         |  |              |  |                  |  |                |  |
 *         v  +--------------+  |                  v  +----------------+  |
 *         T2 |  TX tcpdump  |  T7<----------------T6 |   RX tcpdump   |  T3
 *         |  +--------------+                        +----------------+  ^
 *         |                                                              |
 *         ----------------------------------------------------------------
 */
#include <sched.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <sys/io.h>

/* For thread operations */
#include <pthread.h>
/**
 * shared memory
 * created by lcf
 */
#include<sys/shm.h>  // shared memory
#include<sys/sem.h>  // semaphore
#include<sys/msg.h>  // message queue
#include<string.h>   // memcpy
/**
 * shared memory
 * created by lcf
 */


/**
 * mqtt client include
 * created by lcf
 *
#include <string.h>
#include <unistd.h>

#include "MQTTClient.h"
**
 * mqtt client include
 * created by lcf
 */
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <ua_server_internal.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/log.h>
#include <open62541/types_generated.h>
#include <open62541/plugin/pubsub_ethernet.h>
#include <open62541/plugin/pubsub_ethernet_etf.h>

#include "ua_pubsub.h"

#ifdef UA_ENABLE_PUBSUB_ETH_UADP_XDP
#include <open62541/plugin/pubsub_ethernet_xdp.h>
#include <linux/if_link.h>
#endif
/**
 * ethercat master include
 * created by czs
 */
#include <ecrt.h>
#include <math.h>
/**
 * ethercat master include
 * created by czs
 */
UA_NodeId readerGroupIdentifier;
UA_NodeId readerIdentifier;

UA_DataSetReaderConfig readerConfig;

/* to find load of each thread
 * ps -L -o pid,pri,%cpu -C pubsub_TSN_publisher */

/* Configurable Parameters */
/* These defines enables the publisher and subscriber of the OPCUA stack */
/* To run only publisher, enable PUBLISHER define alone (comment SUBSCRIBER) */
#define             PUBLISHER
/* To run only subscriber, enable SUBSCRIBER define alone (comment PUBLISHER) */
#define             SUBSCRIBER
#define             UPDATE_MEASUREMENTS
/* Cycle time in milliseconds */
#define             CYCLE_TIME                            0.25
/* Qbv offset */
#define             QBV_OFFSET                            25 * 1000
#define             SOCKET_PRIORITY                       3
#if defined(PUBLISHER)
#define             PUBLISHER_ID                          2234
#define             WRITER_GROUP_ID                       101
#define             DATA_SET_WRITER_ID                    62541
//#define             PUBLISHING_MAC_ADDRESS                "opc.eth://01-00-5E-7F-00-01:8.3"
#define             PUBLISHING_MAC_ADDRESS                "opc.eth://C4-00-AD-57-51-E6:1.5"
#endif
#if defined(SUBSCRIBER)
#define             PUBLISHER_ID_SUB                      2235
#define             WRITER_GROUP_ID_SUB                   100
#define             DATA_SET_WRITER_ID_SUB                62541
//#define             SUBSCRIBING_MAC_ADDRESS               "opc.eth://01-00-5E-00-00-01:8.3"
#define             SUBSCRIBING_MAC_ADDRESS               "opc.eth://C4-00-AD-57-52-28:1.5"
#endif
#define             REPEATED_NODECOUNTS                   3
#define             PORT_NUMBER                           62541
#define             RECEIVE_QUEUE                         2
#define             XDP_FLAG                              XDP_FLAGS_SKB_MODE
#define             PUBSUB_CONFIG_RT_INFORMATION_MODEL

/* Non-Configurable Parameters */
/* Milli sec and sec conversion to nano sec */
#define             MILLI_SECONDS                         1000 * 1000
#define             SECONDS                               1000 * 1000 * 1000
#define             SECONDS_SLEEP                         5
/* Publisher will sleep for 60% of cycle time and then prepares the */
/* transmission packet within 40% */
#define             NANO_SECONDS_SLEEP_PUB                CYCLE_TIME * MILLI_SECONDS * 0.6
/* Subscriber will wakeup only during start of cycle and check whether */
/* the packets are received */
#define             NANO_SECONDS_SLEEP_SUB                0
/* User application Pub/Sub will wakeup at the 30% of cycle time and handles the */
/* user data such as read and write in Information model */
#define             NANO_SECONDS_SLEEP_USER_APPLICATION   CYCLE_TIME * MILLI_SECONDS * 0.3
/* Priority of Publisher, Subscriber, User application and server are kept */
/* after some prototyping and analyzing it */
#define             PUB_SCHED_PRIORITY                    78
#define             SUB_SCHED_PRIORITY                    81
#define             USERAPPLICATION_SCHED_PRIORITY        75
#define             SERVER_SCHED_PRIORITY                 1
#define             MAX_MEASUREMENTS                      30000000
#define             CORE_TWO                              2
#define             CORE_THREE                            3
#define             SECONDS_INCREMENT                     1
#define             CLOCKID                               CLOCK_TAI
#define             ETH_TRANSPORT_PROFILE                 "http://opcfoundation.org/UA-Profile/Transport/pubsub-eth-uadp"
/**
 * ethercat master define
 * created by czs
 */
// ethercat主站应用程序所用的参数
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
/**
 * ethercat master define
 * created by czs
 */
/* If the Hardcoded publisher/subscriber MAC addresses need to be changed,
 * change PUBLISHING_MAC_ADDRESS and SUBSCRIBING_MAC_ADDRESS
 */

/* Set server running as true */
UA_Boolean          running                = UA_TRUE;
/* Variables corresponding to PubSub connection creation,
 * published data set and writer group */
UA_NodeId           connectionIdent;
UA_NodeId           publishedDataSetIdent;
UA_NodeId           writerGroupIdent;
UA_NodeId           pubNodeID;
UA_NodeId           subNodeID;
UA_NodeId           pubRepeatedCountNodeID;
UA_NodeId           subRepeatedCountNodeID;
/* Variables for counter data handling in address space */
UA_UInt64           *pubCounterData;
UA_DataValue        *pubDataValueRT;
UA_UInt64           *repeatedCounterData[REPEATED_NODECOUNTS];
UA_DataValue        *repeatedDataValueRT[REPEATED_NODECOUNTS];

UA_UInt64           *subCounterData;
UA_DataValue        *subDataValueRT;
UA_UInt64           *subRepeatedCounterData[REPEATED_NODECOUNTS];
UA_DataValue        *subRepeatedDataValueRT[REPEATED_NODECOUNTS];

#if defined(PUBLISHER)
#if defined(UPDATE_MEASUREMENTS)
/* File to store the data and timestamps for different traffic */
FILE               *fpPublisher;
char               *filePublishedData      = "publisher_T1.csv";
/* Array to store published counter data */
UA_UInt64           publishCounterValue[MAX_MEASUREMENTS];
size_t              measurementsPublisher  = 0;
/* Array to store timestamp */
struct timespec     publishTimestamp[MAX_MEASUREMENTS];
#endif
/* Thread for publisher */
pthread_t           pubthreadID;
struct timespec     dataModificationTime;
#endif

#if defined(SUBSCRIBER)
#if defined(UPDATE_MEASUREMENTS)
/* File to store the data and timestamps for different traffic */
FILE               *fpSubscriber;
char               *fileSubscribedData     = "subscriber_T8.csv";
/* Array to store subscribed counter data */
UA_UInt64           subscribeCounterValue[MAX_MEASUREMENTS];
size_t              measurementsSubscriber = 0;
/* Array to store timestamp */
struct timespec     subscribeTimestamp[MAX_MEASUREMENTS];
#endif
/* Thread for subscriber */
pthread_t           subthreadID;
/* Variable for PubSub connection creation */
UA_NodeId           connectionIdentSubscriber;
struct timespec     dataReceiveTime;
#endif

/* Thread for user application*/
pthread_t           userApplicationThreadID;

typedef struct {
UA_Server*                   ServerRun;
} serverConfigStruct;

/* Structure to define thread parameters */
typedef struct {
UA_Server*                   server;
void*                        data;
UA_ServerCallback            callback;
UA_Duration                  interval_ms;
UA_UInt64*                   callbackId;
} threadArg;

enum txtime_flags {
    SOF_TXTIME_DEADLINE_MODE = (1 << 0),
    SOF_TXTIME_REPORT_ERRORS = (1 << 1),

    SOF_TXTIME_FLAGS_LAST = SOF_TXTIME_REPORT_ERRORS,
    SOF_TXTIME_FLAGS_MASK = (SOF_TXTIME_FLAGS_LAST - 1) |
                             SOF_TXTIME_FLAGS_LAST
};
/**
 * shared memory
 * created by lcf
 */
struct command {
    int read_flag;
    int command_flag;
};

struct msg_form {
   long mtype;
  char mtext;
};

 // 联合体，用于semctl初始化
union semun
{
   int              val; /*for SETVAL*/
  struct semid_ds *buf;
  unsigned short  *array;
};

 // 初始化信号量
 int init_sem(int sem_id, int value)
{
   union semun tmp;
   tmp.val = value;
   if(semctl(sem_id, 0, SETVAL, tmp) == -1)
   {
       perror("Init Semaphore Error");
       return -1;
   }
    return 0;
 }
 
 // P操作:
 //  若信号量值为1，获取资源并将信号量值-1 
//  若信号量值为0，进程挂起等待
 int sem_p(int sem_id)
 {
     struct sembuf sbuf;
     sbuf.sem_num = 0; /*序号*/
     sbuf.sem_op = -1; /*P操作*/
     sbuf.sem_flg = SEM_UNDO;
  
      if(semop(sem_id, &sbuf, 1) == -1)
      {
          perror("P operation Error");
          return -1;
      }
      return 0;
  }
  
  // V操作：
  //  释放资源并将信号量值+1
  //  如果有进程正在挂起等待，则唤醒它们
  int sem_v(int sem_id)
  {
      struct sembuf sbuf;
      sbuf.sem_num = 0; /*序号*/
      sbuf.sem_op = 1;  /*V操作*/
      sbuf.sem_flg = SEM_UNDO;
  
      if(semop(sem_id, &sbuf, 1) == -1)
      {
          perror("V operation Error");
          return -1;
      }
      return 0;
  }
  
  // 删除信号量集
  int del_sem(int sem_id)
  {
      union semun tmp;
      if(semctl(sem_id, 0, IPC_RMID, tmp) == -1)
      {
         perror("Delete Semaphore Error");
         return -1;
      }
      return 0;
  }
  
  // 创建一个信号量集
  int creat_sem(key_t key)
  {
      int sem_id;
      if((sem_id = semget(key, 1, IPC_CREAT|0666)) == -1)
      {
          perror("semget error");
          exit(-1);
      }
      init_sem(sem_id, 1);  /*初值设为1资源未占用*/
      return sem_id;
  }
/**
 * shared memory
 * created by lcf
 */

/**
 * ethercat master struct type
 * created by czs
 */
// DS402 CANOpen over EtherCAT 驱动器状态
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
    int8_t opModeSet;         //电机运行模式的设定值,默认位置模式
    int8_t opmode;            //驱动器当前运行模式
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
};
/**
 * ethercat master struct type
 * created by czs
 */

/**
 * ethercat master function definition
 * created by czs
 */
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

//初始化EtherCAT主站函数
static void
init_EtherCAT_master(struct MOTOR *motor) {
    //变量与对应PDO数据对象关联
    ec_pdo_entry_reg_t domain_regs[] = {
        {EP3ESLAVEPOS, MAXSINE, CTRL_WORD, 0, &motor->drive_variables.ctrl_word},
        {EP3ESLAVEPOS, MAXSINE, OPERATION_MODE, 0,
         &motor->drive_variables.operation_mode},
        {EP3ESLAVEPOS, MAXSINE, TARGET_VELOCITY, 0,
         &motor->drive_variables.target_velocity},
        {EP3ESLAVEPOS, MAXSINE, TARGET_POSITION, 0,
         &motor->drive_variables.target_postion},
        {EP3ESLAVEPOS, MAXSINE, STATUS_WORD, 0, &motor->drive_variables.status_word},
        {EP3ESLAVEPOS, MAXSINE, MODE_DISPLAY, 0, &motor->drive_variables.mode_display},
        {EP3ESLAVEPOS, MAXSINE, CURRENT_VELOCITY, 0,
         &motor->drive_variables.current_velocity},
        {EP3ESLAVEPOS, MAXSINE, CURRENT_POSITION, 0,
         &motor->drive_variables.current_postion},
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

    //创建ethercat主站master
    motor->master = ecrt_request_master(0);
    if(!motor->master) {
        printf("Failed to create ethercat master!\n");
        exit(EXIT_FAILURE);  //创建失败，退出线程
    }

    //创建域domain
    motor->domain = ecrt_master_create_domain(motor->master);
    if(!motor->domain) {
        printf("Failed to create master domain!\n");
        exit(EXIT_FAILURE);  //创建失败，退出线程
    }

    //配置从站
    if(!(motor->maxsine_EP3E =
             ecrt_master_slave_config(motor->master, EP3ESLAVEPOS, MAXSINE))) {
        printf("Failed to get slave configuration for EP3E!\n");
        exit(EXIT_FAILURE);  //配置失败，退出线程
    }

    //配置PDOs
    printf("Configuring PDOs...\n");
    if(ecrt_slave_config_pdos(motor->maxsine_EP3E, EC_END, EP3E_syncs)) {
        printf("Failed to configure EP3E PDOs!\n");
        exit(EXIT_FAILURE);  //配置失败，退出线程
    } else {
        printf("*Success to configuring EP3E PDOs*\n");  //配置成功
    }

    //注册PDO entry
    if(ecrt_domain_reg_pdo_entry_list(motor->domain, domain_regs)) {
        printf("PDO entry registration failed!\n");
        exit(EXIT_FAILURE);  //注册失败，退出线程
    } else {
        printf("*Success to configuring EP3E PDO entry*\n");  //注册成功
    }

    //激活主站master
    printf("Activating master...\n");
    if(ecrt_master_activate(motor->master)) {
        exit(EXIT_FAILURE);  //激活失败，退出线程
    } else {
        printf("*Master activated*\n");  //激活成功
    }
    if(!(motor->domain_pd = ecrt_domain_data(motor->domain))) {
        exit(EXIT_FAILURE);
    }
}
/**
 * ethercat master function definition
 * created by czs
 */

/**
 * mqtt client define
 * created by lcf
 *
#define ADDRESS     "tcp://0.0.0.0:1883"
#define CLIENTID    "Node1Sub"
#define TOPIC       "Node1Sub"
#define QOS         2
#define TIMEOUT     10000L*/

UA_Variant true_variant;
UA_Variant false_variant;
UA_Boolean truevalue;
UA_Boolean falsevalue;
UA_UInt64 motor_positon_value = 0;
UA_UInt16 commandflag = 6;
/*
volatile MQTTClient_deliveryToken deliveredtoken;

void
delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int
msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    // printf("Message arrived\n");
    // printf("     topic: %s\n", topicName);
    //  printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
    // printf("   message1: %c\n", *(char *)message->payload);
    // printf("   message2: %s\n", (char *)message->payload);
    switch(*(char *)message->payload) {
        case '0':
            commandflag = 0;
         //  printf("%d\n",commandflag);
            break;
        case '1':
            commandflag = 1;
           // printf("%d\n",commandflag);
            break;
        case '2':
            commandflag = 2;
          //  printf("%d\n",commandflag);
            break;
        case '3':
            commandflag = 3;
           // printf("%d\n",commandflag);
            break;
        case '4':
            commandflag = 4;
          //  printf("%d\n",commandflag);
            break;
        case '5':
            commandflag = 5;
          //  printf("%d\n",commandflag);
            break;
        default:
            break;
    }
    //   printf("命令标号%d\n",commandflag);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void
connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
long int pubcount = 0;

MQTTClient client;
int rc;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
**
 * mqtt client define
 * created by lcf
 */
/* Publisher thread routine for ETF */
void *publisherETF(void *arg);
/* Subscriber thread routine */
void *subscriber(void *arg);
/* User application thread routine */
void *userApplicationPubSub(void *arg);
/* For adding nodes in the server information model */
static void addServerNodes(UA_Server *server);
/* For deleting the nodes created */
static void removeServerNodes(UA_Server *server);
/* To create multi-threads */
static pthread_t threadCreation(UA_Int16 threadPriority, size_t coreAffinity, void *(*thread) (void *),
                                char *applicationName, void *serverConfig);

/* Stop signal */
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = UA_FALSE;
}

/**
 * **Nanosecond field handling**
 *
 * Nanosecond field in timespec is checked for overflowing and one second
 * is added to seconds field and nanosecond field is set to zero
*/

static void nanoSecondFieldConversion(struct timespec *timeSpecValue) {
    /* Check if ns field is greater than '1 ns less than 1sec' */
    while (timeSpecValue->tv_nsec > (SECONDS -1)) {
        /* Move to next second and remove it from ns field */
        timeSpecValue->tv_sec  += SECONDS_INCREMENT;
        timeSpecValue->tv_nsec -= SECONDS;
    }

}

#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
/* If the external data source is written over the information model, the
 * externalDataWriteCallback will be triggered. The user has to take care and assure
 * that the write leads not to synchronization issues and race conditions. */
static UA_StatusCode
externalDataWriteCallback(UA_Server *server, const UA_NodeId *sessionId,
                          void *sessionContext, const UA_NodeId *nodeId,
                          void *nodeContext, const UA_NumericRange *range,
                          const UA_DataValue *data){
    //node values are updated by using variables in the memory
    //UA_Server_write is not used for updating node values.
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
externalDataReadNotificationCallback(UA_Server *server, const UA_NodeId *sessionId,
                                     void *sessionContext, const UA_NodeId *nodeid,
                                     void *nodeContext, const UA_NumericRange *range){
    //allow read without any preparation
    return UA_STATUSCODE_GOOD;
}
#endif

#if defined(SUBSCRIBER)
static void
addPubSubConnectionSubscriber(UA_Server *server, UA_NetworkAddressUrlDataType *networkAddressUrlSubscriber){
    UA_StatusCode    retval                                 = UA_STATUSCODE_GOOD;
    /* Details about the connection configuration and handling are located
     * in the pubsub connection tutorial */
    UA_PubSubConnectionConfig connectionConfig;
    memset(&connectionConfig, 0, sizeof(connectionConfig));
    connectionConfig.name                                   = UA_STRING("Subscriber Connection");
    connectionConfig.enabled                                = UA_TRUE;
#ifdef UA_ENABLE_PUBSUB_ETH_UADP_XDP
    /* Connection options are given as Key/Value Pairs. */
    UA_KeyValuePair connectionOptions[2];
    connectionOptions[0].key                  = UA_QUALIFIEDNAME(0, "xdpflag");
    UA_UInt32 flags                           = XDP_FLAG;
    UA_Variant_setScalar(&connectionOptions[0].value, &flags, &UA_TYPES[UA_TYPES_UINT32]);
    connectionOptions[1].key                  = UA_QUALIFIEDNAME(0, "hwreceivequeue");
    UA_UInt32 rxqueue                         = RECEIVE_QUEUE;
    UA_Variant_setScalar(&connectionOptions[1].value, &rxqueue, &UA_TYPES[UA_TYPES_UINT32]);
    connectionConfig.connectionProperties     = connectionOptions;
    connectionConfig.connectionPropertiesSize = 2;
#endif
    UA_NetworkAddressUrlDataType networkAddressUrlsubscribe = *networkAddressUrlSubscriber;
    connectionConfig.transportProfileUri                    = UA_STRING(ETH_TRANSPORT_PROFILE);
    UA_Variant_setScalar(&connectionConfig.address, &networkAddressUrlsubscribe, &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
    connectionConfig.publisherId.numeric                    = UA_UInt32_random();
    retval |= UA_Server_addPubSubConnection(server, &connectionConfig, &connectionIdentSubscriber);
    if (retval == UA_STATUSCODE_GOOD)
         UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,"The PubSub Connection was created successfully!");
}

/* Add ReaderGroup to the created connection */
static void
addReaderGroup(UA_Server *server) {
    if (server == NULL) {
        return;
    }

    UA_ReaderGroupConfig     readerGroupConfig;
    memset (&readerGroupConfig, 0, sizeof(UA_ReaderGroupConfig));
    readerGroupConfig.name   = UA_STRING("ReaderGroup1");
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
    readerGroupConfig.rtLevel = UA_PUBSUB_RT_FIXED_SIZE;
#endif
    UA_Server_addReaderGroup(server, connectionIdentSubscriber, &readerGroupConfig,
                             &readerGroupIdentifier);
}

/* Set SubscribedDataSet type to TargetVariables data type
 * Add SubscriberCounter variable to the DataSetReader */
static void addSubscribedVariables (UA_Server *server) {
    UA_Int32 iterator = 0;
    if (server == NULL) {
        return;
    }

    UA_FieldTargetVariable *targetVars = (UA_FieldTargetVariable*)
        UA_calloc((REPEATED_NODECOUNTS + 1),
                  sizeof(UA_FieldTargetVariable));
    /* For creating Targetvariable */
    for (iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
    {
        subRepeatedCounterData[iterator] = UA_UInt64_new();
        *subRepeatedCounterData[iterator] = 0;
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
        subRepeatedDataValueRT[iterator] = UA_DataValue_new();
        UA_Variant_setScalar(&subRepeatedDataValueRT[iterator]->value, subRepeatedCounterData[iterator], &UA_TYPES[UA_TYPES_UINT64]);
        subRepeatedDataValueRT[iterator]->hasValue = UA_TRUE;
        /* Set the value backend of the above create node to 'external value source' */
        UA_ValueBackend valueBackend;
        valueBackend.backendType = UA_VALUEBACKENDTYPE_EXTERNAL;
        valueBackend.backend.external.value = &subRepeatedDataValueRT[iterator];
        valueBackend.backend.external.callback.userWrite = externalDataWriteCallback;
        valueBackend.backend.external.callback.notificationRead = externalDataReadNotificationCallback;
        UA_Server_setVariableNode_valueBackend(server, UA_NODEID_NUMERIC(1, (UA_UInt32)iterator+50000), valueBackend);
#endif
        UA_FieldTargetDataType_init(&targetVars[iterator].targetVariable);
        targetVars[iterator].targetVariable.attributeId  = UA_ATTRIBUTEID_VALUE;
        targetVars[iterator].targetVariable.targetNodeId = UA_NODEID_NUMERIC(1, (UA_UInt32)iterator + 50000);
    }

    subCounterData = UA_UInt64_new();
    *subCounterData = 0;
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
    subDataValueRT = UA_DataValue_new();
    UA_Variant_setScalar(&subDataValueRT->value, subCounterData, &UA_TYPES[UA_TYPES_UINT64]);
    subDataValueRT->hasValue = UA_TRUE;
    /* Set the value backend of the above create node to 'external value source' */
    UA_ValueBackend valueBackend;
    valueBackend.backendType = UA_VALUEBACKENDTYPE_EXTERNAL;
    valueBackend.backend.external.value = &subDataValueRT;
    valueBackend.backend.external.callback.userWrite = externalDataWriteCallback;
    valueBackend.backend.external.callback.notificationRead = externalDataReadNotificationCallback;
    UA_Server_setVariableNode_valueBackend(server, subNodeID, valueBackend);
#endif
    UA_FieldTargetDataType_init(&targetVars[iterator].targetVariable);
    targetVars[iterator].targetVariable.attributeId  = UA_ATTRIBUTEID_VALUE;
    targetVars[iterator].targetVariable.targetNodeId = subNodeID;

    /* Set the subscribed data to TargetVariable type */
    readerConfig.subscribedDataSetType = UA_PUBSUB_SDS_TARGET;
    readerConfig.subscribedDataSet.subscribedDataSetTarget.targetVariables = targetVars;
    readerConfig.subscribedDataSet.subscribedDataSetTarget.targetVariablesSize = REPEATED_NODECOUNTS + 1;
}

/* Add DataSetReader to the ReaderGroup */
static void
addDataSetReader(UA_Server *server) {
    UA_Int32 iterator = 0;
    if (server == NULL) {
        return;
    }

    memset (&readerConfig, 0, sizeof(UA_DataSetReaderConfig));
    readerConfig.name                 = UA_STRING("DataSet Reader 1");
    UA_UInt16 publisherIdentifier     = PUBLISHER_ID_SUB;
    readerConfig.publisherId.type     = &UA_TYPES[UA_TYPES_UINT16];
    readerConfig.publisherId.data     = &publisherIdentifier;
    readerConfig.writerGroupId        = WRITER_GROUP_ID_SUB;
    readerConfig.dataSetWriterId      = DATA_SET_WRITER_ID_SUB;

    readerConfig.messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    readerConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPDATASETREADERMESSAGEDATATYPE];
    UA_UadpDataSetReaderMessageDataType *dataSetReaderMessage = UA_UadpDataSetReaderMessageDataType_new();
    dataSetReaderMessage->networkMessageContentMask           = (UA_UadpNetworkMessageContentMask)(UA_UADPNETWORKMESSAGECONTENTMASK_PUBLISHERID |
                                                                 (UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_GROUPHEADER |
                                                                 (UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_WRITERGROUPID |
                                                                 (UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_PAYLOADHEADER);
    readerConfig.messageSettings.content.decoded.data = dataSetReaderMessage;

    /* Setting up Meta data configuration in DataSetReader */
    UA_DataSetMetaDataType *pMetaData = &readerConfig.dataSetMetaData;
    /* FilltestMetadata function in subscriber implementation */
    UA_DataSetMetaDataType_init(pMetaData);
    pMetaData->name                   = UA_STRING ("DataSet Test");
    /* Static definition of number of fields size to 1 to create one
       targetVariable */
    pMetaData->fieldsSize             = REPEATED_NODECOUNTS + 1;
    pMetaData->fields                 = (UA_FieldMetaData*)UA_Array_new (pMetaData->fieldsSize,
                                                                         &UA_TYPES[UA_TYPES_FIELDMETADATA]);

    for (iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
    {
        UA_FieldMetaData_init (&pMetaData->fields[iterator]);
        UA_NodeId_copy (&UA_TYPES[UA_TYPES_UINT64].typeId,
                        &pMetaData->fields[iterator].dataType);
        pMetaData->fields[iterator].builtInType = UA_NS0ID_UINT64;
        pMetaData->fields[iterator].valueRank   = -1; /* scalar */
    }

    /* Unsigned Integer DataType */
    UA_FieldMetaData_init (&pMetaData->fields[iterator]);
    UA_NodeId_copy (&UA_TYPES[UA_TYPES_UINT64].typeId,
                    &pMetaData->fields[iterator].dataType);
    pMetaData->fields[iterator].builtInType = UA_NS0ID_UINT64;
    pMetaData->fields[iterator].valueRank   = -1; /* scalar */

    /* Setup Target Variables in DSR config */
    addSubscribedVariables(server);

    /* Setting up Meta data configuration in DataSetReader */
    UA_Server_addDataSetReader(server, readerGroupIdentifier, &readerConfig,
                               &readerIdentifier);

    UA_free(readerConfig.subscribedDataSet.subscribedDataSetTarget.targetVariables);
    UA_free(readerConfig.dataSetMetaData.fields);
    UA_UadpDataSetReaderMessageDataType_delete(dataSetReaderMessage);
}
#endif

/* Add a callback for cyclic repetition */
UA_StatusCode
UA_PubSubManager_addRepeatedCallback(UA_Server *server, UA_ServerCallback callback,
                                     void *data, UA_Double interval_ms, UA_UInt64 *callbackId) {
    /* Initialize arguments required for the thread to run */
    threadArg *threadArguments = (threadArg *) UA_malloc(sizeof(threadArg));

    /* Pass the value required for the threads */
    threadArguments->server      = server;
    threadArguments->data        = data;
    threadArguments->callback    = callback;
    threadArguments->interval_ms = interval_ms;
    threadArguments->callbackId  = callbackId;

    /* Check the writer group identifier and create the thread accordingly */
    UA_WriterGroup *tmpWriter  = (UA_WriterGroup *) data;
    if(UA_NodeId_equal(&tmpWriter->identifier, &writerGroupIdent)) {
#if defined(PUBLISHER)
        /* Create the publisher thread with the required priority and core affinity */
        char threadNamePub[10] = "Publisher";
        pubthreadID            = threadCreation(PUB_SCHED_PRIORITY, CORE_TWO, publisherETF, threadNamePub, threadArguments);
#endif
    }
    else {
#if defined(SUBSCRIBER)
        /* Create the subscriber thread with the required priority and core affinity */
        char threadNameSub[11] = "Subscriber";
        subthreadID            = threadCreation(SUB_SCHED_PRIORITY, CORE_TWO, subscriber, threadNameSub, threadArguments);
#endif
    }

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_PubSubManager_changeRepeatedCallbackInterval(UA_Server *server, UA_UInt64 callbackId,
                                                UA_Double interval_ms) {
    /* Callback interval need not be modified as it is thread based implementation.
     * The thread uses nanosleep for calculating cycle time and modification in
     * nanosleep value changes cycle time */
    return UA_STATUSCODE_GOOD;
}

/* Remove the callback added for cyclic repetition */
void
UA_PubSubManager_removeRepeatedPubSubCallback(UA_Server *server, UA_UInt64 callbackId) {
    /* TODO Thread exit functions using pthread join and exit */
}

#if defined(PUBLISHER)
/**
 * **PubSub connection handling**
 *
 * Create a new ConnectionConfig. The addPubSubConnection function takes the
 * config and creates a new connection. The Connection identifier is
 * copied to the NodeId parameter.
 */
static void
addPubSubConnection(UA_Server *server, UA_NetworkAddressUrlDataType *networkAddressUrlPub){
    /* Details about the connection configuration and handling are located
     * in the pubsub connection tutorial */
    UA_PubSubConnectionConfig connectionConfig;
    memset(&connectionConfig, 0, sizeof(connectionConfig));
    connectionConfig.name                                   = UA_STRING("Publisher Connection");
    connectionConfig.enabled                                = UA_TRUE;
    UA_NetworkAddressUrlDataType networkAddressUrl          = *networkAddressUrlPub;
    connectionConfig.transportProfileUri                    = UA_STRING(ETH_TRANSPORT_PROFILE);
    UA_Variant_setScalar(&connectionConfig.address, &networkAddressUrl,
                         &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
    connectionConfig.publisherId.numeric                    = PUBLISHER_ID;
    /* ETF configuration settings */
    connectionConfig.etfConfiguration.socketPriority        = SOCKET_PRIORITY;
    connectionConfig.etfConfiguration.sotxtimeEnabled       = UA_TRUE;
    UA_Server_addPubSubConnection(server, &connectionConfig, &connectionIdent);
}

/**
 * **PublishedDataSet handling**
 *
 * Details about the connection configuration and handling are located
 * in the pubsub connection tutorial
 */
static void
addPublishedDataSet(UA_Server *server) {
    UA_PublishedDataSetConfig publishedDataSetConfig;
    memset(&publishedDataSetConfig, 0, sizeof(UA_PublishedDataSetConfig));
    publishedDataSetConfig.publishedDataSetType = UA_PUBSUB_DATASET_PUBLISHEDITEMS;
    publishedDataSetConfig.name                 = UA_STRING("Demo PDS");
    UA_Server_addPublishedDataSet(server, &publishedDataSetConfig, &publishedDataSetIdent);
}

/**
 * **DataSetField handling**
 *
 * The DataSetField (DSF) is part of the PDS and describes exactly one
 * published field.
 */
static void
addDataSetField(UA_Server *server) {
    /* Add a field to the previous created PublishedDataSet */
    UA_NodeId dataSetFieldIdentRepeated;
    UA_DataSetFieldConfig dataSetFieldConfig;
#if defined PUBSUB_CONFIG_FASTPATH_FIXED_OFFSETS
    staticValueSource = UA_DataValue_new();
#endif
    for (UA_Int32 iterator = 0; iterator <  REPEATED_NODECOUNTS; iterator++)
    {
       memset(&dataSetFieldConfig, 0, sizeof(UA_DataSetFieldConfig));
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
       repeatedCounterData[iterator] = UA_UInt64_new();
       *repeatedCounterData[iterator] = 0;
       repeatedDataValueRT[iterator] = UA_DataValue_new();
       UA_Variant_setScalar(&repeatedDataValueRT[iterator]->value, repeatedCounterData[iterator], &UA_TYPES[UA_TYPES_UINT64]);
       repeatedDataValueRT[iterator]->hasValue = UA_TRUE;
       /* Set the value backend of the above create node to 'external value source' */
       UA_ValueBackend valueBackend;
       valueBackend.backendType = UA_VALUEBACKENDTYPE_EXTERNAL;
       valueBackend.backend.external.value = &repeatedDataValueRT[iterator];
       valueBackend.backend.external.callback.userWrite = externalDataWriteCallback;
       valueBackend.backend.external.callback.notificationRead = externalDataReadNotificationCallback;
       UA_Server_setVariableNode_valueBackend(server, UA_NODEID_NUMERIC(1, (UA_UInt32)iterator+10000), valueBackend);
       /* setup RT DataSetField config */
       dataSetFieldConfig.field.variable.rtValueSource.rtInformationModelNode = UA_TRUE;
       dataSetFieldConfig.field.variable.publishParameters.publishedVariable = UA_NODEID_NUMERIC(1, (UA_UInt32)iterator+10000);
#else
       repeatedCounterData[iterator] = UA_UInt64_new();
       dataSetFieldConfig.dataSetFieldType                                   = UA_PUBSUB_DATASETFIELD_VARIABLE;
       dataSetFieldConfig.field.variable.fieldNameAlias                      = UA_STRING("Repeated Counter Variable");
       dataSetFieldConfig.field.variable.promotedField                       = UA_FALSE;
       dataSetFieldConfig.field.variable.publishParameters.publishedVariable = UA_NODEID_NUMERIC(1, (UA_UInt32)iterator+10000);
       dataSetFieldConfig.field.variable.publishParameters.attributeId       = UA_ATTRIBUTEID_VALUE;
#endif
       UA_Server_addDataSetField(server, publishedDataSetIdent, &dataSetFieldConfig, &dataSetFieldIdentRepeated);
   }

    UA_NodeId dataSetFieldIdent;
    UA_DataSetFieldConfig dsfConfig;
    memset(&dsfConfig, 0, sizeof(UA_DataSetFieldConfig));
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
    pubCounterData = UA_UInt64_new();
    *pubCounterData = 0;
    pubDataValueRT = UA_DataValue_new();
    UA_Variant_setScalar(&pubDataValueRT->value, pubCounterData, &UA_TYPES[UA_TYPES_UINT64]);
    pubDataValueRT->hasValue = UA_TRUE;
    /* Set the value backend of the above create node to 'external value source' */
    UA_ValueBackend valueBackend;
    valueBackend.backendType = UA_VALUEBACKENDTYPE_EXTERNAL;
    valueBackend.backend.external.value = &pubDataValueRT;
    valueBackend.backend.external.callback.userWrite = externalDataWriteCallback;
    valueBackend.backend.external.callback.notificationRead = externalDataReadNotificationCallback;
    UA_Server_setVariableNode_valueBackend(server, pubNodeID, valueBackend);
    /* setup RT DataSetField config */
    dsfConfig.field.variable.rtValueSource.rtInformationModelNode = UA_TRUE;
    dsfConfig.field.variable.publishParameters.publishedVariable = pubNodeID;
#else
    pubCounterData = UA_UInt64_new();
    dsfConfig.dataSetFieldType                                   = UA_PUBSUB_DATASETFIELD_VARIABLE;
    dsfConfig.field.variable.fieldNameAlias                      = UA_STRING("Counter Variable");
    dsfConfig.field.variable.promotedField                       = UA_FALSE;
    dsfConfig.field.variable.publishParameters.publishedVariable = pubNodeID;
    dsfConfig.field.variable.publishParameters.attributeId       = UA_ATTRIBUTEID_VALUE;
#endif
    UA_Server_addDataSetField(server, publishedDataSetIdent, &dsfConfig, &dataSetFieldIdent);

}

/**
 * **WriterGroup handling**
 *
 * The WriterGroup (WG) is part of the connection and contains the primary
 * configuration parameters for the message creation.
 */
static void
addWriterGroup(UA_Server *server) {
    UA_WriterGroupConfig writerGroupConfig;
    memset(&writerGroupConfig, 0, sizeof(UA_WriterGroupConfig));
    writerGroupConfig.name               = UA_STRING("Demo WriterGroup");
    writerGroupConfig.publishingInterval = CYCLE_TIME;
    writerGroupConfig.enabled            = UA_FALSE;
    writerGroupConfig.encodingMimeType   = UA_PUBSUB_ENCODING_UADP;
    writerGroupConfig.writerGroupId      = WRITER_GROUP_ID;
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
    writerGroupConfig.rtLevel            = UA_PUBSUB_RT_FIXED_SIZE;
#endif
    writerGroupConfig.messageSettings.encoding             = UA_EXTENSIONOBJECT_DECODED;
    writerGroupConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPWRITERGROUPMESSAGEDATATYPE];
    /* The configuration flags for the messages are encapsulated inside the
     * message- and transport settings extension objects. These extension
     * objects are defined by the standard. e.g.
     * UadpWriterGroupMessageDataType */
    UA_UadpWriterGroupMessageDataType *writerGroupMessage  = UA_UadpWriterGroupMessageDataType_new();
    /* Change message settings of writerGroup to send PublisherId,
     * WriterGroupId in GroupHeader and DataSetWriterId in PayloadHeader
     * of NetworkMessage */
    writerGroupMessage->networkMessageContentMask          = (UA_UadpNetworkMessageContentMask)(UA_UADPNETWORKMESSAGECONTENTMASK_PUBLISHERID |
                                                              (UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_GROUPHEADER |
                                                              (UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_WRITERGROUPID |
                                                              (UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_PAYLOADHEADER);
    writerGroupConfig.messageSettings.content.decoded.data = writerGroupMessage;
    UA_Server_addWriterGroup(server, connectionIdent, &writerGroupConfig, &writerGroupIdent);
    UA_Server_setWriterGroupOperational(server, writerGroupIdent);
    UA_UadpWriterGroupMessageDataType_delete(writerGroupMessage);
}

/**
 * **DataSetWriter handling**
 *
 * A DataSetWriter (DSW) is the glue between the WG and the PDS. The DSW is
 * linked to exactly one PDS and contains additional informations for the
 * message generation.
 */
static void
addDataSetWriter(UA_Server *server) {
    UA_NodeId dataSetWriterIdent;
    UA_DataSetWriterConfig dataSetWriterConfig;
    memset(&dataSetWriterConfig, 0, sizeof(UA_DataSetWriterConfig));
    dataSetWriterConfig.name            = UA_STRING("Demo DataSetWriter");
    dataSetWriterConfig.dataSetWriterId = DATA_SET_WRITER_ID;
    dataSetWriterConfig.keyFrameCount   = 10;
    UA_Server_addDataSetWriter(server, writerGroupIdent, publishedDataSetIdent,
                               &dataSetWriterConfig, &dataSetWriterIdent);
}

#if defined(UPDATE_MEASUREMENTS)
/**
 * **Published data handling**
 *
 * The published data is updated in the array using this function
 */
#if defined(PUBLISHER)
static void
updateMeasurementsPublisher(struct timespec start_time,
                            UA_UInt64 counterValue) {
    publishTimestamp[measurementsPublisher]        = start_time;
    publishCounterValue[measurementsPublisher]     = counterValue;
    measurementsPublisher++;
}
#endif
#if defined(SUBSCRIBER)
/**
 * Subscribed data handling**
 * The subscribed data is updated in the array using this function Subscribed data handling**
 */
static void
updateMeasurementsSubscriber(struct timespec receive_time, UA_UInt64 counterValue) {
    subscribeTimestamp[measurementsSubscriber]     = receive_time;
    subscribeCounterValue[measurementsSubscriber]  = counterValue;
    measurementsSubscriber++;
}
#endif
#endif

/**
 * **Publisher thread routine**
 *
 * The publisherETF function is the routine used by the publisher thread.
 * This routine publishes the data at a cycle time of 250us.
 */
void *publisherETF(void *arg) {
    struct timespec   nextnanosleeptime;
    UA_ServerCallback pubCallback;
    UA_Server*        server;
    UA_WriterGroup*   currentWriterGroup;
    UA_UInt64         interval_ns;
    UA_UInt64         transmission_time;

    /* Initialise value for nextnanosleeptime timespec */
    nextnanosleeptime.tv_nsec                      = 0;

    threadArg *threadArgumentsPublisher = (threadArg *)arg;
    server                              = threadArgumentsPublisher->server;
    pubCallback                         = threadArgumentsPublisher->callback;
    currentWriterGroup                  = (UA_WriterGroup *)threadArgumentsPublisher->data;
    interval_ns                         = (threadArgumentsPublisher->interval_ms * MILLI_SECONDS);

    /* Get current time and compute the next nanosleeptime */
    clock_gettime(CLOCKID, &nextnanosleeptime);
    /* Variable to nano Sleep until 1ms before a 1 second boundary */
    nextnanosleeptime.tv_sec                      += SECONDS_SLEEP;
    nextnanosleeptime.tv_nsec                      = NANO_SECONDS_SLEEP_PUB;
    nanoSecondFieldConversion(&nextnanosleeptime);

    /* Define Ethernet ETF transport settings */
    UA_EthernetETFWriterGroupTransportDataType ethernetETFtransportSettings;
    memset(&ethernetETFtransportSettings, 0, sizeof(UA_EthernetETFWriterGroupTransportDataType));
    /* TODO: Txtime enable shall be configured based on connectionConfig.etfConfiguration.sotxtimeEnabled parameter */
    ethernetETFtransportSettings.txtime_enabled    = UA_TRUE;
    ethernetETFtransportSettings.transmission_time = 0;

    /* Encapsulate ETF config in transportSettings */
    UA_ExtensionObject transportSettings;
    memset(&transportSettings, 0, sizeof(UA_ExtensionObject));
    /* TODO: transportSettings encoding and type to be defined */
    transportSettings.content.decoded.data       = &ethernetETFtransportSettings;
    currentWriterGroup->config.transportSettings = transportSettings;
    UA_UInt64 roundOffCycleTime                  = (CYCLE_TIME * MILLI_SECONDS) - NANO_SECONDS_SLEEP_PUB;

    while (running) {
        clock_nanosleep(CLOCKID, TIMER_ABSTIME, &nextnanosleeptime, NULL);
        transmission_time                              = ((UA_UInt64)nextnanosleeptime.tv_sec * SECONDS + (UA_UInt64)nextnanosleeptime.tv_nsec) + roundOffCycleTime + QBV_OFFSET;
        ethernetETFtransportSettings.transmission_time = transmission_time;
        pubCallback(server, currentWriterGroup);
        nextnanosleeptime.tv_nsec                     += interval_ns;
        nanoSecondFieldConversion(&nextnanosleeptime);
    }

    UA_free(threadArgumentsPublisher);

    return (void*)NULL;
}
#endif

#if defined(SUBSCRIBER)
/**
 * **Subscriber thread routine**
 *
 * The subscriber function is the routine used by the subscriber thread.
 */

void *subscriber(void *arg) {
    UA_Server*        server;
    UA_ReaderGroup*   currentReaderGroup;
    UA_ServerCallback subCallback;
    struct timespec   nextnanosleeptimeSub;

    threadArg *threadArgumentsSubscriber = (threadArg *)arg;
    server                               = threadArgumentsSubscriber->server;
    subCallback                          = threadArgumentsSubscriber->callback;
    currentReaderGroup                   = (UA_ReaderGroup *)threadArgumentsSubscriber->data;

    /* Get current time and compute the next nanosleeptime */
    clock_gettime(CLOCKID, &nextnanosleeptimeSub);
    /* Variable to nano Sleep until 1ms before a 1 second boundary */
    nextnanosleeptimeSub.tv_sec         += SECONDS_SLEEP;
    nextnanosleeptimeSub.tv_nsec         = NANO_SECONDS_SLEEP_SUB;
    nanoSecondFieldConversion(&nextnanosleeptimeSub);
    while (running) {
        clock_nanosleep(CLOCKID, TIMER_ABSTIME, &nextnanosleeptimeSub, NULL);
        /* Read subscribed data from the SubscriberCounter variable */
        subCallback(server, currentReaderGroup);
        nextnanosleeptimeSub.tv_nsec += (CYCLE_TIME * MILLI_SECONDS);
        nanoSecondFieldConversion(&nextnanosleeptimeSub);
    }

    UA_free(threadArgumentsSubscriber);

    return (void*)NULL;
}
#endif

#if defined(PUBLISHER) || defined(SUBSCRIBER)
/**
 * **UserApplication thread routine**
 *
 */
void *userApplicationPubSub(void *arg) {
    UA_UInt64  repeatedCounterValue = 0;
    struct timespec nextnanosleeptimeUserApplication;
    /* Get current time and compute the next nanosleeptime */
    clock_gettime(CLOCKID, &nextnanosleeptimeUserApplication);
    /* Variable to nano Sleep until 1ms before a 1 second boundary */
    nextnanosleeptimeUserApplication.tv_sec                      += SECONDS_SLEEP;
    nextnanosleeptimeUserApplication.tv_nsec                      = NANO_SECONDS_SLEEP_USER_APPLICATION;
    nanoSecondFieldConversion(&nextnanosleeptimeUserApplication);
    *pubCounterData      = 0;
    /**
     * shared memory
     * created by lcf
     */
     key_t key;
     int shmid, semid, msqid;
     struct command *shm;
     struct shmid_ds buf1;  /*用于删除共享内存*/
     int shared_read_count = 0;
 
     // 获取key值
     if((key = ftok(".", 'z')) < 0)
     {
         perror("ftok error");
         exit(1);
     }
 
     // 创建共享内存
     if((shmid = shmget(key, 1024, IPC_CREAT|0666)) == -1)
     {
         perror("Create Shared Memory Error");
         exit(1);
     }
 
     // 连接共享内存
     shm = (struct command*)shmat(shmid, 0, 0);
     if((int)shm == -1)
     {
         perror("Attach Shared Memory Error");
         exit(1);
     }
 
     // 创建信号量
     semid = creat_sem(key);    

    /**
     * shared memory
     * created by lcf
     */
     /**
     * ethercat master init
     * created by czs
     */
    struct MOTOR motor;
    init_EtherCAT_master(&motor);
    printf("*It's working now*\n");
    motor.targetPosition = 0;
    motor.opModeSet = 8;         //位置模式
    int8_t reset_count = 0;      //复位的时候计数用
    int16_t position_count = 0;  //位置模式下计数
    double t = 0.00;
    /**
     * ethercat master init
     * created by czs
     */
    for (UA_Int32 iterator = 0; iterator <  REPEATED_NODECOUNTS; iterator++)
    {
        *repeatedCounterData[iterator] = repeatedCounterValue;
    }
#ifndef PUBSUB_CONFIG_RT_INFORMATION_MODEL
    UA_Server* server;
    serverConfigStruct *serverConfig = (serverConfigStruct*)arg;
    server = serverConfig->ServerRun;
#endif
    while (running) {
        clock_nanosleep(CLOCKID, TIMER_ABSTIME, &nextnanosleeptimeUserApplication, NULL);
        /**
        * shared memory
        * created by lcf
        */
        shared_read_count = shared_read_count + 1;
        if(shared_read_count >= 4000)
        {
            shared_read_count = 0;
            sem_p(semid);
            if((*shm).read_flag == 1)
            {
                commandflag = (*shm).command_flag;
                (*shm).read_flag = 0;
            }     
            sem_v(semid);
        }
       
        /**
        * shared memory
        * created by lcf
        */
        
        /*用户按钮按下，更改地址空间*/
        switch(commandflag) {
            case 0:
                commandflag = 6;
                if(*repeatedCounterData[1]%10==0)
                *repeatedCounterData[1] += 1;
               //  printf("使能了%ld\n",*repeatedCounterData[1]);
                motor.powerBusy = true;
                break;
            case 1:
                commandflag = 6;
                 if((*repeatedCounterData[1]%100)/10==0)
                *repeatedCounterData[1] += 10;
                  if((*repeatedCounterData[1]%100000)/10000==1)
                     *repeatedCounterData[1] -= 10000;
                //printf("回零了并且去运动%ld\n",*repeatedCounterData[1]);

                motor.homeBusy = true;
                //  printf("22222222222222222\n");
                break;
            case 2:
                commandflag = 6;
                 if((*repeatedCounterData[1]%1000)/100==0)
                *repeatedCounterData[1] += 100;
                motor.resetBusy = true;
                //   printf("333333333333333333\n");
                break;
            case 3:
                commandflag = 6;
                 if((*repeatedCounterData[1]%10000)/1000==0)
                *repeatedCounterData[1] += 1000;
                 if(*repeatedCounterData[1]%10==1)
                *repeatedCounterData[1] -= 1;
                 if((*repeatedCounterData[1]%100000)/10000==1)
                     *repeatedCounterData[1] -= 10000;
// printf("急停了并且去使能去运动%ld\n",*repeatedCounterData[1]);
                motor.quickStopBusy = true;
                //  printf("444444444444444444\n");
                break;
            case 4:
                commandflag = 6;
                 if((*repeatedCounterData[1]%100000)/10000==0)
                *repeatedCounterData[1] += 10000;
               //    printf("运动了%ld\n",*repeatedCounterData[1]);
                motor.positionMoving = true;
                //    printf("55555555555555555555\n");
                break;
            case 5:
                commandflag = 6;
                 if((*repeatedCounterData[1]%100000)/10000==1)
                *repeatedCounterData[1] -= 10000;
              //     printf("停止运动了%ld\n",*repeatedCounterData[1]);

                motor.positionMoving = false;
                break;
            case 6:   
                if((*repeatedCounterData[1]%10)==1)//使能只发一次
                        *repeatedCounterData[1] -= 1;
                         if((*repeatedCounterData[1]%100)/10==1)//回零只发一次
                        *repeatedCounterData[1] -= 10;
                    if((*repeatedCounterData[1]%10000)/1000==1)//急停只发一次
                        *repeatedCounterData[1] -= 1000;
                        if((*repeatedCounterData[1]%1000)/100==1)//复位只发一次
                        *repeatedCounterData[1] -= 100;
                        break;
                }
            /*用户按钮按下，更改地址空间*/

         /**
         * ethercat master program
         * created by czs
         */
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

        // DS402 CANOpen over EtherCAT status machine
        //获取当前驱动器的驱动状态
        if((motor.status & 0x004F) == 0x0000)
            motor.driveState = dsNotReadyToSwitchOn;  //初始化 未完成状态
        else if((motor.status & 0x004F) == 0x0040)
            motor.driveState = dsSwitchOnDisabled;  //初始化 完成状态
        else if((motor.status & 0x006F) == 0x0021)
            motor.driveState = dsReadyToSwitchOn;  //主电路电源OFF状态
        else if((motor.status & 0x006F) == 0x0023)
            motor.driveState = dsSwitchedOn;  //伺服OFF/伺服准备
        else if((motor.status & 0x006F) == 0x0027)
            motor.driveState = dsOperationEnabled;  //伺服ON
        else if((motor.status & 0x006F) == 0x0007)
            motor.driveState = dsQuickStopActive;  //即停止
        else if((motor.status & 0x004F) == 0x000F)
            motor.driveState = dsFaultReactionActive;  //异常（报警）判断
        else if((motor.status & 0x004F) == 0x0008)
            motor.driveState = dsFault;  //异常（报警）状态

        //轮询检测是否在进行复位、急停或者使能
        //复位
        if(motor.resetBusy == true) {
            reset_count = reset_count + 1;
            if(reset_count == 1) {
                EC_WRITE_U16(motor.domain_pd + motor.drive_variables.ctrl_word, 0x0);
            } else {
                EC_WRITE_U16(motor.domain_pd + motor.drive_variables.ctrl_word, 0x0080);
                if(motor.driveState == dsSwitchOnDisabled) {
                    motor.resetBusy = false;
                    reset_count = 0;
                }
            }
            // UA_Server_writeValue(server, resetNode, false_variant);

        }  //急停
        else if(motor.quickStopBusy == true) {
            //控制驱动器的状态转化到Switch On Disabled
            switch(motor.driveState) {
                case dsSwitchedOn:
                case dsReadyToSwitchOn:
                case dsOperationEnabled:
                    EC_WRITE_U16(motor.domain_pd + motor.drive_variables.ctrl_word,
                                 0x0);  // Disable Voltage
                    break;

                default:
                    motor.quickStopBusy = false;
                    motor.positionMoving = false;
            }

        }  //使能
        else if(motor.powerBusy == true) {
            switch(motor.driveState) {
                case dsNotReadyToSwitchOn:
                    break;

                case dsSwitchOnDisabled:
                    //设置运行模式为位置模式
                    EC_WRITE_S8(motor.domain_pd + motor.drive_variables.operation_mode,
                                motor.opModeSet);
                    EC_WRITE_U16(motor.domain_pd + motor.drive_variables.ctrl_word,
                                 0x0006);
                    break;

                case dsReadyToSwitchOn:
                    EC_WRITE_U16(motor.domain_pd + motor.drive_variables.ctrl_word,
                                 0x0007);
                    break;

                case dsSwitchedOn:
                    EC_WRITE_U16(motor.domain_pd + motor.drive_variables.ctrl_word,
                                 0x000f);  // enable operation
                    motor.targetPosition =
                        motor
                            .currentPosition;  //将当前位置复制给目标位置，防止使能后电机震动
                    EC_WRITE_S32(motor.domain_pd + motor.drive_variables.target_postion,
                                 motor.targetPosition);
                    break;
                default:
                    motor.powerBusy = false;
            }
        }

        if(motor.driveState == dsOperationEnabled && motor.resetBusy == 0 &&
           motor.powerBusy == 0 && motor.quickStopBusy == 0) {
            if(motor.opmode == 8) {  //位置模式
                //    printf("电机当前位置%d\n",motor.currentPosition);
                if(motor.homeBusy == true) {  //开始回零
                    // printf("电机实时位置%d\t,电机目标位置%d\n",motor.currentPosition,motor.targetPosition);
                    if(motor.currentPosition > 0) {
                        motor.targetPosition = motor.currentPosition - HOME_STEP;
                        if(motor.targetPosition < 0) {
                            motor.targetPosition = 0;
                        }
                    } else if(motor.currentPosition < 0) {
                        motor.targetPosition = motor.currentPosition + HOME_STEP;
                        if(motor.targetPosition > 0) {
                            motor.targetPosition = 0;
                        }
                    } else if(motor.currentPosition == 0) {
                        motor.homeBusy = false;  //回零结束
                        motor.positionMoving = false;
                        position_count = 0;
                        t = 0;
                    }
                    EC_WRITE_S32(motor.domain_pd + motor.drive_variables.target_postion,
                                 motor.targetPosition);
                } else {
                    if(motor.positionMoving == true) {  //开始运动
                        if(position_count == 8000) {
                            position_count = 0;
                            t = 0;
                        }
                        motor.targetPosition =
                            (int32_t)(ENCODER_RESOLUTION * sin(Pi * t) /
                                      2);  //位置模式时传送位置信息
                        // printf("电机实时位置%d\t,电机目标位置%d\n",motor.currentPosition,motor.targetPosition);
                        EC_WRITE_S32(motor.domain_pd +
                                         motor.drive_variables.target_postion,
                                     motor.targetPosition);
                        //写入地址空间start
                        *repeatedCounterData[0] = (UA_UInt64)motor.targetPosition;
                        //写入地址空间end
                        t = t + (double)POSITION_STEP;
                        //     printf("buchang%f\t",POSITION_STEP);
                        position_count = position_count + 1;
                    } else {
                    }
                }
            }
            EC_WRITE_U16(motor.domain_pd + motor.drive_variables.ctrl_word, 0x001f);
        }

        //发送过程数据
        ecrt_domain_queue(motor.domain);
        ecrt_master_send(motor.master);
        /**
         * ethercat master program
         * created by czs
         */
#if defined(PUBLISHER)
        *pubCounterData      = *pubCounterData + 1;
        /*for (UA_Int32 iterator = 0; iterator <  REPEATED_NODECOUNTS - 1; iterator++)
            *repeatedCounterData[iterator] = *repeatedCounterData[iterator] + 1;*/
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
        clock_gettime(CLOCKID, &dataModificationTime);
        *repeatedCounterData[2] = (uint64_t)dataModificationTime.tv_sec * SECONDS + (uint64_t)dataModificationTime.tv_nsec;
#else
        UA_Variant pubCounter;
        UA_Variant_init(&pubCounter);
        UA_Variant_setScalar(&pubCounter, pubCounterData, &UA_TYPES[UA_TYPES_UINT64]);
        UA_NodeId currentNodeId = UA_NODEID_STRING(1, "PublisherCounter");
        UA_Server_writeValue(server, currentNodeId, pubCounter);
        clock_gettime(CLOCKID, &dataModificationTime);
        for (UA_Int32 iterator = 0; iterator <  REPEATED_NODECOUNTS; iterator++)
        {
            UA_Variant rpCounter;
            UA_Variant_init(&rpCounter);
            UA_Variant_setScalar(&rpCounter, repeatedCounterData[iterator], &UA_TYPES[UA_TYPES_UINT64]);
            UA_Server_writeValue(server, UA_NODEID_NUMERIC(1, (UA_UInt32)iterator+10000), rpCounter);
        }
#endif
#endif
#if defined(SUBSCRIBER)
#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
        clock_gettime(CLOCKID, &dataReceiveTime);
#else
        const UA_NodeId nodeid  = UA_NODEID_STRING(1,"SubscriberCounter");
        UA_Variant subCounter;
        UA_Variant_init(&subCounter);
        UA_Server_readValue(server, nodeid, &subCounter);
        clock_gettime(CLOCKID, &dataReceiveTime);
        *subCounterData          = *(UA_UInt64 *)subCounter.data;
        UA_Variant_clear(&subCounter);
#endif
#if defined(UPDATE_MEASUREMENTS)
        updateMeasurementsPublisher(dataModificationTime, *pubCounterData);
        if (*subCounterData > 0&&measurementsSubscriber-1>=0&& *subCounterData != subscribeCounterValue[measurementsSubscriber-1])//T8去重复
            updateMeasurementsSubscriber(dataReceiveTime, *subCounterData);
#endif
#endif
        nextnanosleeptimeUserApplication.tv_nsec += (CYCLE_TIME * MILLI_SECONDS);
        nanoSecondFieldConversion(&nextnanosleeptimeUserApplication);
    }
    /**
     * shared memory
     * created by lcf
     */
    // 断开连接
     shmdt(shm);
 
     /*删除共享内存、消息队列、信号量*/
     shmctl(shmid, IPC_RMID, &buf1);
     del_sem(semid);
    /**
     * shared memory
     * created by lcf
     */
     
     /**
     * ethercat master close
     * created by czs
     */
     ecrt_master_deactivate(motor.master);
     /**
     * ethercat master close
     * created by czs
     */
    return (void*)NULL;
}
#endif
/**
 * **Deletion of nodes**
 *
 * The removeServerNodes function is used to delete the publisher and subscriber
 * nodes.
 */
static void removeServerNodes(UA_Server *server) {
    /* Delete the Publisher Counter Node*/
    UA_Server_deleteNode(server, pubNodeID, UA_TRUE);
    UA_NodeId_clear(&pubNodeID);
    for (UA_Int32 iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
    {
        UA_Server_deleteNode(server, pubRepeatedCountNodeID, UA_TRUE);
        UA_NodeId_clear(&pubRepeatedCountNodeID);
    }

    UA_Server_deleteNode(server, subNodeID, UA_TRUE);
    UA_NodeId_clear(&subNodeID);
    for (UA_Int32 iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
    {
        UA_Server_deleteNode(server, subRepeatedCountNodeID, UA_TRUE);
        UA_NodeId_clear(&subRepeatedCountNodeID);
    }
}

static pthread_t threadCreation(UA_Int16 threadPriority, size_t coreAffinity, void *(*thread) (void *), char *applicationName, void *serverConfig){

    /* Core affinity set */
    cpu_set_t           cpuset;
    pthread_t           threadID;
    struct sched_param  schedParam;
    UA_Int32         returnValue         = 0;
    UA_Int32         errorSetAffinity    = 0;
    /* Return the ID for thread */
    threadID = pthread_self();
    schedParam.sched_priority = threadPriority;
    returnValue = pthread_setschedparam(threadID, SCHED_FIFO, &schedParam);
    if (returnValue != 0) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"pthread_setschedparam: failed\n");
        exit(1);
    }
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,\
                "\npthread_setschedparam:%s Thread priority is %d \n", \
                applicationName, schedParam.sched_priority);
    CPU_ZERO(&cpuset);
    CPU_SET(coreAffinity, &cpuset);
    errorSetAffinity = pthread_setaffinity_np(threadID, sizeof(cpu_set_t), &cpuset);
    if (errorSetAffinity) {
        fprintf(stderr, "pthread_setaffinity_np: %s\n", strerror(errorSetAffinity));
        exit(1);
    }

    returnValue = pthread_create(&threadID, NULL, thread, serverConfig);
    if (returnValue != 0) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,":%s Cannot create thread\n", applicationName);
    }

    if (CPU_ISSET(coreAffinity, &cpuset)) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"%s CPU CORE: %ld\n", applicationName, coreAffinity);
    }

   return threadID;

}
/**
 * **Creation of nodes**
 *
 * The addServerNodes function is used to create the publisher and subscriber
 * nodes.
 */
static void addServerNodes(UA_Server *server) {
    UA_NodeId objectId;
    UA_NodeId newNodeId;
    UA_ObjectAttributes object           = UA_ObjectAttributes_default;
    object.displayName                   = UA_LOCALIZEDTEXT("en-US", "Counter Object");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Counter Object"), UA_NODEID_NULL,
                            object, NULL, &objectId);
    UA_VariableAttributes publisherAttr  = UA_VariableAttributes_default;
    UA_UInt64 publishValue               = 0;
    publisherAttr.accessLevel            = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Variant_setScalar(&publisherAttr.value, &publishValue, &UA_TYPES[UA_TYPES_UINT64]);
    publisherAttr.displayName            = UA_LOCALIZEDTEXT("en-US", "Publisher Counter");
    publisherAttr.dataType               = UA_TYPES[UA_TYPES_UINT64].typeId;
    newNodeId                            = UA_NODEID_STRING(1, "PublisherCounter");
    UA_Server_addVariableNode(server, newNodeId, objectId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "Publisher Counter"),
                              UA_NODEID_NULL, publisherAttr, NULL, &pubNodeID);
    UA_VariableAttributes subscriberAttr = UA_VariableAttributes_default;
    UA_UInt64 subscribeValue             = 0;
    subscriberAttr.accessLevel           = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Variant_setScalar(&subscriberAttr.value, &subscribeValue, &UA_TYPES[UA_TYPES_UINT64]);
    subscriberAttr.displayName           = UA_LOCALIZEDTEXT("en-US", "Subscriber Counter");
    subscriberAttr.dataType              = UA_TYPES[UA_TYPES_UINT64].typeId;
    newNodeId                            = UA_NODEID_STRING(1, "SubscriberCounter");
    UA_Server_addVariableNode(server, newNodeId, objectId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "Subscriber Counter"),
                              UA_NODEID_NULL, subscriberAttr, NULL, &subNodeID);
    for (UA_Int32 iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
    {
        UA_VariableAttributes repeatedNodePub = UA_VariableAttributes_default;
        UA_UInt64 repeatedPublishValue        = 0;
        repeatedNodePub.accessLevel           = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        UA_Variant_setScalar(&repeatedNodePub.value, &repeatedPublishValue, &UA_TYPES[UA_TYPES_UINT64]);
        repeatedNodePub.displayName           = UA_LOCALIZEDTEXT("en-US", "Publisher RepeatedCounter");
        repeatedNodePub.dataType              = UA_TYPES[UA_TYPES_UINT64].typeId;
        newNodeId                             = UA_NODEID_NUMERIC(1, (UA_UInt32)iterator+10000);
        UA_Server_addVariableNode(server, newNodeId, objectId,
                                 UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                 UA_QUALIFIEDNAME(1, "Publisher RepeatedCounter"),
                                 UA_NODEID_NULL, repeatedNodePub, NULL, &pubRepeatedCountNodeID);
    }

    for (UA_Int32 iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
    {
        UA_VariableAttributes repeatedNodeSub = UA_VariableAttributes_default;
        UA_UInt64 repeatedSubscribeValue;
        UA_Variant_setScalar(&repeatedNodeSub.value, &repeatedSubscribeValue, &UA_TYPES[UA_TYPES_UINT64]);
        repeatedNodeSub.accessLevel           = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        repeatedNodeSub.displayName           = UA_LOCALIZEDTEXT("en-US", "Subscriber RepeatedCounter");
        repeatedNodeSub.dataType              = UA_TYPES[UA_TYPES_UINT64].typeId;
        newNodeId                             = UA_NODEID_NUMERIC(1, (UA_UInt32)iterator+50000);
        UA_Server_addVariableNode(server, newNodeId, objectId,
                                 UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                 UA_QUALIFIEDNAME(1, "Subscriber RepeatedCounter"),
                                 UA_NODEID_NULL, repeatedNodeSub, NULL, &subRepeatedCountNodeID);
    }

}

static void
usage(char *progname) {
    printf("usage: %s <ethernet_interface> \n", progname);
    printf("Provide the Interface parameter to run the application. Exiting \n");
 }

/**
 * **Main Server code**
 *
 * The main function contains publisher and subscriber threads running in
 * parallel.
 */
int main(int argc, char **argv) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Int32         returnValue         = 0;
    UA_StatusCode    retval              = UA_STATUSCODE_GOOD;
    UA_Server       *server              = UA_Server_new();
    UA_ServerConfig *config              = UA_Server_getConfig(server);
    pthread_t        userThreadID;
    UA_ServerConfig_setMinimal(config, PORT_NUMBER, NULL);
    UA_NetworkAddressUrlDataType networkAddressUrlPub;

#if defined(SUBSCRIBER)
    UA_NetworkAddressUrlDataType networkAddressUrlSub;
#endif
    if (argc == 1) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }

#if defined(PUBLISHER)
        networkAddressUrlPub.networkInterface = UA_STRING(argv[1]);
#endif
#if defined(SUBSCRIBER)
        networkAddressUrlSub.networkInterface = UA_STRING(argv[1]);
#endif

    }

    else {
#if defined(PUBLISHER)
        networkAddressUrlPub.url = UA_STRING(PUBLISHING_MAC_ADDRESS); /* MAC address of subscribing node*/
#endif
#if defined(SUBSCRIBER)
        networkAddressUrlSub.url = UA_STRING(SUBSCRIBING_MAC_ADDRESS); /* Self MAC address */
#endif
    }

#if defined(UPDATE_MEASUREMENTS)
#if defined(PUBLISHER)
    fpPublisher                   = fopen(filePublishedData, "w");
#endif
#if defined(SUBSCRIBER)
    fpSubscriber                  = fopen(fileSubscribedData, "w");
#endif
#endif

#if defined(PUBLISHER) && defined(SUBSCRIBER)
/* Details about the connection configuration and handling are located in the pubsub connection tutorial */
    config->pubsubTransportLayers = (UA_PubSubTransportLayer *)
                                    UA_malloc(2 * sizeof(UA_PubSubTransportLayer));
#else
    config->pubsubTransportLayers = (UA_PubSubTransportLayer *)
                                    UA_malloc(sizeof(UA_PubSubTransportLayer));
#endif
    if (!config->pubsubTransportLayers) {
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

/* It is possible to use multiple PubSubTransportLayers on runtime.
 * The correct factory is selected on runtime by the standard defined
 * PubSub TransportProfileUri's.
*/

#if defined (PUBLISHER)
    config->pubsubTransportLayers[0] = UA_PubSubTransportLayerEthernetETF();
    config->pubsubTransportLayersSize++;
#endif

    /* Create variable nodes for publisher and subscriber in address space */
    addServerNodes(server);

#if defined(PUBLISHER)
    addPubSubConnection(server, &networkAddressUrlPub);
    addPublishedDataSet(server);
    addDataSetField(server);
    addWriterGroup(server);
    addDataSetWriter(server);
    UA_Server_freezeWriterGroupConfiguration(server, writerGroupIdent);
#endif

#if defined (PUBLISHER) && defined(SUBSCRIBER)
#if defined (UA_ENABLE_PUBSUB_ETH_UADP_XDP)
    config->pubsubTransportLayers[1] = UA_PubSubTransportLayerEthernetXDP();
    config->pubsubTransportLayersSize++;
#else
    config->pubsubTransportLayers[1] = UA_PubSubTransportLayerEthernetETF();
    config->pubsubTransportLayersSize++;
#endif
#endif

#if defined(SUBSCRIBER) && !defined(PUBLISHER)
#if defined (UA_ENABLE_PUBSUB_ETH_UADP_XDP)
    config->pubsubTransportLayers[0] = UA_PubSubTransportLayerEthernetXDP();
    config->pubsubTransportLayersSize++;
#else
    config->pubsubTransportLayers[0] = UA_PubSubTransportLayerEthernetETF();
    config->pubsubTransportLayersSize++;
#endif
#endif

#if defined(SUBSCRIBER)
    addPubSubConnectionSubscriber(server, &networkAddressUrlSub);
    addReaderGroup(server);
    addDataSetReader(server);
    UA_Server_freezeReaderGroupConfiguration(server, readerGroupIdentifier);
    UA_Server_setReaderGroupOperational(server, readerGroupIdentifier);
#endif
    serverConfigStruct *serverConfig;
    serverConfig            = (serverConfigStruct*)UA_malloc(sizeof(serverConfigStruct));
    serverConfig->ServerRun = server;

#if defined(PUBLISHER) || defined(SUBSCRIBER)
    char threadNameUserApplication[22] = "UserApplicationPubSub";
    userThreadID                       = threadCreation(USERAPPLICATION_SCHED_PRIORITY, CORE_THREE, userApplicationPubSub, threadNameUserApplication, serverConfig);
#endif
 /**
     * initialize and create subsriber connection
     * created by lcf
     *
    if((rc = MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,
                               NULL)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
    }

    if((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) !=
       MQTTCLIENT_SUCCESS) {
        printf("Failed to set callbacks, return code %d\n", rc);
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
    }

    if((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe, return code %d\n", rc);
    }
    **
     * initialize and create subsriber connection
     * created by lcf
     */
    retval |= UA_Server_run(server, &running);

    UA_Server_unfreezeReaderGroupConfiguration(server, readerGroupIdentifier);
#if defined(PUBLISHER)
    returnValue = pthread_join(pubthreadID, NULL);
    if (returnValue != 0) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"\nPthread Join Failed for publisher thread:%d\n", returnValue);
    }
#endif
#if defined(SUBSCRIBER)
    returnValue = pthread_join(subthreadID, NULL);
    if (returnValue != 0) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"\nPthread Join Failed for subscriber thread:%d\n", returnValue);
    }
#endif
#if defined(PUBLISHER) || defined(SUBSCRIBER)
    returnValue = pthread_join(userThreadID, NULL);
    if (returnValue != 0) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"\nPthread Join Failed for User thread:%d\n", returnValue);
    }
#endif
/**
     * free mqtt client
     * created by lcf
     *
    if((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to disconnect, return code %d\n", rc);
    }
    MQTTClient_destroy(&client);
    **
     * free mqtt client
     * created by lcf
     */
#if defined(PUBLISHER)
#if defined(UPDATE_MEASUREMENTS)
    /* Write the published data in the publisher_T1.csv file */
   size_t pubLoopVariable               = 0;
   for (pubLoopVariable = 0; pubLoopVariable < measurementsPublisher;
        pubLoopVariable++) {
        fprintf(fpPublisher, "%ld,%ld.%09ld\n",
                publishCounterValue[pubLoopVariable],
                publishTimestamp[pubLoopVariable].tv_sec,
                publishTimestamp[pubLoopVariable].tv_nsec);
    }
#endif
#endif
#if defined(SUBSCRIBER)
#if defined(UPDATE_MEASUREMENTS)
    /* Write the subscribed data in the subscriber_T8.csv file */
    size_t subLoopVariable               = 0;
    for (subLoopVariable = 0; subLoopVariable < measurementsSubscriber;
         subLoopVariable++) {
        fprintf(fpSubscriber, "%ld,%ld.%09ld\n",
                subscribeCounterValue[subLoopVariable],
                subscribeTimestamp[subLoopVariable].tv_sec,
                subscribeTimestamp[subLoopVariable].tv_nsec);
    }
#endif
#endif

#if defined(PUBLISHER) || defined(SUBSCRIBER)
    removeServerNodes(server);
    UA_Server_delete(server);
    UA_free(serverConfig);
#endif
#if defined(PUBLISHER)
    UA_free(pubCounterData);
    for (UA_Int32 iterator = 0; iterator <  REPEATED_NODECOUNTS; iterator++)
        UA_free(repeatedCounterData[iterator]);

#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
    /* Free external data source */
    UA_free(pubDataValueRT);
    for (UA_Int32 iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
        UA_free(repeatedDataValueRT[iterator]);
#endif
#if defined(UPDATE_MEASUREMENTS)
    fclose(fpPublisher);
#endif
#endif

#if defined(SUBSCRIBER)
    UA_free(subCounterData);
    for (UA_Int32 iterator = 0; iterator <  REPEATED_NODECOUNTS; iterator++)
        UA_free(subRepeatedCounterData[iterator]);

#if defined PUBSUB_CONFIG_RT_INFORMATION_MODEL
    /* Free external data source */
    UA_free(subDataValueRT);
    for (UA_Int32 iterator = 0; iterator < REPEATED_NODECOUNTS; iterator++)
        UA_free(subRepeatedDataValueRT[iterator]);
#endif
#if defined(UPDATE_MEASUREMENTS)
    fclose(fpSubscriber);
#endif
#endif
    return (int)retval;
}
