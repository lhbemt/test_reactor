# 记录下调试test_reactor
## coredump
coredump又叫核心转储，当程序运行过程中发生异常退出的时候，操作系统就会把当前的内存状况存储在一个core文件中，叫core_dump，一般文件名是core.PID。当程序异常终止时，进程用户空间的数据就会被写到磁盘，我们就可以通过debug这个core文件，来得知当初发生了什么，导致进程down了，coredump也可以叫做内存快照，记录了当时发生了什么状况。
### test_reactor的coredump生成
由cmake生成makefile文件，并编译完后，我立刻兴奋的启动了test_reactor，结果当即挂了：

	[li@localhost build]$ ./test_reactor 
	terminate called after throwing an instance of 'std::system_error'
	what():  Invalid argument
	Aborted (core dumped)
然而当我查看这个目录下的时候，却未发现任何coredump文件，因为我这个虚拟机是刚装不久的，就忘了设置core文件大小，因此系统默认的core文件大小为0，所以没生成coredump。可以用ulimit -a来查看系统默认的corefile大小。

	[li@localhost build]$ ulimit -a
	core file size          (blocks, -c) 0
	data seg size           (kbytes, -d) unlimited
	scheduling priority             (-e) 0
	file size               (blocks, -f) unlimited
	pending signals                 (-i) 7167
	max locked memory       (kbytes, -l) 64
	max memory size         (kbytes, -m) unlimited
	open files                      (-n) 1024
	pipe size            (512 bytes, -p) 8
	POSIX message queues     (bytes, -q) 819200
	real-time priority              (-r) 0
	stack size              (kbytes, -s) 8192
	cpu time               (seconds, -t) unlimited
	max user processes              (-u) 4096
	virtual memory          (kbytes, -v) unlimited
	file locks                      (-x) unlimited
可以看到，我的core file size这一行是0。
做出修改就是对core file大小不限制，使用下面这个命令。

	ulimit -c unlimited
执行命令后，ulimit -a 可以看到
	
	[li@localhost build]$ ulimit -a
	core file size          (blocks, -c) unlimited
此时再次执行test_reactor，dump文件就生成了：

	[li@localhost build]$ ls -l
	total 592
	-rw-rw-r-- 1 li li    13852 Apr  5 20:59 CMakeCache.txt
	drwxrwxr-x 5 li li      309 Apr  5 20:59 CMakeFiles
	-rw-rw-r-- 1 li li     2903 Apr  5 20:59 cmake_install.cmake
	-rw------- 1 li li 25763840 Apr  5 21:22 core.62065
	-rw-rw-r-- 1 li li    13742 Apr  5 20:59 Makefile
	-rwxrwxr-x 1 li li   140136 Apr  5 20:59 test_reactor
ps：每次关了终端，又回重新置为0，所以需要放到环境变量里: /etc/profile
### 使用coredump
coredump文件是使用gdb来查看的，gdb -c core.pid program_name

	gdb -c  core.62065 test_reactor
