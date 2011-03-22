#ifndef __WLOG_H__
#define __WLOG_H__
/**
 * @file wlog.h
 * @brief WLog日志输出接口，适用于简单日志输出，可供后台程序或内核使用.
 * <pre>正确的include方法：在你要的公共头文件里面include <wlog.h> 
        其他的地方include 你的公共头文件，不要在每个地方去include <wlog.h>
        why？wlog里面使用的大量的内联与宏，如果你要定义自己的宏宏配置选项，
        在多个地方include <wlog.h>而忘记同步修改宏配置选项，会生成多种
        不同的展开表达式，导致最终结果并不是你想要的。
        此日志当前支持4种风格，皆为变参：
            1. logXXX   最常用，注意由于使用宏的原因，第一个参数不要直接输出非常量字符串，如要输出string str;需要logXXX(_T("%s"), str.c_str()); 而不是直接logXXX(str.c_str())。
            2. logXXXC  带条件的输出，此条件放在第一个参数上，条件为动态检测，如logXXXC(nAge > 18, "你已经大于18岁了，年龄(%d)", nAge)。
            3. logXXXN  此系列是用来输出没有结尾标志'\0'的字符串的，但可以将字符输出成其他格式，如logXXXN("%02X", szBuf, 10, "将szBuf前10位当成整数输出，真实长度(%d)", 444)。
            4. logXXXCN 此系列结合了C与N的功能，除了系统一个参数是条件外，与logXXXN功用法相同。
 * usage:
		你可以在include之前修改“宏”配置，包括
		1. WLOG_TO, 用户太默认是输出到CONSOLE，内核态默认输出到KERNEL
		2. WLOG_STATIC_TYPE_SWITCH，设置静态(编译时)开关，默认是输出所有日志类型，
			即#define WLOG_STATIC_TYPE_SWITCH (WLOG_TYPE_ALL)
		3. WLOG_DYNAMIC_TYPE_SWITCH，是否启动动态(运行时)开关，默认不启用，
			即#define WLOG_DYNAMIC_TYPE_SWITCH 0，如果启用的话需要定义全局变量
			unsigned int g_wlogDynamicTypeSwitch = (这里调节你要动态开启的日志类型);
		4. WLOG_DYNAMIC_CHECK_LOG_FILE，如果当前WLOG_TO定义成WLOG_TO_FILE，此值可
			设置是否检测日志文件存在才写日志，默认是如果不存在直接创建，即
			#define WLOG_DYNAMIC_CHECK_LOG_FILE 0，一般在Release版本开启此开关有利于今后调试
		5. WLOG_FILE_NAME，如果当前WLOG_TO定义成WLOG_TO_FILE，此值可
			设置日志文件存放的具体路径，默认是 #define WLOG_FILE_NAME _T("wlog.default.log")
		#include <wlog.h>
		你可以在这里修改变量配置，包括
		1. unsigned int g_wlogDynamicTypeSwitch，如果WLOG_DYNAMIC_TYPE_SWITCH定义成1，需要定义此变量，并可动态改变此变量的值
			如g_wlogDynamicTypeSwitch = (WLOG_TYPE_TEXT | WLOG_TYPE_BASE | WLOG_TYPE_DEBUG | WLOG_TYPE_NOTICE);  
	</pre>
 * @os windows, linux
 * @author wtd, weitidong220@163.com
 *
 * @date
 	<ul>
	   <li>20101130 --- v1.00   支持windows与linux后台程序，支持windows下unicode与ansi，支持输出到console, file </li>
	   <li>20101203 --- v1.01   增加logAssert与logVerify的实现 </li>
	   <li>20101217 --- v1.10   增加logXXXN系列 </li>
	   <li>20101218 --- v1.11   修改windows下共享写入文件，使用fsopen </li>
	   <li>20101218 --- v1.12   修改使其能在LINUX KERNEL下使用 </li>
	   <li>20101223 --- v1.13   修正logXXXN系列的打印输出函数__wlog_log_text_n，使其正确打印偏移<br/>
					--- v1.14   将format time定义成内联函数，减少预处理后代码量 </li>
	   <li>20101224 --- v1.15   优化logXXXN系列，减少一个szSuffix参数，据经验来看，几乎使用不到szSuffix这个参数<br/>
					--- v1.16   将WLOG_TO_FILE的handle换成static变量，不再需要在源文件中申明File * g_wlogOutFileHandle = NULL </li>
	   <li>20101225 --- v1.17   修复logXXXN系列在静态开关部分失效问题 --- merry christmas 2010  </li>
	   <li>20101229 --- v2.00   添加了logXXXC, logXXXCN系列，判断condition为真时才打印日志。主要方便linux kernel模式下过滤数据包。</li>
       <li>20101230 --- V2.01   修复linux用户态下未include <time.h>错误</li>
       <li>20110113 --- V2.02   将logXXXN系列修改成变参方式</li>
       <li>20110121 --- V2.03   修改写文件预定义，对于没有预定义WLOG_TO的，如果定义了WLOG_FILE_NAME也默认写入文件</li>
       <li>20110226 --- V2.04   添加一些用法注释，发布成公共代码</li>
	</ul>
 */

