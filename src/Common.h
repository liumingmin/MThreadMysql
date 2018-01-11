#ifndef _COMMON_H_
#define _COMMON_H_

typedef enum _trace_level
{
	SYS_FATAL = 0,
	SYS_BUG = 1,
	SYS_ERROR = 2,
	SYS_WARNING = 3,
	SYS_INFO1 = 4,
	SYS_INFO2 = 5,
	SYS_INFO3 = 6,
	SYS_INFO4 = 7,
	SYS_INFO5 = 8,
	SYS_INFO6 = 9,
	SYS_INFO7 = 10,
	SYS_INFO8 = 11,
	END_TRACE_LEVEL = 12
}
sys_trace_level_t;

int sys_trace (char *fi, int li, sys_trace_level_t level, char *chfr, ...);

#define LogText(Level, ...)  sys_trace(__FILE__, __LINE__, (sys_trace_level_t)Level, __VA_ARGS__)

#endif


