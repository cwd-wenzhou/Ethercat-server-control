#include "ecmk.h"
//ethercat cstruct
//填充相关PDOS信息
ec_pdo_entry_info_t ECMK_pdo_entries[] = {
    {0x7010, 0x01, 8}, /* CH1_ENABLE 1 */
    {0x7010, 0x02, 8}, /* CH2_ENABLE 2 */
    {0x7010, 0x03, 8}, /* CH3_ENABLE 3 */
    {0x7010, 0x04, 8}, /* CH4_ENABLE 4 */
    {0x7010, 0x05, 16}, /* CH1_Frequency 5 */
    {0x7010, 0x06, 16}, /* CH2_Frequency 6 */
    {0x7010, 0x07, 16}, /* CH3_Frequency 7 */
    {0x7010, 0x08, 16}, /* CH4_Frequency 8 */
    {0x6020, 0x01, 32}, /* CH1_VOLTAGE 1 */
    {0x6020, 0x02, 32}, /* CH2_VOLTAGE 2 */
    {0x6020, 0x03, 32}, /* CH3_VOLTAGE 3 */
    {0x6020, 0x04, 32}, /* CH4_VOLTAGE 4 */

};
ec_pdo_info_t ECMK_pdos[] = {
    {0x1601, 8, ECMK_pdo_entries + 0}, /* DO RxPDO-Map */
    {0x1a02, 4, ECMK_pdo_entries + 8}, /* AI TxPDO-Map */
};
ec_sync_info_t ECMK_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, ECMK_pdos + 0, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, ECMK_pdos + 1, EC_WD_DISABLE},
    {0xff}

    };

ec_sync_info_t* ECMK::get_ec_sync_info_t_(){
    return ECMK_syncs;
}

ec_pdo_entry_reg_t *ECMK::Domain_regs(uint16_t position){
    ec_pdo_entry_reg_t*  ans= new ec_pdo_entry_reg_t[13]{
        //alias,position,vender ID,Product_code,index,subindex,offset,bit_position
        {0,position, ECMKCODE,  0x7010,1, &this->pdo_offset.CHx_Enable_t[0]},
        {0,position, ECMKCODE,  0x7010,2, &this->pdo_offset.CHx_Enable_t[1]},
        {0,position, ECMKCODE,  0x7010,3, &this->pdo_offset.CHx_Enable_t[2]},  
        {0,position, ECMKCODE,  0x7010,4, &this->pdo_offset.CHx_Enable_t[3]},
        {0,position, ECMKCODE,  0x7010,5, &this->pdo_offset.CHx_Frequency_t[0]},
        {0,position, ECMKCODE,  0x7010,6, &this->pdo_offset.CHx_Frequency_t[1]},
        {0,position, ECMKCODE,  0x7010,7, &this->pdo_offset.CHx_Frequency_t[2]},
        {0,position, ECMKCODE,  0x7010,8, &this->pdo_offset.CHx_Frequency_t[3]},
/*
        {0,position, ECMKCODE,  0x6000,1, &this->pdo_offset.CHx_Displacement[0]},
        {0,position, ECMKCODE,  0x6000,2, &this->pdo_offset.CHx_Displacement[1]},
        {0,position, ECMKCODE,  0x6000,3, &this->pdo_offset.CHx_Displacement[2]},
        {0,position, ECMKCODE,  0x6000,4, &this->pdo_offset.CHx_Displacement[3]},

        {0,position, ECMKCODE,  0x6010,1, &this->pdo_offset.CHx_Enable[0]},
        {0,position, ECMKCODE,  0x6010,2, &this->pdo_offset.CHx_Enable[1]},
        {0,position, ECMKCODE,  0x6010,3, &this->pdo_offset.CHx_Enable[2]},
        {0,position, ECMKCODE,  0x6010,4, &this->pdo_offset.CHx_Enable[3]},
        {0,position, ECMKCODE,  0x6010,5, &this->pdo_offset.CHx_Frequency[0]},
        {0,position, ECMKCODE,  0x6010,6, &this->pdo_offset.CHx_Frequency[1]},
        {0,position, ECMKCODE,  0x6010,7, &this->pdo_offset.CHx_Frequency[2]},
        {0,position, ECMKCODE,  0x6010,8, &this->pdo_offset.CHx_Frequency[3]},
*/
        {0,position, ECMKCODE,  0x6020,1, &this->pdo_offset.CHx_Current[0]},
        {0,position, ECMKCODE,  0x6020,2, &this->pdo_offset.CHx_Current[1]},
        {0,position, ECMKCODE,  0x6020,3, &this->pdo_offset.CHx_Current[2]},
        {0,position, ECMKCODE,  0x6020,4, &this->pdo_offset.CHx_Current[3]},
/*
        {0,position, ECMKCODE,  0x6030,1, &this->pdo_offset.CHx_STATE[0]},
        {0,position, ECMKCODE,  0x6030,2, &this->pdo_offset.CHx_STATE[1]},
        {0,position, ECMKCODE,  0x6030,3, &this->pdo_offset.CHx_STATE[2]},
        {0,position, ECMKCODE,  0x6030,4, &this->pdo_offset.CHx_STATE[3]},
        {0,position, ECMKCODE,  0x6030,5, &this->pdo_offset.software_state},
        {0,position, ECMKCODE,  0x6030,6, &this->pdo_offset.hardware_state},
        {0,position, ECMKCODE,  0x6030,7, &this->pdo_offset.ethercat_state},
        {0,position, ECMKCODE,  0x6030,8, &this->pdo_offset.all_status},

        {0,position, ECMKCODE,  0x6050,1, &this->pdo_offset.product_name},
        {0,position, ECMKCODE,  0x6050,2, &this->pdo_offset.Hardware_Version},
        {0,position, ECMKCODE,  0x6050,3, &this->pdo_offset.Software_Version},
        {0,position, ECMKCODE,  0x6050,4, &this->pdo_offset.uid},
        {0,position, ECMKCODE,  0x6050,5, &this->pdo_offset.uid},
        {0,position, ECMKCODE,  0x6050,6, &this->pdo_offset.product_id},
        {0,position, ECMKCODE,  0x6050,7, &this->pdo_offset.product_id},
        {0,position, ECMKCODE,  0x6050,8, &this->pdo_offset.num_id},
*/
        {}
    };
    return ans;
}

void ECMK::read_data(){
    /*
    this->CHx_Current[0] = EC_READ_S32(this->domain_pd+this->pdo_offset.CHx_Current[0]);
    this->CHx_Current[1] = EC_READ_S32(this->domain_pd+this->pdo_offset.CHx_Current[1]);
    this->CHx_Current[2] = EC_READ_S32(this->domain_pd+this->pdo_offset.CHx_Current[2]);
    this->CHx_Current[3] = EC_READ_S32(this->domain_pd+this->pdo_offset.CHx_Current[3]);
    this->CHx_Displacement[0] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[0]);
    this->CHx_Displacement[1] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[1]);
    this->CHx_Displacement[2] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[2]);
    this->CHx_Displacement[3] = EC_READ_S16(this->domain_pd+this->pdo_offset.CHx_Displacement[3]);
    printf("CH1_Current=%f   CH2_Current=%f   CH3_Current=%f   CH4_Current=%f\n"
    ,CHx_Current[0],CHx_Current[1],CHx_Current[2],CHx_Current[3]);
    */
}

void ECMK::send_data(){
    //empty function;
}