//输出类型
#define WLOG_TO_CONSOLE		(0x01 << 1)
#define WLOG_TO_IDE			(0x01 << 2)
#define WLOG_TO_KERNEL		(0x01 << 3)
#define WLOG_TO_FILE		(0x01 << 4)

//默认输出
#ifndef WLOG_TO
	#ifdef _WIN32
		#if defined(WLOG_FILE_NAME)
            #define WLOG_TO	WLOG_TO_FILE
		#else
            #define WLOG_TO	WLOG_TO_CONSOLE
		#endif		
		//#pragma message("default define WLOG_TO WLOG_TO_CONSOLE")
		
	#else
		#if defined(__KERNEL__) //内核暂时不支持写文件！
			#define WLOG_TO	WLOG_TO_KERNEL
		#else
            #if defined(WLOG_FILE_NAME)
                #define WLOG_TO	WLOG_TO_FILE
            #else
                #define WLOG_TO	WLOG_TO_CONSOLE
            #endif
		#endif
	#endif
	
#endif

//include
#ifdef _WIN32
	#if	  (WLOG_TO == WLOG_TO_CONSOLE)
	#elif (WLOG_TO == WLOG_TO_IDE)
	#elif (WLOG_TO == WLOG_TO_KERNEL)
	#elif (WLOG_TO == WLOG_TO_FILE)
	#else
		#error "you must define WLOG_TO to a valid type!"
	#endif

	#include <tchar.h>
	#if (WLOG_TO == WLOG_TO_CONSOLE) || (WLOG_TO == WLOG_TO_IDE) || (WLOG_TO == WLOG_TO_FILE)
		#include <stdio.h>
		#include <time.h>
		#include <assert.h>
		#include <shlwapi.h>
		#pragma comment(lib, "shlwapi.lib")
	#endif

	
	#include <share.h>
#else
	#if	  (WLOG_TO == WLOG_TO_CONSOLE)
	#elif (WLOG_TO == WLOG_TO_IDE)
	#elif (WLOG_TO == WLOG_TO_KERNEL)
	#elif (WLOG_TO == WLOG_TO_FILE)
	#else
		#error "you must define WLOG_TO to a valid type!"
	#endif
	
	#if (WLOG_TO == WLOG_TO_CONSOLE) || (WLOG_TO == WLOG_TO_IDE) || (WLOG_TO == WLOG_TO_FILE)
        #include <stdio.h>
		#include <time.h>
		#include <assert.h>
	#endif
	#include <stdarg.h>
	#include <unistd.h>
#endif

#ifndef WLOG_MAX_BUFFER_SIZE
	#define WLOG_MAX_BUFFER_SIZE 512
#endif

//日志类型
#define WLOG_TYPE_TEXT		(0x01 << 1)
#define WLOG_TYPE_BASE		(0x01 << 2)
#define WLOG_TYPE_TRACE		(0x01 << 3)
#define WLOG_TYPE_DEBUG		(0x01 << 4)
#define WLOG_TYPE_INFO		(0x01 << 5)
#define WLOG_TYPE_NOTICE	(0x01 << 6)
#define WLOG_TYPE_WARNING	(0x01 << 7)
#define WLOG_TYPE_ERROR		(0x01 << 8)
#define WLOG_TYPE_VERIFY	(0x01 << 9)
#define WLOG_TYPE_ASSERT	(0x01 << 10)
#define WLOG_TYPE_FATAL		(0x01 << 11)
#define WLOG_TYPE_ALL		(WLOG_TYPE_TEXT | WLOG_TYPE_BASE | WLOG_TYPE_TRACE | WLOG_TYPE_DEBUG | WLOG_TYPE_INFO | WLOG_TYPE_NOTICE | WLOG_TYPE_WARNING | WLOG_TYPE_ERROR | WLOG_TYPE_VERIFY | WLOG_TYPE_ASSERT | WLOG_TYPE_FATAL)

//默认打开所有静态编译日志类型开关
#ifndef WLOG_STATIC_TYPE_SWITCH
	#define WLOG_STATIC_TYPE_SWITCH WLOG_TYPE_ALL
#endif

//是否使用动态编译日志类型开关
#if WLOG_DYNAMIC_TYPE_SWITCH
	extern unsigned int g_wlogDynamicTypeSwitch;
	#define WLOG_DYNAMIC_CHECK(type) if (!(type & g_wlogDynamicTypeSwitch)) break
	#define WLOG_DYNAMIC_CHECK_TEXT if (WLOG_TYPE_TEXT & g_wlogDynamicTypeSwitch)
