###在libpomelo基础上写的一个简单的压力测试工具

* xcode console 工程
* 依赖库：libpomelo
* 测试服务器：fruit slot pomelo server [github](https://github.com/sric0880/fruit_slot)

传入两个个参数：

1. arg1，child_process_nums，子进程数量，见注意事项2。
2. arg2，clients_nums，每进程发起客户端个数。
	
	可以配置客户端请求事件的顺序和数量。客户一旦连接上服务器，就一直保持长连接，不释放，并且循环重复发送请求事件。
	
	<strong>注意：
	* 并不是同时clients_nums并发，理论并发数=clients_nums * (child_process_nums+1)，实际并发数要小于理论并发数。
	* 客户端由于使用libuv库，每进程中子线程数以3倍的在线客户数量增长。参见注意事项2。
	* 支持的总在线用户数为理论并发数. 总在线客户数受max open files限制，参见注意1.
	</strong>

####注意
1. 需要修改MacOS X 10.8.5 max open files（限制本地socket连接上限）：

	```
		1. 修改或添加/etc/launchd.conf: `limit maxfiles 60000 100000`
		2. 重启
		3. `launchctl limit`查看是否生效
	```
2. 每一个进程最大线程数有限制。在MacOS 10.8.5上限制每进程2048子线程。客户端如果在线，那么每个客户端将占用3个线程，当每进程在线客户达到700左右，客户端程序会报错。所以推荐client_nums最大为650（以个人机子为依据）。




