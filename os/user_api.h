#ifndef __USER_API_H__
#define __USER_API_H__

#include "type.h"

#ifdef RV32
extern int sleep(uint32_t tick);
#else
extern int sleep(uint64_t tick);
#endif

#endif