#else
	#define WLOG_DYNAMIC_CHECK(type)
	#define WLOG_DYNAMIC_CHECK_TEXT
#endif

//unicode处理事务
#if defined(_UNICODE) || defined(UNICODE)
	#define _STR2WIDE(x) L ## x
	#define STR2WIDE(x) _STR2WIDE(x)
	#define __TFILE__ STR2WIDE(__FILE__)
	
	#ifndef WLOG_OPEN_APPEND
		#define WLOG_OPEN_APPEND _T(",ccs=utf-8")
	#endif
#else
	#define __TFILE__ __FILE__
	#ifndef _T
		#define _T(x) x 
	#endif
	#ifndef WLOG_OPEN_APPEND
		#define WLOG_OPEN_APPEND
	#endif
#endif

//输出设置
#if (WLOG_TO == WLOG_TO_FILE)
	#ifndef WLOG_FILE_NAME
		#define WLOG_FILE_NAME _T("w.log")
	#endif
#endif

//检测文件是否存在
#if WLOG_DYNAMIC_CHECK_LOG_FILE && defined(_WIN32)
	#define WLOG_CHECK_FILE_EXIST if (!PathFileExists(WLOG_FILE_NAME)) return
#elif WLOG_DYNAMIC_CHECK_LOG_FILE
	#define WLOG_CHECK_FILE_EXIST if (0 != access(WLOG_FILE_NAME,W_OK)) return
#else
	#define WLOG_CHECK_FILE_EXIST 
#endif

