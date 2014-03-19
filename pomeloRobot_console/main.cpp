//
//  main.cpp
//  pomeloRobot_console
//
//  Created by qiong on 14-2-24.
//  Copyright (c) 2014年 qiong. All rights reserved.
//

#include <iostream>
#include <cstdlib>
#include <vector>
#include <functional>
#include <random>
#include <unistd.h>
#include "TaskRunner.h"
#include "TaskRunnerContainer.h"
using namespace std;

//const char* HOST = "127.0.0.1";
const char* HOST = "192.168.1.117";
//const char* HOST = "115.28.33.15";
const int PORT = 3010;
const int bet_cols_size = 8;
random_device rd;
default_random_engine ran(rd());
uniform_int_distribution<int> uni(0,149);
uniform_int_distribution<int> uni1(1,8);
uniform_int_distribution<int> distribution1(0,bet_cols_size-1);
std::mt19937 mt;//the finest random engine
std::minstd_rand0 g1;//the fastest. minstd_rand0 is a standard linear_congruential_engine

void random_bet(int bet_cols, int max_limit, int* res)
{
    uniform_int_distribution<int> distribution(1,max_limit);
    int i = 0;
    int arr_index[bet_cols_size] ={0};
    if (bet_cols == bet_cols_size) {
        while (i != bet_cols_size) {
            arr_index[i] = 1;
            ++i;
        }
    }else{
        while(i != bet_cols){
            int index = distribution1(g1);
            if (arr_index[index] == 0) {
                arr_index[index] = 1;
                ++i;
            }
        }
    }
    for (int i = 0; i < bet_cols_size; ++i) {
        if (arr_index[i]==1) {
            res[i] = distribution(g1);
        }
    }
}

void spawn(int n)
{
    if(n)
    {
        if(fork())//parent process
        {
            signal(SIGCHLD, SIG_IGN);
            if(n)
            {
                spawn(n-1);
            }
            else
                return;
        }
    }
}

int main(int argc, const char * argv[])
{
    if (argc!=4) {
        cout<<"Usage: ./pomelo_robot [child_process_nums] [thread_nums] [clients_nums]"<<endl;
        return 0;
    }
    int proc_nums = atoi(argv[1]);
    int clients_nums = atoi(argv[3]);
    int thread_nums = atoi(argv[2]);
    
    if (clients_nums == 0) {
        clients_nums = 1;
    }
    if (thread_nums == 0) {
        thread_nums = 1;
    }
    
    //create more than one process
    spawn(proc_nums);
    
    TaskRunnerContainer trc(HOST,PORT,clients_nums);
    trc.setGenerateFunc(bind([](int id)->TaskRunner*{
        TaskRunner* tr = new TaskRunner(id);
        char username[20];
        sprintf(username, "%d-%d",getpid(), id);
        
        /*login request*/
        json_t *msg = json_object();
        json_object_set_new(msg, "uid", json_string(username));
        tr->addRequestTask("connector.entryHandler.enter",msg);
        
        /*send msg request*/
        for (int k = 0; k < 10; ++k) {
            json_t *msg1 = json_object();
            int res[bet_cols_size] = {0};
            random_bet(uni1(ran), 10, res);
            json_t* arr = json_array();
            for (int i = 0; i < bet_cols_size; ++i) {
                json_array_append(arr, json_integer(res[i]));
            }
            json_object_set_new(msg1, "v", arr);
            json_object_set_new(msg1, "gid", json_integer(uni(ran)));
            tr->addRequestTask("connector.entryHandler.random",msg1);
        }
        return tr;
    }, placeholders::_1));
    trc.startRun(thread_nums);
    return 0;
}

