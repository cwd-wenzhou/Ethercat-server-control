#include "ecmk.h"
//ethercat cstruct
//填充相关PDOS信息
ec_pdo_entry_info_t ECMK_pdo_entries[] = {
    {0x7010, 0x01, 1}, /* CH1_ENABLE 1 */
    {0x7010, 0x02, 1}, /* CH2_ENABLE 2 */
    {0x7010, 0x03, 1}, /* CH3_ENABLE 3 */
    {0x7010, 0x04, 1}, /* CH4_ENABLE 4 */
    {0x0000, 0x00, 12}, /* Gap */
    {0x7010, 0x05, 16}, /* CH1_Frequency 5 */
    {0x7010, 0x06, 16}, /* CH2_Frequency 6 */
    {0x7010, 0x07, 16}, /* CH3_Frequency 7 */
    {0x7010, 0x08, 16}, /* CH4_Frequency 8 */
    {0x6000, 0x01, 16}, /* CH 1 */
    {0x6000, 0x02, 16}, /* CH 2 */
    {0x6000, 0x03, 16}, /* CH 3 */
    {0x6000, 0x04, 16}, /* CH 4 */
    {0x6010, 0x01, 1}, /* CH1_ENABLE 1 */
    {0x6010, 0x02, 1}, /* CH2_ENABLE 2 */
    {0x6010, 0x03, 1}, /* CH3_ENABLE 3 */
    {0x6010, 0x04, 1}, /* CH4_ENABLE 4 */
    {0x0000, 0x00, 12}, /* Gap */
    {0x6010, 0x05, 16}, /* CH1_Frequency 5 */
    {0x6010, 0x06, 16}, /* CH2_Frequency 6 */
    {0x6010, 0x07, 16}, /* CH3_Frequency 7 */
    {0x6010, 0x08, 16}, /* CH4_Frequency 8 */
    {0x6020, 0x01, 32}, /* CH1_VOLTAGE 1 */
    {0x6020, 0x02, 32}, /* CH2_VOLTAGE 2 */
    {0x6020, 0x03, 32}, /* CH3_VOLTAGE 3 */
    {0x6020, 0x04, 32}, /* CH4_VOLTAGE 4 */
    {0x6030, 0x01, 1}, /* CH1_state 1 */
    {0x6030, 0x02, 1}, /* CH2_state 2 */
    {0x6030, 0x03, 1}, /* CH3_state 3 */
    {0x6030, 0x04, 1}, /* CH4_state 4 */
    {0x6030, 0x05, 1}, /* software_state 5 */
    {0x6030, 0x06, 1}, /* hardware_state 6 */
    {0x6030, 0x07, 1}, /* EtherCAT_state 7 */
    {0x6030, 0x08, 1}, /* ALL_state 8 */
    {0x0000, 0x00, 8}, /* Gap */
    {0x6050, 0x01, 48}, /* ECMKA/B 1 */
    {0x6050, 0x02, 32}, /* HARDWARE-VERSION 2 */
    {0x6050, 0x03, 32}, /* SOFTWARE-VERSION 3 */
    {0x6050, 0x04, 16}, /* UDI 4 */
    {0x6050, 0x05, 32}, /* manufactured ID 5 */
    {0x6050, 0x06, 32}, /* PRODUCT ID 6 */
    {0x6050, 0x07, 32}, /* PRODUCT ID 7 */
    {0x6050, 0x08, 32}, /* PRODUCT ID 8 */
};
ec_pdo_info_t ECMK_pdos[] = {
    {0x1601, 9, ECMK_pdo_entries + 0}, /* DO RxPDO-Map */
    {0x1a00, 4, ECMK_pdo_entries + 9}, /* DI TxPDO-Map */
    {0x1a01, 9, ECMK_pdo_entries + 13}, /* AO RxPDO-Map */
    {0x1a02, 4, ECMK_pdo_entries + 22}, /* AI TxPDO-Map */
    {0x1a03, 9, ECMK_pdo_entries + 26}, /* AI TxPDO-Map */
    {0x1a05, 8, ECMK_pdo_entries + 35}, /* PRODUCT INFO TxPDO-Map */
};
ec_sync_info_t ECMK_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, ECMK_pdos + 0, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 5, ECMK_pdos + 1, EC_WD_DISABLE},
    {0xFF}
    };

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
    //printf("CH1_Current=%f   CH2_Current=%f   CH3_Current=%f   CH4_Current=%f"
    //,CHx_Current[0],CHx_Current[1],CHx_Current[2],CHx_Current[3]);
}

void ECMK::send_data(){
    //empty function;
}