//针对windows版本的日志接口定义
#ifdef _WIN32	
	//logText
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_TEXT)
		#if (WLOG_TO == WLOG_TO_CONSOLE)
			#define logText(format, ...)  WLOG_DYNAMIC_CHECK_TEXT _tprintf(format,__VA_ARGS__)
		#elif (WLOG_TO == WLOG_TO_KERNEL)
			#error "haven't implement!"
		#elif (WLOG_TO == WLOG_TO_IDE)
			inline void __wlog_ide_write_imp(const TCHAR* format, ...) {
				va_list arglist;
				va_start(arglist, format);
				TCHAR buffer[WLOG_MAX_BUFFER_SIZE];
				TCHAR* pBuffer = buffer;
				int nNeedSize = _vsctprintf(format, arglist) + 1;// _vscprintf doesn't count
				if (nNeedSize > WLOG_MAX_BUFFER_SIZE) {
					pBuffer = (TCHAR*)malloc(nNeedSize * sizeof(TCHAR));
				}
				if ((NULL != pBuffer) && (nNeedSize-1) == _vstprintf_s(pBuffer, nNeedSize, format, arglist)) {
					OutputDebugString(pBuffer);
				}
				if (nNeedSize > WLOG_MAX_BUFFER_SIZE && NULL != pBuffer)free(pBuffer);
				va_end(arglist);
			}
			#define logText(format, ...)  WLOG_DYNAMIC_CHECK_TEXT __wlog_ide_write_imp(format,__VA_ARGS__)
		#else
			inline void __wlog_file_write_valist_imp(const TCHAR* format, va_list arglist) {
				static FILE *g_wlogOutFileHandle = NULL;
				if (g_wlogOutFileHandle == NULL) {
					WLOG_CHECK_FILE_EXIST;
					g_wlogOutFileHandle = _tfsopen(WLOG_FILE_NAME, _T("a"), _SH_DENYNO);
					//if (0 != _tfopen_s(&g_wlogOutFileHandle, WLOG_FILE_NAME,_T("w")WLOG_OPEN_APPEND)) return;
					if(g_wlogOutFileHandle) {
						_ftprintf(g_wlogOutFileHandle,_T("\n++++++++++W+++++++++++\n"));
					} else return;
				}
				if(_vftprintf_s(g_wlogOutFileHandle, format, arglist) < 0) {
					g_wlogOutFileHandle = NULL;
				} else {		
					fflush(g_wlogOutFileHandle);
				}
			}
			inline void __wlog_file_write_imp(const TCHAR* format, ...) {
				va_list arglist;
				va_start(arglist, format);
				__wlog_file_write_valist_imp(format, arglist);
				va_end(arglist);
			}
			#define logText(format, ...) WLOG_DYNAMIC_CHECK_TEXT __wlog_file_write_imp(format, __VA_ARGS__)
		#endif				
		inline void __wlog_log_text_n(const TCHAR* szFormat, const TCHAR* szBuf, int nPrintCount) {
			if(0 == szFormat || 0 == szBuf || nPrintCount == 0) {
				logText(_T("CAN'T LOG TEXT N {format(%p), szBuf(%p), printcount(%d)}\n"), (void*)szFormat, szBuf, nPrintCount);
				return;
			}
			const TCHAR* pPrintStart = (const TCHAR*)szBuf;
			if(nPrintCount < 0) {
				pPrintStart = pPrintStart + nPrintCount;
				nPrintCount = -nPrintCount;
			}		    
			unsigned int nIdx = 0;
			while(nIdx < (unsigned int)nPrintCount) {
				#if defined(_UNICODE) || defined(UNICODE)
					logText(szFormat, (wchar_t)pPrintStart[nIdx++]);
				#else
					logText(szFormat, (unsigned char)pPrintStart[nIdx++]);
				#endif
			}
			logText(_T("\n"));
		}
		#define logTextC(condition, format, ...) if(condition) logText(format, __VA_ARGS__)
		#define logTextN(szFormat, szBuf, nPrintCount) WLOG_DYNAMIC_CHECK_TEXT __wlog_log_text_n(szFormat, szBuf, nPrintCount)
		#define logTextCN(condition, szFormat, szBuf, nPrintCount) if(condition) logTextN(szFormat, szBuf, nPrintCount)
	#endif

	//logBase
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_BASE)
		inline void __wlog_format_time_imp(TCHAR * pOutBuffer, unsigned int nBufferCount) {
			time_t timeData = time(0);
			struct tm winTM;
			if (!localtime_s(&winTM, &timeData)) {
				winTM.tm_mon += 1;
				_sntprintf_s(pOutBuffer, nBufferCount, _TRUNCATE,_T("%02d-%02d %02d:%02d:%02d"), winTM.tm_mon, winTM.tm_mday,winTM.tm_hour,winTM.tm_min,winTM.tm_sec);
			} else {
				memcpy_s(pOutBuffer, nBufferCount*sizeof(TCHAR), _T("<error time>"), nBufferCount*sizeof(TCHAR));
			}
		}
		#define logBase(nType, chType, format, ...)  do {\
			WLOG_DYNAMIC_CHECK(nType);\
			TCHAR __wlog_tmp_ctime_buf[15];\
			__wlog_format_time_imp(__wlog_tmp_ctime_buf, 15);\
			logText(_T("%s %c %s:%-4d| ") format _T("\n"), __wlog_tmp_ctime_buf, chType, _tcsrchr(__TFILE__, _T('\\'))+1,__LINE__,__VA_ARGS__);\
		} while (0)
		#define logBaseC(condition, nType, chType, format, ...) if(condition) logBase(nType, chType, format, __VA_ARGS__)
		#define logBaseN(nType, chType, szFormat, szBuf, nPrintCount, format, ...) do {\
			WLOG_DYNAMIC_CHECK(nType); \
			logBase(nType, chType, _T("[START](%d) ")format, nPrintCount, __VA_ARGS__);\
			logTextN(szFormat, szBuf, nPrintCount);\
			logBase(nType, chType, _T("[E.N.D]"));\
		} while (0)
		#define logBaseCN(condition, nType, chType, szFormat, szBuf, nPrintCount, format, ...) if(condition) logBaseN(nType, chType, szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif

	//log other type
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_TRACE)
		#define logTrace(format, ...) logBase(WLOG_TYPE_TRACE, _T('T'), format, __VA_ARGS__)
		#define logTraceC(condition, format, ...) logBaseC(condition, WLOG_TYPE_TRACE, _T('T'), format, __VA_ARGS__)
		#define logTraceN(szFormat, szBuf, nPrintCount, format, ...) logBaseN(WLOG_TYPE_TRACE, _T('T'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
		#define logTraceCN(condition, szFormat, szBuf, nPrintCount, format, ...) logBaseCN(condition, WLOG_TYPE_TRACE, _T('T'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_DEBUG)
		#define logDebug(format, ...) logBase(WLOG_TYPE_DEBUG, _T('D'), format, __VA_ARGS__)
		#define logDebugC(condition, format, ...) logBaseC(condition, WLOG_TYPE_DEBUG, _T('D'), format, __VA_ARGS__)
		#define logDebugN(szFormat, szBuf, nPrintCount, format, ...) logBaseN(WLOG_TYPE_DEBUG, _T('D'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
		#define logDebugCN(condition, szFormat, szBuf, nPrintCount, format, ...) logBaseCN(condition, WLOG_TYPE_DEBUG, _T('D'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_INFO)
		#define logInfo(format, ...) logBase(WLOG_TYPE_INFO, _T('I'), format, __VA_ARGS__)
		#define logInfoC(condition, format, ...) logBaseC(condition, WLOG_TYPE_INFO, _T('I'), format, __VA_ARGS__)
		#define logInfoN(szFormat, szBuf, nPrintCount, format, ...) logBaseN(WLOG_TYPE_INFO, _T('I'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
		#define logInfoCN(condition, szFormat, szBuf, nPrintCount, format, ...) logBaseCN(condition, WLOG_TYPE_INFO, _T('I'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_NOTICE)
		#define logNotice(format, ...) logBase(WLOG_TYPE_NOTICE, _T('N'), format, __VA_ARGS__)
		#define logNoticeC(condition, format, ...) logBaseC(condition, WLOG_TYPE_NOTICE, _T('N'), format, __VA_ARGS__)
		#define logNoticeN(szFormat, szBuf, nPrintCount, format, ...) logBaseN(WLOG_TYPE_NOTICE, _T('N'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
		#define logNoticeCN(condition, szFormat, szBuf, nPrintCount, format, ...) logBaseCN(condition, WLOG_TYPE_NOTICE, _T('N'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_WARNING)
		#define logWarning(format, ...) logBase(WLOG_TYPE_WARNING, _T('W'), format, __VA_ARGS__)
		#define logWarningC(condition, format, ...) logBaseC(condition, WLOG_TYPE_WARNING, _T('W'), format, __VA_ARGS__)
		#define logWarningN(szFormat, szBuf, nPrintCount, format, ...) logBaseN(WLOG_TYPE_WARNING, _T('W'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
		#define logWarningCN(condition, szFormat, szBuf, nPrintCount, format, ...) logBaseCN(condition, WLOG_TYPE_WARNING, _T('W'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif	
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_VERIFY)
		#define logVerify(condition) if(!(condition)){logBase(WLOG_TYPE_VERIFY, _T('V'), _T(#condition));abort();}
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_ASSERT) && (defined(_DEBUG) || defined(DEBUG))
		#define logAssert(condition) if(!(condition)){logBase(WLOG_TYPE_ASSERT, _T('A'), _T(#condition));assert(condition);}
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_ERROR)
		#define logError(format, ...) logBase(WLOG_TYPE_ERROR, _T('E'), format, __VA_ARGS__)
		#define logErrorC(condition, format, ...) logBaseC(condition, WLOG_TYPE_ERROR, _T('E'), format, __VA_ARGS__)
		#define logErrorN(szFormat, szBuf, nPrintCount, format, ...) logBaseN(WLOG_TYPE_ERROR, _T('E'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
		#define logErrorCN(condition, szFormat, szBuf, nPrintCount, format, ...) logBaseCN(condition, WLOG_TYPE_ERROR, _T('E'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_FATAL)
		#define logFatal(format, ...) logBase(WLOG_TYPE_FATAL, _T('F'), format, __VA_ARGS__)
		#define logFatalC(condition, format, ...) logBaseC(condition, WLOG_TYPE_FATAL, _T('F'), format, __VA_ARGS__)
		#define logFatalN(szFormat, szBuf, nPrintCount, format, ...) logBaseN(WLOG_TYPE_FATAL, _T('F'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
		#define logFatalCN(condition, szFormat, szBuf, nPrintCount, format, ...) logBaseCN(condition, WLOG_TYPE_FATAL, _T('F'), szFormat, szBuf, nPrintCount, format, __VA_ARGS__)
	#endif
