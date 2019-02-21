#ifndef _PMS_H_
#define _PMS_H_

#include <stdint.h>


struct PmsData
{
//  uint16_t stdPm01;
//  uint16_t stdPm25;
//  uint16_t stdPm10;
  
  unsigned short atmPm01;
  unsigned short atmPm25;
  unsigned short atmPm10;
};

void pmsRegisterFrameCallback(void (*callback)(PmsData));
void pmsPushByte(uint8_t aByte);


#endif // _PMS_H_
