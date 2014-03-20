#include "TaskRunner.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <mutex>
#include <condition_variable>
#include <TaskRunnerContainer.h>

TaskRunner::TaskRunner(int runnerId)
:_id(runnerId),_stop_conn_time(0),isOnline(false)
{
    char filename[20];
    sprintf(filename, "log_%d.txt", runnerId);
#if __FILELOG__
    fout.open(filename);
#endif
}
TaskRunner::~TaskRunner(){
    for (auto& req: _req_list) {
        json_decref(req.content);
    }
#if __FILELOG__
    fout.close();
#endif
}

void _pc_request_cb(pc_request_t *req, int status, json_t *resp);

void TaskRunner::addRequestTask(const char* router,json_t* content, int times)
{
    ReqBody body;
    body.route = router;
    body.content = content;
    body.repeatTimes = times;
	_req_list.push_back(body);
}

void TaskRunner::run(const char* addr, int port)
{
#if __FILELOG__
    fout<<"TaskRunner #"<<_id<<" start running..."<<endl;
#endif
    _total_count = 0;
#if __FILELOG__
    fout<<"TaskRunner #"<<_id<<" connect success."<<endl;
#endif
    vector<double> result;
    auto iter = _req_list.begin();
    if (iter == _req_list.end()) {
        return;
    }
    for (;iter!=_req_list.end();++iter) {
        for (int i = 0; i < iter->repeatTimes; ++i) {
            result.push_back(durationOfFunction(bind(&TaskRunner::_request,this,iter->route,iter->content)));
            ++_total_count;
//        ++TaskRunnerContainer::_totalReqs;
        }
    }
    
#if __FILELOG__
    fout<<"TaskRunner #"<<_id<<" end running..."<<endl;
#endif
    
    auto minmax = minmax_element(result.begin(),result.end());
    _min_req_time = *minmax.first;
    _max_req_time = *minmax.second;
    _duration = accumulate(result.begin(),result.end(),0);
    _avg_req_time = _duration/(_total_count);
    _query_per_sec = (_total_count)*1000/_duration;
}

void TaskRunner::_request(string& router,json_t* content)
{
    pc_request_t *request = pc_request_new();
    request->data = this;
    int ret = pc_request(_client, request, router.c_str(), content, _pc_request_cb);
    if (ret) {
        isOnline = false;
    }
    unique_lock<mutex> lk(m);
    cv.wait(lk);
    lk.unlock();
#if __FILELOG__
    fout<<"TaskRunner #"<<_id<<" send request success after"<<endl;
#endif
}

void _pc_request_cb(pc_request_t *req, int status, json_t *resp)
{
    TaskRunner* tr = (TaskRunner*)req->data;
    tr->cv.notify_one();
#if __FILELOG__
    tr->fout<<"TaskRunner #"<<tr->get_id()<<" send request success"<<endl;
#endif
    pc_request_destroy(req);
}

void TaskRunner::connect(const char* addr, int port)
{
    _connection_time = durationOfFunction(bind(&TaskRunner::_connect,this,addr,port));
}
void TaskRunner::_connect(const char* addr, int port)
{
    _client = pc_client_new();
    struct sockaddr_in address;
    
    memset(&address, 0, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(addr);
    
    // try to connect to server.
    int ret = pc_client_connect(_client, &address);
    if(ret) {
#if __FILELOG__
        fout<<"TaskRunner #"<<_id<<" fail to connect server "<<addr<<" on port"<<port<<endl;
#endif
        pc_client_destroy(_client);
        exit(100);
    }
    isOnline = true;
}

void TaskRunner::stop()
{
    /*close the connection*/
    _stop_conn_time = durationOfFunction(bind(&TaskRunner::_stop,this));
}
void TaskRunner::_stop()
{
    if(_client){
        pc_client_destroy(_client);
    }
#if __FILELOG__
    fout<<"TaskRunner #"<<_id<<" stop."<<endl;
#endif
    
}

double TaskRunner::durationOfFunction(function<void()>&& func)
{
    /*copy from http://en.cppreference.com/w/cpp/chrono*/
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    func();
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    return elapsed_seconds.count()*1000;
}

int TaskRunner::get_id()
{
    return _id;
}

double TaskRunner::getAvgReqTime()
{
    return _avg_req_time;
}

void TaskRunner::printHeader(ostream& output)
{
    output.setf(ios::left);
    output<<"#"<<"id\t"<<"conn\t"<<"stop\t"<<"max_req\t"<<"min_req\t"<<"avg_req\t"<<
    "duration\t"<<"total_count\t"<<"query_per_sec\t"<<endl;
}

void TaskRunner::printStatistics(ostream& output)
{
    output.setf(ios::fixed|ios::left);
    output<<"#"<<_id<<"\t"<<std::setprecision(3)<<_connection_time<<"\t"<<_stop_conn_time<<"\t"<<_max_req_time<<"\t"<<_min_req_time<<"\t"<<_avg_req_time<<"\t"<<
    _duration<<"\t"<<_total_count<<"\t"<<_query_per_sec<<endl;
}