#else// not _WIN32
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_TEXT)
		#if (WLOG_TO == WLOG_TO_CONSOLE)
			#define logText(format, args...)  WLOG_DYNAMIC_CHECK_TEXT printf(format,##args)
		#elif (WLOG_TO == WLOG_TO_KERNEL)
			#define logText(format, args...)  WLOG_DYNAMIC_CHECK_TEXT printk(format,##args)
		#else
			inline void __wlog_file_write_valist_imp(const char* format, va_list arglist) {
				static FILE *g_wlogOutFileHandle = NULL;
				if (g_wlogOutFileHandle == NULL) {
					WLOG_CHECK_FILE_EXIST;
					g_wlogOutFileHandle = fopen(WLOG_FILE_NAME,_T("a"));
					if(g_wlogOutFileHandle) {
						fprintf(g_wlogOutFileHandle,_T("\n++++++++++WLOG+++++++++++\n"));
					} else return;
				}
				if (vfprintf(g_wlogOutFileHandle, format, arglist) < 0) {
					g_wlogOutFileHandle = NULL;
				} else {
					fflush(g_wlogOutFileHandle);
				}
			}
			inline void __wlog_file_write_imp(const char* format, ...) {
				va_list arglist;
				va_start(arglist, format);
				__wlog_file_write_valist_imp(format, arglist);
				va_end(arglist);
			}
			#define logText(format, args...) WLOG_DYNAMIC_CHECK_TEXT __wlog_file_write_imp(format, ##args)
		#endif
		inline void __wlog_log_text_n(const char* szFormat, const char* szBuf, int nPrintCount) {
			if(0 == szFormat || 0 == szBuf || nPrintCount == 0) {
				logText(_T("CAN'T LOG TEXT N {format(%p), szBuf(%p), printcount(%d)}\n"), (void*)szFormat, szBuf, nPrintCount);
				return;
			}		    
			unsigned char* pPrintStart = (unsigned char*)szBuf;
			if(nPrintCount < 0) {
				pPrintStart = pPrintStart + nPrintCount;
				nPrintCount = -nPrintCount;
			}		    
			unsigned int nIdx = 0;
			while(nIdx < (unsigned int)nPrintCount) {
				logText(szFormat, (unsigned char)pPrintStart[nIdx++]);
			}
			logText(_T("\n"));
		}
		#define logTextC(condition, format, args...) if(condition) logText(format, ##args)		
		#define logTextN(szFormat, szBuf, nPrintCount) WLOG_DYNAMIC_CHECK_TEXT __wlog_log_text_n(szFormat, szBuf, nPrintCount)
		#define logTextCN(condition, szFormat, szBuf, nPrintCount) if(condition) logTextN(szFormat, szBuf, nPrintCount)
	#endif
	
	//logBase
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_BASE)
        #if (WLOG_TO == WLOG_TO_KERNEL)
            #define logBase(nType, chType, format, args...)  do {\
                WLOG_DYNAMIC_CHECK(nType); \
                logText(_T("%lu %c %s:%-4d| ")format _T("\n"), jiffies, chType, __FILE__,__LINE__,##args);\
            } while (0)
        #else
			inline void __wlog_format_time_imp(char * pOutBuffer, unsigned int nBufferCount) {
				time_t timeData = time(NULL);
				struct tm *p = localtime(&timeData);
				p->tm_mon += 1;
				snprintf(pOutBuffer,nBufferCount,_T("%02d-%02d %02d:%02d:%02d"), p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
			}
            #define logBase(nType, chType, format, args...)  do {\
                WLOG_DYNAMIC_CHECK(nType); \
                char __wlog_tmp_ctime_buf[15];\
                __wlog_format_time_imp(__wlog_tmp_ctime_buf, 15);\
                logText(_T("%s %c %s:%-4d| ") format _T("\n"), __wlog_tmp_ctime_buf, chType, __FILE__,__LINE__,##args);\
            } while (0)
        #endif
        #define logBaseC(condition, nType, chType, format, args...) if(condition) logBase(nType, chType, format, ##args)
		#define logBaseN(nType, chType, szFormat, szBuf, nPrintCount, format, args...) do {\
			WLOG_DYNAMIC_CHECK(nType); \
			logBase(nType, chType, _T("[START](%d) ")format, nPrintCount, ##args);\
			logTextN(szFormat, szBuf, nPrintCount);\
			logBase(nType, chType, _T("[E.N.D]"));\
		} while (0)
		#define logBaseCN(condition, nType, chType, szFormat, szBuf, nPrintCount, format, args...) if(condition) logBaseN(nType, chType, szFormat, szBuf, nPrintCount, format, ##args)
	#endif

	//log other type
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_TRACE)
        #define logTrace(format, args...) logBase(WLOG_TYPE_TRACE, _T('T'), format, ##args)
        #define logTraceC(condition, format, args...) logBaseC(condition, WLOG_TYPE_TRACE, _T('T'), format, ##args)
		#define logTraceN(szFormat, szBuf, nPrintCount, format, args...) logBaseN(WLOG_TYPE_TRACE, _T('T'), szFormat, szBuf, nPrintCount, format, ##args)
		#define logTraceCN(condition, szFormat, szBuf, nPrintCount, format, args...) logBaseCN(condition, WLOG_TYPE_TRACE, _T('T'), szFormat, szBuf, nPrintCount, format, ##args)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_DEBUG)
		#define logDebug(format, args...) logBase(WLOG_TYPE_DEBUG, _T('D'), format, ##args)
		#define logDebugC(condition, format, args...) logBaseC(condition, WLOG_TYPE_DEBUG, _T('D'), format, ##args)
		#define logDebugN(szFormat, szBuf, nPrintCount, format, args...) logBaseN(WLOG_TYPE_DEBUG, _T('D'), szFormat, szBuf, nPrintCount, format, ##args)
		#define logDebugCN(condition, szFormat, szBuf, nPrintCount, format, args...) logBaseCN(condition, WLOG_TYPE_DEBUG, _T('D'), szFormat, szBuf, nPrintCount, format, ##args)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_INFO)
		#define logInfo(format, args...) logBase(WLOG_TYPE_INFO, _T('I'), format, ##args)
		#define logInfoC(condition, format, args...) logBaseC(condition, WLOG_TYPE_INFO, _T('I'), format, ##args)
		#define logInfoN(szFormat, szBuf, nPrintCount, format, args...) logBaseN(WLOG_TYPE_INFO, _T('I'), szFormat, szBuf, nPrintCount, format, ##args)
		#define logInfoCN(condition, szFormat, szBuf, nPrintCount, format, args...) logBaseCN(condition, WLOG_TYPE_INFO, _T('I'), szFormat, szBuf, nPrintCount, format, ##args)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_NOTICE)
		#define logNotice(format, args...) logBase(WLOG_TYPE_NOTICE, _T('N'), format, ##args)
		#define logNoticeC(condition, format, args...) logBaseC(condition, WLOG_TYPE_NOTICE, _T('N'), format, ##args)
		#define logNoticeN(szFormat, szBuf, nPrintCount, format, args...) logBaseN(WLOG_TYPE_NOTICE, _T('N'), szFormat, szBuf, nPrintCount, format, ##args)
		#define logNoticeCN(condition, szFormat, szBuf, nPrintCount, format, args...) logBaseCN(condition, WLOG_TYPE_NOTICE, _T('N'), szFormat, szBuf, nPrintCount, format, ##args)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_WARNING)
		#define logWarning(format, args...) logBase(WLOG_TYPE_WARNING, _T('W'), format, ##args)
		#define logWarningC(condition, format, args...) logBaseC(condition, WLOG_TYPE_WARNING, _T('W'), format, ##args)
		#define logWarningN(szFormat, szBuf, nPrintCount, format, args...) logBaseN(WLOG_TYPE_WARNING, _T('W'), szFormat, szBuf, nPrintCount, format, ##args)
		#define logWarningCN(condition, szFormat, szBuf, nPrintCount, format, args...) logBaseCN(condition, WLOG_TYPE_WARNING, _T('W'), szFormat, szBuf, nPrintCount, format, ##args)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_VERIFY)
		#define logVerify(condition) if(!(condition)){logBase(WLOG_TYPE_VERIFY, _T('V'), _T(#condition)); abort();}
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_ASSERT) && (defined(_DEBUG) || defined(DEBUG))
		#define logAssert(condition) if(!(condition)){logBase(WLOG_TYPE_ASSERT, _T('A'), _T(#condition)); assert(condition);}
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_ERROR)
		#define logError(format, args...) logBase(WLOG_TYPE_ERROR, _T('E'), format, ##args)
		#define logErrorC(condition, format, args...) logBaseC(condition, WLOG_TYPE_ERROR, _T('E'), format, ##args)
		#define logErrorN(szFormat, szBuf, nPrintCount, format, args...) logBaseN(WLOG_TYPE_ERROR, _T('E'), szFormat, szBuf, nPrintCount, format, ##args)
		#define logErrorCN(condition, szFormat, szBuf, nPrintCount, format, args...) logBaseCN(condition, WLOG_TYPE_ERROR, _T('E'), szFormat, szBuf, nPrintCount, format, ##args)
	#endif
	#if (WLOG_STATIC_TYPE_SWITCH&WLOG_TYPE_FATAL)
		#define logFatal(format, args...) logBase(WLOG_TYPE_FATAL, _T('F'), format, ##args)
		#define logFatalC(condition, format, args...) logBaseC(condition, WLOG_TYPE_FATAL, _T('F'), format, ##args)
		#define logFatalN(szFormat, szBuf, nPrintCount, format, args...) logBaseN(WLOG_TYPE_FATAL, _T('F'), szFormat, szBuf, nPrintCount, format, ##args)
		#define logFatalCN(condition, szFormat, szBuf, nPrintCount, format, args...) logBaseCN(condition, WLOG_TYPE_FATAL, _T('F'), szFormat, szBuf, nPrintCount, format, ##args)
	#endif
