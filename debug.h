#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <cstdio>
#include <cstdlib>

#define OUTS(e,...) printf(e, ##__VA_ARGS__)

#ifdef DEBUG
#	define DMSG(e,...) OUTS("[%s:%d] " e, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#	define DMSG(e,...) do{}while(0)
#endif

#endif