/*
 * @Author       : cwd
 * @Date         : 2021-5-15
 * @Place  : hust
 * sheding 
 * */

#ifndef SIG_H
#define SIG_H

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include "log/log.h"


void addsig(int sig,void (*handler)(int) )
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL)!=-1);//捕获到信号sig，使用sa中规定的方法处理
}

#endif