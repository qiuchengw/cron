#pragma once

namespace cron {

	namespace dict {

		inline mstring getRunningStatusDescription(TimerRunningStatus eStatus) {
			switch (eStatus) {
			case TimerRunningStatus::kBadTimer://-2://	// 不能解析timer表达式
				return "无效的定时器";
			case TimerRunningStatus::kAppError://-1://	// 应用程序出现了错误
				return "应用程序错误";
			case TimerRunningStatus::kOk://0://	// 任务正常启动
				return "一切正常";
			case TimerRunningStatus::kNotStartup://1://	// 任务还未启动
				return "任务还未启动";
			case TimerRunningStatus::kPaused: // 暂停中
				return "任务暂停";
			case TimerRunningStatus::kOverdue://	// 任务过期了
				return "任务过期";
			case TimerRunningStatus::kUntilNextSysReboot://	// 需要下次机器重启，任务才执行
				return "下次开机执行";
			case TimerRunningStatus::kUntilNextAppReboot:	// 需要程序重启，任务才执行
				return "下次启动程序运行";
			case TimerRunningStatus::kBasedOnExternalApp://	// 依赖的外部程序并没有运行
				return "依赖的外部程序并没有运行";
				//////////////////////////////////////////////////////////////////////////
				// 绝对时间
			case TimerRunningStatus::kTimeNotMatch://	// 无可执行的时间匹配
				return "无可执行的时间匹配";
			case TimerRunningStatus::kNoChanceExec://	// 虽然任务未过期，但是余下的时间里，任务都没有机会再执行了
				return "任务没有机会再执行";
			default:
				return "未知标志";
			}
		}

		inline mstring getExecFlagText(TimerExecType eFlag) {
			switch (eFlag) {
			case TimerExecType::kNotSet: // 0,	// 未设置
				return "未设置";
				// 相对时间
			case TimerExecType::kAfterSysBoot: // 0x00000001,	// 系统启动
				return "系统启动";
			case TimerExecType::kAfterTimerStart: // 0x00000002,	// 任务启动
				return "任务启动后";
			case TimerExecType::kAfterAppStart: // 0x00000004,// 本程序启动
				return "本程序启动";
			case TimerExecType::kAfterExternalAppStart: // 0x00000008,// 外部程序启动
				return "相对于外部程序启动";
			case TimerExecType::kAfterAppExit: // 0x00000010,// 外部程序退出
				return "相对于外部程序退出";
				// 绝对时间标记
			case TimerExecType::kAtDate: // 0x00010000,	// 绝对日期 2011/11/11
				return "日期";
			case TimerExecType::kAtYearDay: // 0x00010000,	// 多个日期 2011/11/11
				return "每年的某天";
			case TimerExecType::kAtDaily: // 0x00020000,	// 每隔x天
				return "每隔x天";
			case TimerExecType::kAtMonthDay: // 0x00040000,	// 每月的x号
				return "每月的x号";
			case TimerExecType::kAtWeekday: // 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
				return "每月的x周";

				//////////////////////////////////////////////////////////////////////////
			case TimerExecType::kExecAndThen: // 0x01000000,	// 相对时间之后再次执行多次间隔
				return "相对时间之后再次执行多次间隔";
			}
			ASSERT(false);
			return "GetExecFlagText->未知参数";
		}

		inline mstring timeUnitStr(char unit) {
			switch (unit) {
			case 's':
			case 'S':
				return "秒";
			case 'm':
			case 'M':
				return "分";
			case 'h':
			case 'H':
				return "时";
			default:
				ASSERT(false);
				return "无效单位";
			}
		}


	}

}

