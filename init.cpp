#include "init.h"
ec_master_t * init_EtherCAT_slaves(ec_master_t * master,std::vector<SLAVE*> &slaves) {

    int SLAVE_NUM = slaves.size();
    for (uint16_t i=0;i< SLAVE_NUM;i++){
        ec_slave_info_t *slave_info = new ec_slave_info_t;
        if (ecrt_master_get_slave(master, i,slave_info))
            printf("get slave_info error\n");
        else printf("name of %d is %s\n",i,slave_info->name);
        
        if (strcmp(slave_info->name,"EP3E-EC")==0){
            slaves[i] = new MOTOR(0x000007DD,0x00000001);
            slaves[i]->slave = ecrt_master_slave_config(master, 0,i, 0x000007DD,0x00000001);
        }
        else if (strcmp(slave_info->name,"L7N")==0){
            slaves[i] = new MOTOR(0x00007595,0x00000000);
            slaves[i]->slave = ecrt_master_slave_config(master, 0,i, 0x00007595,0x00000000);
        }
        else if (strcmp(slave_info->name,"ECMK-A")==0)
        {
            slaves[i] = new ECMK;
            slaves[i]->slave = ecrt_master_slave_config(master, 0,i, ECMKCODE);
        }
            

        //变量与对应PDO数据对象关联
        ec_pdo_entry_reg_t* domain_regs=slaves[i]->Domain_regs(i);

        //创建域domain
        slaves[i]->domain = Ecrt_master_create_domain(master);

         //检查从站配置
        if (slaves[i]->slave==NULL){
            printf("Failed to get slave configuration for EP3E!\n");
            exit(EXIT_FAILURE);  //配置失败，退出线程
        }
            
    
        //配置PDOs
        printf("Configuring PDOs...\n");
        if(ecrt_slave_config_pdos(slaves[i]->slave, EC_END, slaves[i]->get_ec_sync_info_t_())) {
            printf("Failed to configure EP3E PDOs!\n");
            exit(EXIT_FAILURE);  //配置失败，退出线程
        } else {
            printf("*Success to configuring EP3E PDOs*\n");  //配置成功
        }

        //注册PDO entry
        if(ecrt_domain_reg_pdo_entry_list(slaves[i]->domain, domain_regs)) {
            printf("PDO entry registration failed!\n");
            exit(EXIT_FAILURE);  //注册失败，退出线程
        } else {
            printf("*Success to configuring EP3E PDO entry*\n");  //注册成功
        }  
    }
    //激活主站master
    printf("Activating master...\n");
    if(ecrt_master_activate(master)) {
        exit(EXIT_FAILURE);  //激活失败，退出线程
    } else {
        printf("*Master activated*\n");  //激活成功
    }

    for (uint16_t i=0;i< SLAVE_NUM;i++){
        if(!(slaves[i]->domain_pd = ecrt_domain_data(slaves[i]->domain))) {
        exit(EXIT_FAILURE);
        }
    }
    return master;
}


ec_master_t *Ecrt_request_master(unsigned int master_index  ){
    ec_master_t * master=ecrt_request_master(0);
    if(!master) {
        printf("Failed to create ethercat master!\n");
        exit(EXIT_FAILURE);  //创建失败，退出线程
    }
    else
        printf("ethercat master create seccessed\n");
    return master;
}

ec_domain_t *Ecrt_master_create_domain(ec_master_t *master ){
    ec_domain_t * domain = ecrt_master_create_domain(master);
        if(!domain) {
            printf("Failed to create master domain!\n");
            exit(EXIT_FAILURE);  //创建失败，退出线程
        }
        return domain;
}

//主站检查函数
void  check_master_state(ec_master_t *master, ec_master_state_t *master_state) {
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

//域检查函数
void check_domain_state(ec_domain_t *domain, ec_domain_state_t *domain_state) {
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

//从站配置检查函数
void check_slave_config_states(ec_slave_config_t *slave,
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
