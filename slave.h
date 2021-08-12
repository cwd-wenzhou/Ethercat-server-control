/*
 * @Author       : cwd
 * @Date         : 2021-8-4
 * @Place  : hust
 * 该头文件建立符合igh的从站slave类，加入用户定制的类一律需要继承该类。
 * 
 */
#include <iostream>
#ifndef SLAVE_DEF
#define SLAVE_DEF
class SLAVE{
public:
    //每一个motor一个域
    ec_domain_t *domain;             //域
    ec_domain_state_t domain_state;  //域状态

    ec_slave_config_t *slave;  //从站配置，这里只有一台迈信伺服
    ec_slave_config_state_t slave_state;  //从站配置状态

    uint8_t *domain_pd;

    virtual void read_data();
    virtual void send_data();
    virtual void print();
    virtual ec_pdo_entry_reg_t *Domain_regs(uint16_t position);
    virtual ec_sync_info_t* get_ec_sync_info_t_();

    virtual ~SLAVE(){};
};

#endif //SLAVE_DEF