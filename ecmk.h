/*
 * @Author       : cwd
 * @Date         : 2021-8-2
 * @Place  : hust
 * 该头文件建立由自己定义的ethercat从站ECMK-A/B
 * 
 */
#ifndef ECMKA
#define ECMKA

#include <ecrt.h>
#include "slave.h"
#include <stdio.h>
#define ECMKCODE 0x00000009, 0x00009252  // EP3E的厂家标识和产品标识
//PDO入口的偏移量
struct PDO_OFFSET {
    unsigned int CHx_Displacement[4];   //4通道位移值
    unsigned int CHx_Current[4];        //4通道电流值
    unsigned int product_name;
    unsigned int Hardware_Version;
    unsigned int Software_Version;
};

class ECMK:public SLAVE{
public:
    struct PDO_OFFSET pdo_offset;  //从站驱动器变量

    unsigned int CHx_Displacement[4];   //4通道位移值
    double CHx_Current[4];        //4通道电流值
    char product_name[20];
    char Hardware_Version[20];
    char Software_Version[20];

    void read_data();
    void send_data();
    ec_pdo_entry_reg_t *Domain_regs(uint16_t position);
    ec_sync_info_t* get_ec_sync_info_t_();
    ~ECMK(){};
};

extern ec_pdo_entry_info_t ECMK_pdo_entries[43];
extern ec_pdo_info_t ECMK_pdos[6] ;
extern ec_sync_info_t ECMK_syncs[5];



    
#endif //ECMKA