#endif//_WIN32

#ifdef _WIN32
	#ifndef logText
		#define logText(format, ...)
		#define logTextC(condition, format, ...)
		#define logTextN(szFormat, szBuf, nPrintCount)
		#define logTextCN(condition, szFormat, szBuf, nPrintCount)
	#endif
	#ifndef logBase
		#define logBase(nType, chType, format, ...)
		#define logBaseC(condition, nType, chType, format, ...)
		#define logBaseN(nType, chType, szFormat, szBuf, nPrintCount, format, ...)
		#define logBaseCN(condition, nType, chType, szFormat, szBuf, nPrintCount, format, ...)
	#endif
	#ifndef logTrace
		#define logTrace(format, ...)
		#define logTraceC(condition, format, ...)
		#define logTraceN(szFormat, szBuf, nPrintCount, format, ...)
		#define logTraceCN(condition, szFormat, szBuf, nPrintCount, format, ...)
	#endif
	#ifndef logDebug
		#define logDebug(format, ...)
		#define logDebugC(condition, format, ...)
		#define logDebugN(szFormat, szBuf, nPrintCount, format, ...)
		#define logDebugCN(condition, szFormat, szBuf, nPrintCount, format, ...)
	#endif
	#ifndef logInfo
		#define logInfo(format, ...)
		#define logInfoC(condition, format, ...)
		#define logInfoN(szFormat, szBuf, nPrintCount, format, ...)
		#define logInfoCN(condition, szFormat, szBuf, nPrintCount, format, ...)
	#endif
	#ifndef logNotice
		#define logNotice(format, ...)
		#define logNoticeC(condition, format, ...)
		#define logNoticeN(szFormat, szBuf, nPrintCount, format, ...)
		#define logNoticeCN(condition, szFormat, szBuf, nPrintCount, format, ...)
	#endif
	#ifndef logWarning
		#define logWarning(format, ...)
		#define logWarningC(condition, format, ...)
		#define logWarningN(szFormat, szBuf, nPrintCount, format, ...)
		#define logWarningCN(condition, szFormat, szBuf, nPrintCount, format, ...)
	#endif
	#ifndef logVerify
		#define logVerify(format, ...)
	#endif
	#ifndef logAssert
		#define logAssert(format, ...)
	#endif
	#ifndef logError
		#define logError(format, ...)
		#define logErrorC(condition, format, ...)
		#define logErrorN(szFormat, szBuf, nPrintCount, format, ...)
		#define logErrorCN(condition, szFormat, szBuf, nPrintCount, format, ...)
	#endif	
	#ifndef logFatal
		#define logFatal(format, ...)
		#define logFatalC(condition, format, ...)
		#define logFatalN(szFormat, szBuf, nPrintCount, format, ...)
		#define logFatalCN(condition, szFormat, szBuf, nPrintCount, format, ...)
	#endif