打开coredump文件后，使用bt或者where命令，都可以查看相应的调用栈，看你的程序down在哪里。

	(gdb) bt
	#0  0x00007fdd5e5eb207 in __GI_raise (sig=sig@entry=6) at ../nptl/sysdeps/unix/sysv/linux/raise.c:55
	#1  0x00007fdd5e5ec8f8 in __GI_abort () at abort.c:90
	#2  0x00007fdd5eefaa95 in __gnu_cxx::__verbose_terminate_handler() () from /lib64/libstdc++.so.6
	#3  0x00007fdd5eef8a06 in ?? () from /lib64/libstdc++.so.6
	#4  0x00007fdd5eef79b9 in ?? () from /lib64/libstdc++.so.6
	#5  0x00007fdd5eef8624 in __gxx_personality_v0 () from /lib64/libstdc++.so.6
	#6  0x00007fdd5e9918e3 in ?? () from /lib64/libgcc_s.so.1
	#7  0x00007fdd5e991c7b in _Unwind_RaiseException () from /lib64/libgcc_s.so.1
	#8  0x00007fdd5eef8c46 in __cxa_throw () from /lib64/libstdc++.so.6
	#9  0x00007fdd5ef4df30 in std::__throw_system_error(int) () from /lib64/libstdc++.so.6
	#10 0x00007fdd5ef4f0e8 in std::thread::join() () from /lib64/libstdc++.so.6
	#11 0x0000000000415077 in lemt::Reactor::~Reactor() ()
	#12 0x00000000004153bf in lemt::SubReactorSocket::SubReactorSocket(lemt::SubReactorSocket&&) ()
	#13 0x0000000000418bf0 in void __gnu_cxx::new_allocator<lemt::SubReactorSocket>::construct<lemt::SubReactorSocket, lemt::SubReactorSocket>(lemt::SubReactorSocket*, lemt::SubReactorSocket&&) ()
	#14 0x0000000000418434 in std::enable_if<std::allocator_traits<std::allocator<lemt::SubReactorSocket> >::__construct_helper<lemt::SubReactorSocket, lemt::SubReactorSocket>::value, void>::type std::allocator_traits<std::allocator<lemt::SubReactorSocket> >::_S_construct<lemt::SubReactorSocket, lemt::SubReactorSocket>(std::allocator<lemt::SubReactorSocket>&, lemt::SubReactorSocket*, lemt::SubReactorSocket&&) ()
	#15 0x0000000000416f92 in decltype (_S_construct({parm#1}, {parm#2}, (forward<lemt::SubReactorSocket>)({parm#3}))) std::allocator_traits<std::allocator<lemt::SubReactorSocket> >::construct<lemt::SubReactorSocket, lemt::SubReactorSocket>(std::allocator<lemt::SubReactorSocket>&, lemt::SubReactorSocket*, lemt::SubReactorSocket&&) ()
	#16 0x000000000041600f in void std::vector<lemt::SubReactorSocket, std::allocator<lemt::SubReactorSocket> >::emplace_back<lemt::SubReactorSocket>(lemt::SubReactorSocket&&) ()
	#17 0x000000000041571c in lemt::ReactorMgr::ReactorMgr() ()
	#18 0x00000000004157f0 in lemt::ReactorMgr::get_instance()::{lambda()#1}::operator()() const ()
	#19 0x0000000000418cc4 in void std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()>::_M_invoke<>(std::_Index_tuple<>) ()
	#20 0x0000000000418645 in std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()>::operator()() ()
	#21 0x00000000004173a0 in void std::__once_call_impl<std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()> >() ()
	#22 0x00007fdd5f1aee40 in pthread_once () at ../nptl/sysdeps/unix/sysv/linux/x86_64/pthread_once.S:103
	#23 0x0000000000414aa8 in __gthread_once(int*, void (*)()) ()
	#24 0x00000000004161db in void std::call_once<lemt::ReactorMgr::get_instance()::{lambda()#1}>(std::once_flag&, lemt::ReactorMgr::get_instance()::{lambda()#1}&&) ()
	#25 0x0000000000415858 in lemt::ReactorMgr::get_instance() ()
	#26 0x0000000000414c08 in main ()
从上面的调用栈可以看出，程序是挂在lemt::Reactor::~Reactor()这个析构函数中，调用了join()函数。涉及的是子reactor的初始化的时候，即临时对象析构的时候，挂了。
稍加分析下，因为我在start函数调用的时候，才会使线程运行：	
	

	void start() {
    	t = std::move(std::thread(&Reactor::run, this));
    }
而此时线程t变量还是未初始化的，所以此时join显然是错误的，修改方案就是先判断是否joinable：

	~Reactor() {
    	close(epoll_fd);
        close(stop_fd[0]);
        close(stop_fd[1]);
        if (t.joinable()) {
            t.join();
        }
    }
修改完后，即正常运行。
### stop问题
当我程序运行起来了，我是使用监听信号SIGINT，SIGTERM来notify终止while循环，然后退出reactor的，但是我发现我ctrl c后无反应，那显然就是代码有问题了，因此需要使用gdb来调试。

	void sig_handle(int sig) {
    	cv.notify_one();
	}

	int main(int argc, char* argv[]) {
    	signal(SIGPIPE, SIG_IGN);
    	signal(SIGINT, sig_handle);
    	signal(SIGTERM, sig_handle);
    	is_running = true;
    	lemt::ReactorMgr::get_instance()->start();
    	while (is_running) {
        	std::unique_lock<std::mutex> lk(m);
        	cv.wait(lk, [](){ return is_running; });
        	lemt::ReactorMgr::get_instance()->stop();
        	is_running = false;
    	}   
    	return 0;
	}
我需要在main函数中的第23行打一个断点，来确定下是信号量是否成功唤醒了，gbd支持直接filename:line的方式设置断点。

	(gdb) b test_reactor.cpp:23
	Breakpoint 1 at 0x420d3d: file /home/li/bpsj_server/reactor/src/test_reactor.cpp, line 23.
