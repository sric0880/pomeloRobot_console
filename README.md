###在libpomelo基础上写的一个简单的压力测试工具

* xcode console 工程
* 依赖库：libpomelo
* 测试服务器：chatofpomelo-websocket [github](https://github.com/NetEase/chatofpomelo-websocket)

传入三个参数：

1. arg1，child_process_nums，子线程数量，见注意事项2。
2. arg2，threads_num，每进程中并发线程数量，实现并发访问服务器。
3. arg3，clients_nums，每进程发起客户端个数。
	
	可以配置客户端请求事件的顺序和数量，完成所有请求，可以控制客户端是否释放连接。完成所有请求之后，当前线程继续发送其他客户端的请求。
	
	<strong>注意：
	* 并不是同时clients_nums并发，实际并发数=threads_num * (child_process_nums+1)。
	* 客户端由于使用libuv库，请求结束后不释放，每进程中子线程数以2倍的在线客户数量增长。参见注意事项2。
	* 支持的总在线用户数为 child_process_nums * clients_nums. 总在线客户数受max open files限制，参见注意1.
	* 当一个子进程结束，该线程下的所有客户将会被释放。
	</strong>

####注意
1. 需要修改MacOS X 10.8.5 max open files（限制本地socket连接上限）：

	```
		1. 修改或添加/etc/launchd.conf: `limit maxfiles 60000 100000`
		2. 重启
		3. `launchctl limit`查看是否生效
	```
2. 每一个进程最大线程数有限制。在MacOS 10.8.5上限制每进程2048子线程。如果客户端请求结束不及时释放，那么每个客户端将占用2个线程，当每进程在线客户达到1000左右，客户端程序会报错。所以推荐client_nums最大为1000（以个人机子为依据）。




