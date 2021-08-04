/*
 * @Author       : cwd
 * @Date         : 2021-7-26
 * @Place  : hust
 * 主站初始化需要的配置函数
 */
#ifndef INIT_H
#define INIT_H

#include "motor.h"
#include "ecmk.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <ecrt.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <string.h>
//从站配置所用的参数
#define EP3ESLAVEPOS 0              //迈信伺服EP3E在ethercat总线上的位置
#define MAXSINE 0x000007DD, 0x00000001  // EP3E的厂家标识和产品标识

// CoE对象字典
#define RXPDO 0x1600
#define TXPDO 0x1A00

//初始化EtherCT主站函数
ec_master_t * init_EtherCAT_slaves(ec_master_t *master,std::vector<SLAVE*> &slaves) ;
//主站检查函数
void check_master_state(ec_master_t *master, ec_master_state_t *master_state);
//域检查函数
void check_domain_state(ec_domain_t *domain, ec_domain_state_t *domain_state);
//从站配置检查函数
void check_slave_config_states(ec_slave_config_t *slave,ec_slave_config_state_t *slave_state);
//带输出的ecrt_request_master
ec_master_t *Ecrt_request_master( unsigned int master_index);
ec_domain_t *Ecrt_master_create_domain(ec_master_t *master );
#endif