然后再run即可。在新起一个终端，kill即可发送SIGINT信号。

	[li@localhost build]$ ps -ef|grep reactor
	li        63201  62691  0 22:35 pts/1    00:00:00 gdb test_reactor
	li        63211  63201  0 22:36 pts/1    00:00:00 /home/li/bpsj_server/reactor/build/test_reactor
	li        63216  62531  0 22:37 pts/0    00:00:00 grep --color=auto reactor
	[li@localhost build]$ kill -2 63211
-2表示SIGINT信号，注意是向63211发送信号，这个是gdb里面运行的进程，可以看到父进程的pid是63201。
发送信号后，发现有收到信号，但是并没有进入断点，bt查看下当前堆栈，

	Program received signal SIGINT, Interrupt.
	pthread_once () at ../nptl/sysdeps/unix/sysv/linux/x86_64/pthread_once.S:94
	94              jmp     6b
	Missing separate debuginfos, use: debuginfo-install libgcc-4.8.5-44.el7.x86_64 libstdc++-4.8.5-44.el7.x86_64
	(gdb) bt
	#0  pthread_once () at ../nptl/sysdeps/unix/sysv/linux/x86_64/pthread_once.S:94
	#1  0x0000000000418ee8 in __gthread_once (__once=0x628518 <lemt::ReactorMgr::init_flag>, __func=0x418c60 <__once_proxy@plt>)
    at /usr/include/c++/4.8.2/x86_64-redhat-linux/bits/gthr-default.h:699
	#2  0x000000000041a8a1 in std::call_once<lemt::ReactorMgr::get_instance()::{lambda()#1}>(std::once_flag&, lemt::ReactorMgr::get_instance()::{lambda()#1}&&) (__once=..., __f=<unknown type in /home/li/bpsj_server/reactor/build/test_reactor, CU 0x0, DIE 0x13b61>)
    at /usr/include/c++/4.8.2/mutex:786
	#3  0x0000000000419db8 in lemt::ReactorMgr::get_instance () at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:39
	#4  0x0000000000418fb6 in lemt::Acceptor::Acceptor (this=0x62c090) at /home/li/bpsj_server/reactor/src/acceptor.cpp:10
	#5  0x0000000000419592 in lemt::MainReactor::MainReactor (this=0x629190)
    at /home/li/bpsj_server/reactor/include/main_reactor.hpp:11
	#6  0x0000000000419b00 in lemt::ReactorMgr::ReactorMgr (this=0x629190) at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:28
	#7  0x0000000000419d50 in lemt::ReactorMgr::get_instance()::{lambda()#1}::operator()() const (__closure=0x7fffffffe30b)
    at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:39
	#8  0x000000000041d8de in std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()>::_M_invoke<>(std::_Index_tuple<>) (
    this=0x7fffffffe30b) at /usr/include/c++/4.8.2/functional:1732
	#9  0x000000000041d0c9 in std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()>::operator()() (this=0x7fffffffe30b)
    at /usr/include/c++/4.8.2/functional:1720
	#10 0x000000000041bbb8 in std::__once_call_impl<std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()> >() ()
    at /usr/include/c++/4.8.2/mutex:754
	#11 0x00007ffff7bcbe40 in pthread_once () at ../nptl/sysdeps/unix/sysv/linux/x86_64/pthread_once.S:103
	#12 0x0000000000418ee8 in __gthread_once (__once=0x628518 <lemt::ReactorMgr::init_flag>, __func=0x418c60 <__once_proxy@plt>)
    at /usr/include/c++/4.8.2/x86_64-redhat-linux/bits/gthr-default.h:699
	#13 0x000000000041a8a1 in std::call_once<lemt::ReactorMgr::get_instance()::{lambda()#1}>(std::once_flag&, lemt::ReactorMgr::get_instance()::{lambda()#1}&&) (__once=..., __f=<unknown type in /home/li/bpsj_server/reactor/build/test_reactor, CU 0x0, DIE 0x13b61>)
    at /usr/include/c++/4.8.2/mutex:786
	#14 0x0000000000419db8 in lemt::ReactorMgr::get_instance () at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:39
	#15 0x0000000000420d06 in main (argc=1, argv=0x7fffffffe478) at /home/li/bpsj_server/reactor/src/test_reactor.cpp:19
