#ifndef INIT_H
#define INIT_H

#include "header.h"

//初始化EtherCAT主站函数
static void
init_EtherCAT_master(struct MOTOR *motor,struct MOTOR_DATA *motor_data) {
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

#endif