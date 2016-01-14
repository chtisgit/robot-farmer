#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include "color.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_INTERNAL Logger().stream()
#define LOG LOG_INTERNAL << SH_GRN << "[robot_farmer] " << SH_RST << SH_PRP << __FILENAME__ << ":" << SH_L_TRQ << __LINE__ << SH_RST << " "

#ifdef DEBUG
#define DEBUG_LOG(x) LOG << x
#define DEBUG_LOG_NONL(x) LOG_INTERNAL << x
#else
#define DEBUG_LOG(x) 
#define DEBUG_LOG_NONL(x)
#endif

#endif