好吧，发现是get_instance()这个函数卡住了。

	static std::unique_ptr<ReactorMgr>& get_instance() {
    	if (!instance) {
        	std::call_once(init_flag, [&](){ instance.reset(new ReactorMgr());});
        }
        return instance;
    }
在get_instance处和start处都加断点，看下到底是哪里的问题
	
	(gdb) b reactor_mgr.hpp:38
	(gdb) b reactor_mgr.hpp:29
run，发现卡在call_once

	Breakpoint 1, lemt::ReactorMgr::get_instance () at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:38
	38                  if (!instance) {
	Missing separate debuginfos, use: debuginfo-install libgcc-4.8.5-44.el7.x86_64 libstdc++-4.8.5-44.el7.x86_64
	(gdb) n
	39                      std::call_once(init_flag, [&](){ instance.reset(new ReactorMgr());});
	(gdb) n

	Breakpoint 1, lemt::ReactorMgr::get_instance () at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:38
	38                  if (!instance) {
	(gdb) n
	39                      std::call_once(init_flag, [&](){ instance.reset(new ReactorMgr());});
	(gdb) n
在n的时候卡在了，此时的get_instance进入了2次。同一个线程进入get_instance 2次，导致死锁了。分别在这两次的时候，输下bt命令，看下调用堆栈分别是什么。
第一次：
	
	(gdb) bt
	#0  lemt::ReactorMgr::get_instance () at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:38
	#1  0x0000000000420d06 in main (argc=1, argv=0x7fffffffe478) at /home/li/bpsj_server/reactor/src/test_reactor.cpp:20
20行是没错，是第一次调用get_instance()的地方，接着n执行，进入到第二次调用get_instance()的地方：

	(gdb) bt
	#0  lemt::ReactorMgr::get_instance () at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:38
	#1  0x0000000000418fb6 in lemt::Acceptor::Acceptor (this=0x62c090) at /home/li/bpsj_server/reactor/src/acceptor.cpp:10
	#2  0x0000000000419592 in lemt::MainReactor::MainReactor (this=0x629190)
    	at /home/li/bpsj_server/reactor/include/main_reactor.hpp:11
	#3  0x0000000000419b00 in lemt::ReactorMgr::ReactorMgr (this=0x629190) at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:28
	#4  0x0000000000419d50 in lemt::ReactorMgr::get_instance()::{lambda()#1}::operator()() const (__closure=0x7fffffffe30b)
    	at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:39
	#5  0x000000000041d8de in std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()>::_M_invoke<>(std::_Index_tuple<>) (
    	this=0x7fffffffe30b) at /usr/include/c++/4.8.2/functional:1732
	#6  0x000000000041d0c9 in std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()>::operator()() (this=0x7fffffffe30b)
    	at /usr/include/c++/4.8.2/functional:1720
	#7  0x000000000041bbb8 in std::__once_call_impl<std::_Bind_simple<lemt::ReactorMgr::get_instance()::{lambda()#1} ()> >() ()
    	at /usr/include/c++/4.8.2/mutex:754
	#8  0x00007ffff7bcbe40 in pthread_once () at ../nptl/sysdeps/unix/sysv/linux/x86_64/pthread_once.S:103
	#9  0x0000000000418ee8 in __gthread_once (__once=0x628518 <lemt::ReactorMgr::init_flag>, __func=0x418c60 <__once_proxy@plt>)
    	at /usr/include/c++/4.8.2/x86_64-redhat-linux/bits/gthr-default.h:699
	#10 0x000000000041a8a1 in std::call_once<lemt::ReactorMgr::get_instance()::{lambda()#1}>(std::once_flag&, lemt::ReactorMgr::get_instance()::{lambda()#1}&&) (__once=..., __f=<unknown type in /home/li/bpsj_server/reactor/build/test_reactor, CU 0x0, DIE 0x13b61>)
    at /usr/include/c++/4.8.2/mutex:786
	#11 0x0000000000419db8 in lemt::ReactorMgr::get_instance () at /home/li/bpsj_server/reactor/include/reactor_mgr.hpp:39
	#12 0x0000000000420d06 in main (argc=1, argv=0x7fffffffe478) at /home/li/bpsj_server/reactor/src/test_reactor.cpp:20
ok，立马发现，是在Acceptor构造函数的时候，调用了get_instance()来注册，导致死锁了。解决办法显然是Acceptor此时构造函数时不注册fd，而是添加一个start函数，这时候再来注册。
这里也发现了std::call_once的坑了，在call_once的初始化函数中，又调用了call_once，就会导致死锁了。

	std::error_code Acceptor::start() {
        return ReactorMgr::get_instance()->register_main_reactor(lfd, EPOLLIN);
    }
ok，新版本后再次开始调试我们的停止功能。
好吧，依旧不行，但是通过我们的打印调试大法，我们在singal那里和cv.await()函数那里加了std::cout，在按ctrl的时候，确实出现了打印，证明唤醒是没问题的，那就是在stop那里了，
此时我想说一句，打印调试大法好。信打印，得永生。

	[li@localhost build]$ ./test_reactor 
	^Csig_handle: 2
	notify stop 
接着加断点，看为何没有stop。
我们的stop函数，是通过写pipe管道，从而让epoll退出线程的方式：

	void stop() {
        write(stop_fd[1], "stop", strlen("stop"));
    }
    void Reactor::run() {
        while(true) {
            int ret = epoll_wait(epoll_fd, &event_array[0], event_num, -1);
            if (ret == -1 && errno == EINTR)
                continue;
            for (int i = 0; i < ret; ++i) {
                if (event_array[i].data.fd == stop_fd[0]) {
                    return; // stop
                }
                dispatch(event_array[i]);
            }
        }
    }
所以初步判断是stop_fd[0]没收取到消息，在发送消息那里和epoll_wait处加个断点看看。看是否有触发epoll_wait函数。
	
	(gdb) b reactor.cpp:27
	Breakpoint 1 at 0x420631: file /home/li/bpsj_server/reactor/src/reactor.cpp, line 27.
但是发现上面的kill -2发信号的方法不行。为了使应用程序能获取到信号，需要在gdb里设置一下：

	(gdb) handle SIGINT nostop print pass
这样在gdb里，直接按ctrl c，也能发送出SIGINT信号，从而进入断点：
	
	Program received signal SIGINT, Interrupt.
	sig_handle: 2
	notify stop 
	Breakpoint 2, lemt::Reactor::stop (this=0x629190) at /home/li/bpsj_server/reactor/include/reactor.hpp:40
	40                  write(stop_fd[1], "stop", strlen("stop"));
可见确实写入了pipe，continue执行的时候，也到了epoll_wait返回，也走到了return

	Breakpoint 1, lemt::Reactor::run (this=0x629190) at /home/li/bpsj_server/reactor/src/reactor.cpp:30
	30                  for (int i = 0; i < ret; ++i) {
	(gdb) n
	31                      if (event_array[i].data.fd == stop_fd[0]) {
	(gdb) n
	32                          return; // stop
	(gdb) n
	37          }
最后确实走完了整个流程，光荣的退出了：

	30                  for (int i = 0; i < ret; ++i) {
	(gdb) c
	Continuing.
	[Thread 0x7ffff5fcf700 (LWP 64907) exited]
	[Inferior 1 (process 64901) exited normally]
	(gdb) c
	The program is not being run.
	(gdb) c
	The program is not being run.
	(gdb)
但是很奇怪，直接运行的时候，ctrl c确实没退出，还是打印大法吧。先看看卡在那里。
	
	[li@localhost build]$ ./test_reactor 
	^Cnotify stop
	reactor stop end
	stop reactor
	stop reactor
	stop reactor
但是gdb中却是：
	
	[New Thread 0x7ffff6fd1700 (LWP 65095)]
	[New Thread 0x7ffff67d0700 (LWP 65096)]
	[New Thread 0x7ffff5fcf700 (LWP 65097)]
	[New Thread 0x7ffff57ce700 (LWP 65098)]
	^C
	Program received signal SIGINT, Interrupt.
	notify stop
	reactor stop end
	stop reactor
	stop reactor
	stop reactor
	stop reactor
	[Thread 0x7ffff6fd1700 (LWP 65095) exited]
	[Thread 0x7ffff67d0700 (LWP 65096) exited]
	[Thread 0x7ffff57ce700 (LWP 65098) exited]
	[Thread 0x7ffff5fcf700 (LWP 65097) exited]
	[Inferior 1 (process 65091) exited normally]
加上打印的日志：

	[li@localhost build]$ ./test_reactor 
	^Cnotify stop
	reactor stop begin: 5
	reactor stop begin: 12
	reactor stop begin: 15
	reactor stop begin: 18
	reactor stop end
	stop reactor: 4
	stop reactor: 14
	stop reactor: 17
stopfd已经发出，但是此时线程已经结束了，看下Reactor的析构函数：

	~Reactor() {
		close(epoll_fd);
        close(stop_fd[0]);
        close(stop_fd[1]);
        if (t.joinable()) {
            t.join();
        }
    }
显然看出了问题，先close再join的，close了管道的读，导致线程一直无法结束，解决方法很明显，换一下位置就好了：

	~Reactor() {
        if (t.joinable()) {
            t.join();
        }
        close(epoll_fd);
        close(stop_fd[0]);
        close(stop_fd[1]);
    }
最后能正确的结束了：
	
	[li@localhost build]$ ./test_reactor 
	^Cnotify stop
	reactor stop begin: 5
	reactor stop begin: 12
	reactor stop begin: 15
	stop reactor: reactor stop begin: 18
	4
	stop reactor: 14
	stop reactor: 17
	stop reactor: 11
	reactor stop end
至于为啥gdb的时候，不会出现这种情况，等待大神来解答下。。
## 一个巨坑
出现了一次Program terminated with signal 11, Segmentation fault的dump，这个原因是因为我的clients是std::map，而用[]取值的时候，如果key不存在，std::map会给你一个默认的值，这导致了我的client是nullptr，在handle的时候，直接炸了。
还有出现错误要去处理，下面这段代码之前我没去取消注册，导致clients里面找不到fd，直接内存错误的coredump。

	std::error_code register_fd(int fd, poll_event poll_events) {
        std::error_code ec = Reactor::register_fd(fd, poll_events);
        if (ec.value() == 0){
            clients.insert(std::make_pair(fd, std::make_shared<ClientSocket>(fd, idx))); // 这里可以使用mialloc等优化内存的第三方内存分配库
        }
        else { // 注册失败 关闭socket并取消注册 不然epoll_wait会返回导致 clients取到的fd的value是nullptr，炸了
            unregister_fd(fd);
            close(fd);
        }
        return ec;
    }
## accept数量不够问题--listen的第二个参数
我一开始设置的listen的第二个参数是5，然后客户端一次进行几百个连接的时候，发现accept的数量总是会少一些，于是我就看了下listen函数，它是这样描述的，
为了更好的理解backlog参数，我们必须认识到内核为任何一个给定的监听套接口维护两个队列：
1、未完成连接队列（incomplete connection queue），每个这样的SYN分节对应其中一项：已由某个客户发出并到达服务器，而服务器正在等待完成相应的TCP三路握手过程。这些套接口处于SYN_RCVD状态。
2、已完成连接队列（completed connection queue），每个已完成TCP三路握手过程的客户对应其中一项。这些套接口处于ESTABLISHED状态。
当来自客户的SYN到达时，TCP在未完成连接队列中创建一个新项，然后响应以三路握手的第二个分节：服务器的SYN响应，其中稍带对客户SYN的ACK（即SYN+ACK）。这一项一直保留在未完成连接队列中，直到三路握手的第三个分节（客户对服务器SYN的ACK）到达或者该项超时为止（曾经源自Berkeley的实现为这些未完成连接的项设置的超时值为75秒）。如果三路握手正常完成，该项就从未完成连接队列移到已完成连接队列的队尾。当进程调用accept时，已完成连接队列中的队头项将返回给进程，或者如果该队列为空，那么进程将被投入睡眠，直到TCP在该队列中放入一项才唤醒它。
而在man函数中：
“backlog参数确定了connection队列可以增长的最大长度”，如果connection队列已满（达到backlog确定的长度），那么新的connection请求到来时，客户端会得到一个ECONNREFUSED error，或者，如果底层协议支持重发，那么这个请求会被服务器忽略而使客户端重新发送connection请求。现在backlog用来确定已完成队列（完成三次握手等待accept）的长度，而不再是已完成队列和未完成连接队列之和（linux 2.2之前）。
未完成队列（incomplete connection queue）的长度现在由/proc/sys/net/ipv4/tcp_max_syn_backlog设置，在现在大多数最新linux内核都是默认512。
继续看backlog，如果我们给listen的backlog参数设值超过了/proc/sys/net/core/somaxconn，那么backlog参数的值为自动被改写为/proc/sys/net/core/somaxconn的值，它的默认大小为128.
所以我把listen函数的第二个参数改为了512。此时，所有accept的次数就对了。可以看到，都打满了。

	ret = listen(lfd, 512);

	[li@localhost build]$ ./client 1 1000
	connect_count: 1000
	done!
	accept: 996
	accept: 997
	accept: 998
	accept: 999
	accept: 1000



