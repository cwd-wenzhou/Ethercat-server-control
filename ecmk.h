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

#define ECMKCODE 0x00000001, 0x00000001  // EP3E的厂家标识和产品标识
//PDO入口的偏移量
struct PDO_OFFSET {
    unsigned int CHx_Displacement[4];   //4通道位移值
    unsigned int CHx_Current[4];        //4通道电流值
    unsigned int product_name;
    unsigned int Hardware_Version;
    unsigned int Software_Version;
};

struct ECMK{
    ec_domain_t *domain;             //域
    ec_domain_state_t domain_state;  //域状态

    ec_slave_config_t *ECMK_Slave_Config;  //从站配置
    ec_slave_config_state_t ECMK_Slave_Config_State;  //从站配置状态

    uint8_t *domain_pd;                     // Process Data
    struct PDO_OFFSET pdo_offset;  //从站驱动器变量

    unsigned int CHx_Displacement[4];   //4通道位移值
    double CHx_Current[4];        //4通道电流值
    char product_name[20];
    char Hardware_Version[20];
    char Software_Version[20];

};

//填充相关PDOS信息
ec_pdo_entry_info_t ECMK_pdo_entries[] = {
                                        /*TxPdo 0x1A00--CHx_Displacement[4]*/
                                        {0x6000, 0x01, 16},
                                        {0x6000, 0x02, 16},
                                        {0x6000, 0x03, 16},
                                        {0x6000, 0x04, 16},
                                        /*TxPdo 0x1A01--CHx_Current[4]*/
                                        {0x6010, 0x05, 32},
                                        {0x6010, 0x06, 32},
                                        {0x6010, 0x07, 32},
                                        {0x6010, 0x08, 32},
                                        /*TxPdo 0x1A05--Product_Info_Query*/
                                        {0x6050, 0x01, 48},
                                        {0x6050, 0x02, 32},
                                        {0x6050, 0x03, 32},
                                        };
ec_pdo_info_t ECMK_pdos[] = {
                            {0x1A00, 4, ECMK_pdo_entries + 0},
                            {0x1A01, 4, ECMK_pdo_entries + 4},
                            {0x1A05, 3, ECMK_pdo_entries + 8}};
ec_sync_info_t EP3E_syncs[] = {{0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
                            {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
                            {2, EC_DIR_INPUT, 1, ECMK_pdos + 0, EC_WD_DISABLE},
                            {3, EC_DIR_INPUT, 1, ECMK_pdos + 1, EC_WD_DISABLE},
                            {4, EC_DIR_INPUT, 1, ECMK_pdos + 3, EC_WD_DISABLE},
                            {0xFF}};

ec_pdo_entry_reg_t *domain_ECMK_regs(uint16_t i,struct ECMK* ecmk){

    ec_pdo_entry_reg_t*  ans= new ec_pdo_entry_reg_t[9]
    {
            //alias,position,vender ID,Product_code,index,subindex,offset,bit_position
            {0,i, ECMKCODE,  0x6000,1, &(ecmk+i)->pdo_offset.CHx_Displacement[0]},
            {0,i, ECMKCODE,  0x6000,2, &(ecmk+i)->pdo_offset.CHx_Displacement[1]},
            {0,i, ECMKCODE,  0x6000,3, &(ecmk+i)->pdo_offset.CHx_Displacement[2]},
            {0,i, ECMKCODE,  0x6000,4, &(ecmk+i)->pdo_offset.CHx_Displacement[3]},
            {0,i, ECMKCODE,  0x6020,1, &(ecmk+i)->pdo_offset.CHx_Current[0]},
            {0,i, ECMKCODE,  0x6020,2, &(ecmk+i)->pdo_offset.CHx_Current[1]},
            {0,i, ECMKCODE,  0x6020,3, &(ecmk+i)->pdo_offset.CHx_Current[2]},
            {0,i, ECMKCODE,  0x6020,4, &(ecmk+i)->pdo_offset.CHx_Current[3]},
            {}};
        return ans;
}

    
#endif //ECMKA