#include "ecmk.h"

//填充相关PDOS信息
ec_pdo_entry_info_t ECMK_pdo_entries[] = {
                                        /*TxPdo 0x1A00--CHx_Displacement[4]*/
                                        {0x6000, 0x01, 16},
                                        {0x6000, 0x02, 16},
                                        {0x6000, 0x03, 16},
                                        {0x6000, 0x04, 16},
                                        /*TxPdo 0x1A02--CHx_Current[4]*/
                                        {0x6020, 0x01, 32},
                                        {0x6020, 0x02, 32},
                                        {0x6020, 0x03, 32},
                                        {0x6020, 0x04, 32},
                                        /*TxPdo 0x1A05--Product_Info_Query
                                        {0x6050, 0x01, 48},
                                        {0x6050, 0x02, 32},
                                        {0x6050, 0x03, 32},*/
                                        };
ec_pdo_info_t ECMK_pdos[] = {
                            {0x1a00, 4, ECMK_pdo_entries + 0},
                            {0x1a02, 4, ECMK_pdo_entries + 4}
                            //{0x1A05, 3, ECMK_pdo_entries + 8}
                            };
ec_sync_info_t ECMK_syncs[] = {{0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
                            {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
                            {2, EC_DIR_INPUT, 1, ECMK_pdos + 0, EC_WD_DISABLE},
                            {3, EC_DIR_INPUT, 1, ECMK_pdos + 1, EC_WD_DISABLE},
                            //{4, EC_DIR_INPUT, 1, ECMK_pdos + 3, EC_WD_DISABLE},
                            {0xFF}};

ec_sync_info_t* ECMK::get_ec_sync_info_t_(){
    return ECMK_syncs;
}

ec_pdo_entry_reg_t *ECMK::Domain_regs(uint16_t position){
    ec_pdo_entry_reg_t*  ans= new ec_pdo_entry_reg_t[9]{
        //alias,position,vender ID,Product_code,index,subindex,offset,bit_position
        {0,position, ECMKCODE,  0x6000,1, &this->pdo_offset.CHx_Displacement[0]},
        {0,position, ECMKCODE,  0x6000,2, &this->pdo_offset.CHx_Displacement[1]},
        {0,position, ECMKCODE,  0x6000,3, &this->pdo_offset.CHx_Displacement[2]},
        {0,position, ECMKCODE,  0x6000,4, &this->pdo_offset.CHx_Displacement[3]},
        {0,position, ECMKCODE,  0x6020,1, &this->pdo_offset.CHx_Current[0]},
        {0,position, ECMKCODE,  0x6020,2, &this->pdo_offset.CHx_Current[1]},
        {0,position, ECMKCODE,  0x6020,3, &this->pdo_offset.CHx_Current[2]},
        {0,position, ECMKCODE,  0x6020,4, &this->pdo_offset.CHx_Current[3]},
        {}
    };
    return ans;
}

void ECMK::read_data(){
    this->CHx_Current[0] = EC_READ_S64(this->domain_pd+this->pdo_offset.CHx_Current[0]);
    this->CHx_Current[1] = EC_READ_S64(this->domain_pd+this->pdo_offset.CHx_Current[1]);
    this->CHx_Current[2] = EC_READ_S64(this->domain_pd+this->pdo_offset.CHx_Current[2]);
    this->CHx_Current[3] = EC_READ_S64(this->domain_pd+this->pdo_offset.CHx_Current[3]);
    this->CHx_Displacement[0] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[0]);
    this->CHx_Displacement[1] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[1]);
    this->CHx_Displacement[2] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[2]);
    this->CHx_Displacement[3] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[3]);
}

void ECMK::send_data(){
    //empty function;
}