#else //not windows
	#ifndef logText
		#define logText(format, args...)
		#define logTextC(condition, format, args...)
		#define logTextN(szFormat, szBuf, nPrintCount)
		#define logTextCN(condition, szFormat, szBuf, nPrintCount)
	#endif
	#ifndef logBase
		#define logBase(nType, chType, format, args...)
		#define logBaseC(condition, nType, chType, format, args...)
		#define logBaseN(nType, chType, szFormat, szBuf, nPrintCount, format, args...)
		#define logBaseCN(condition, nType, chType, szFormat, szBuf, nPrintCount, format, args...)
	#endif
	#ifndef logTrace
		#define logTrace(format, args...)
		#define logTraceC(condition, format, args...)
		#define logTraceN(szFormat, szBuf, nPrintCount, format, args...)
		#define logTraceCN(condition, szFormat, szBuf, nPrintCount, format, args...)
	#endif
	#ifndef logDebug
		#define logDebug(format, args...)
		#define logDebugC(condition, format, args...)
		#define logDebugN(szFormat, szBuf, nPrintCount, format, args...)
		#define logDebugCN(condition, szFormat, szBuf, nPrintCount, format, args...)
	#endif
	#ifndef logInfo
		#define logInfo(format, args...)
		#define logInfoC(condition, format, args...)
		#define logInfoN(szFormat, szBuf, nPrintCount, format, args...)
		#define logInfoCN(condition, szFormat, szBuf, nPrintCount, format, args...)
	#endif
	#ifndef logNotice
		#define logNotice(format, args...)
		#define logNoticeC(condition, format, args...)
		#define logNoticeN(szFormat, szBuf, nPrintCount, format, args...)
		#define logNoticeCN(condition, szFormat, szBuf, nPrintCount, format, args...)
	#endif
	#ifndef logWarning
		#define logWarning(format, args...)
		#define logWarningC(condition, format, args...)
		#define logWarningN(szFormat, szBuf, nPrintCount, format, args...)
		#define logWarningCN(condition, szFormat, szBuf, nPrintCount, format, args...)
	#endif	
	#ifndef logVerify
		#define logVerify(format, args...)
	#endif
	#ifndef logAssert
		#define logAssert(format, args...)
	#endif
	#ifndef logError
		#define logError(format, args...)
		#define logErrorC(condition, format, args...)
		#define logErrorN(szFormat, szBuf, nPrintCount, format, args...)
		#define logErrorCN(condition, szFormat, szBuf, nPrintCount, format, args...)
	#endif
	#ifndef logFatal
		#define logFatal(format, args...)
		#define logFatalC(condition, format, args...)
		#define logFatalN(szFormat, szBuf, nPrintCount, format, args...)
		#define logFatalCN(condition, szFormat, szBuf, nPrintCount, format, args...)
	#endif
#endif

#endif //__WLOG_H__
