#ifndef _LOG_H_
#define _LOG_H_

#include "pms.h"


void logPush(PmsData frame);
bool logSampleLatestIsValid();
PmsData logSampleGetLatest();
bool logAverage1mIsValid(unsigned short offset);
PmsData logGetAverage1m(unsigned short offset);
bool logAverage1hIsValid(unsigned short offset);
PmsData logGetAverage1h(unsigned short offset);
bool logAverage1dIsValid();
PmsData logGetAverage1d();

#endif // _LOG_H_
