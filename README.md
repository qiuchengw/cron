# 关于cron

### cron的前身LittleT
[关于LittleT介绍](https://www.appinn.com/littlet/)：一个集任务跟踪、事件提醒、TODO 列表为一体的软件。

cron则是从LittleT项目中抠出来用于定时执行任务的代码。

LittleT的前身是大学的时候写的一个条件表达式定时器，首次发布在cnbeta上（原文已找不到了）。毕业后对其改进，美化了界面，加了一些新的功能，重命名为LittleT发布。LittleT现在也已经不再维护，但是其中的定时器管理器还是挺有用处的。所以提取出来命名为cron开源。

cron的原始代码写的太烂，几个月前，经过了好几天业余时间的摘取重构，算是能跑起来了，但代码还是非常丑陋粗暴，现在先发布出来，这两天集中时间再重构一下。


### cron特点
- 跨平台
- 时间精确到秒 （可以自定制TimerManager以实现更高精度的定时器）
- 解析字符串表达式定时
- 支持相对时间定时
- 支持绝对时间定时

### 目前的问题
- 代码太烂！需要强力重构。
- 没有检测进程的启动和退出相关的代码实现（之前写的一个windows下的进程检测代码，不能跨平台）

### 定时器的使用
![定时器的功能](https://github.com/qiuchengw/cron/raw/master/sc.png)

#### 相对时间表达式
```cpp
    //  相对时间表达式：
    //	R = 1; P = 20m; Q = 22m; C = 100;
    //		R: 相对于1（TASK_EXEC_AFTERSYSBOOT）
    //  	P : 20minutes后执行任务 P[1s~24h]
    //  	Q : 然后每个22minutes执行一次 Q[1s~24h]
    //  	C : 执行100次后停止，C[1~86400]
```

#### 绝对时间表达式
```cpp
    //  绝对时间表达式：
    //  A = d; S = d; E = d; P = d; T = d1, d2, d3...; X = s;
    //
    //  A: 执行时间标志ENUM_TASK_EXECFLAG
    //
    //  	S, E : 起止日期，任务的有效期
    //
    //  	P : 单个INT值
    //  	间隔日期：A = TASK_EXEC_ATDAILY有效，单位天。 比如每隔3天 P = 3;
    //
    //  Q: 单个INT值
    //  	提前天数：A = AUTOTASK_EXEC_ATYEARDAY有效，单位天，比如提前3天
    //
    //  	T : 执行时间点，一个或多个INT值，此值可解析为绝对时间
    //  	比如一天中的13：30，14：30，16：20。。。
    //
    //  	X : 1个多个整数值，以‘, ’分隔，具体含义由A决定
    //  	1 > A = TASK_EXEC_ATDATE, X为多个日期点，可解析为日期
    //  	2 > A = TASK_EXEC_ATDAILY, X为一个单个值，保存P间隔的天数
    //  	3 > A = TASK_EXEC_ATMONTHDAY, X一个整数值代表为月的日号
    //  	此数值为哪一天可执行，使用位标示，共32位，最低位为1号..
    //  	4 > A = TASK_EXEC_ATWEEKDAY, X包含一个整数值，
    //  	此值表示哪一天（星期日到星期六）可执行，使用位标识表示，ENUM_NTH_WEEKDAY
    //  	5 > A = AUTOTASK_EXEC_ATYEARDAY，X为一个整数值，代表[月 - 日]
    //  	每年的某个日期 2011~2020 的[11 / 11]，可以有一个提前量。类似于生日提醒
```

### 代码的集成使用
```cpp
    MyTask a_task;
	// 启动任务后5s执行一次，然后每隔2s执行一次，共计执行5次后停止
	if (auto t = ExpTimer::create("R=2;P=5s; Q = 2s; C = 5;")) {
		t->startFrom(dt::now(), [](void *p) {
			reinterpret_cast<MyTask*>(p)->doTask();
		}, &a_task);
	}
```

### TODO
- [x] 完善README说明
- [ ] 清洁代码
    - [x] 统一文件名命名风格
    - [ ] 统一方法/变量/类等命名风格
    - [x] 使用namespace隔离代码
    - [x] 清理无效代码
    - [ ] 去掉windows平台依赖（shell仅支持windows，暂不跨平台）
- [x] 使用cmake管理项目
- [ ] 支持gcc 5.4.0编译
- [ ] 支持linux平台
- [ ] 相对时间：检测系统/进程启动时间

