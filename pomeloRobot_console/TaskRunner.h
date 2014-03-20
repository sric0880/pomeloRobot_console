#include <string>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <vector>
#include "pomelo.h"
using namespace std;

#define __FILELOG__ 0

/*
* 
*/
typedef struct requset_body {
    string route;
    json_t* content;
    int repeatTimes;
}ReqBody;

class TaskRunner
{
public:
	TaskRunner(int runnerId);
	virtual ~TaskRunner();

	void addRequestTask(const char* router,json_t* content, int);

	/**
 *	@brief	start a new worker thread
 *
 *	@Modified by qiong at 2014-02-24 14:28:56
 *
 *	@param 	addr 	ip
 *	@param 	port 	port
**/
    void run(const char* addr, int port);
    void connect(const char* addr, int port);
    void stop();
    
    condition_variable cv;
    mutex m;
    ofstream fout;
    int get_id();
    bool isOnline;
    
    void printStatistics(ostream& output);
    static void printHeader(ostream& output);
    double getAvgReqTime();

	/* data */
private:
	int _id;
	pc_client_t * _client;
	vector<ReqBody> _req_list;

	/*statistic unit: s*/
	double _connection_time;
	double _stop_conn_time;
	double _max_req_time;
	double _min_req_time;
	double _avg_req_time;
	int _query_per_sec;
	double _duration;
	int _total_count;
    
    
    void _request(string& router,json_t* content);
    void _connect(const char* addr, int port);
    void _stop();
    
    double durationOfFunction(function<void()>&& func);
};