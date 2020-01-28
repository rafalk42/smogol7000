#include <stddef.h>
#include <string.h>

#include "pms.h"


void (*onFrameCallback)(PmsData) = NULL;

void pmsRegisterFrameCallback(void (*callback)(PmsData))
{
  onFrameCallback = callback;
}

#define PMS_FRAME_DATA_LENGTH   0x1C

static enum {READING_HEADER, READING_DATA} _pmsFrameState;
static int _pmsBufferIndex = 0;
static uint8_t _pmsBuffer[32];
static uint8_t _pmsHeader[4] = {0x42, 0x4D, 0x00, 0x1C};
void _pmsBufferReset()
{
  _pmsBufferIndex = 0;
}

void _pmsBufferPush(uint8_t aByte)
{
  _pmsBuffer[_pmsBufferIndex++] = aByte;
}

PmsData _pmsFrameDataParse(uint8_t *data)
{
  PmsData frame;

//  frame.stdPm01 = (data[0] << 8) + data[1];
//  frame.stdPm25 = (data[2] << 8) + data[3];
//  frame.stdPm10 = (data[4] << 8) + data[5];

  frame.atmPm01 = (data[6] << 8) + data[7];
  frame.atmPm25 = (data[8] << 8) + data[9];
  frame.atmPm10 = (data[10] << 8) + data[11];

  return frame;
}

void pmsPushByte(uint8_t aByte)
{
  switch (_pmsFrameState)
  {
    case READING_HEADER:
      _pmsBufferPush(aByte);
      if (strncmp(_pmsBuffer, (const char*)_pmsHeader, _pmsBufferIndex) != 0)
      {
        _pmsBufferReset();
      }
      else
      {
        if(_pmsBufferIndex == sizeof(_pmsHeader))
        {
          _pmsFrameState = READING_DATA;
        }
      }
    break;

    case READING_DATA:
      _pmsBufferPush(aByte);
      if (_pmsBufferIndex == PMS_FRAME_DATA_LENGTH)
      {
        // parse buffer
        PmsData frame = _pmsFrameDataParse(_pmsBuffer);
        if (onFrameCallback != NULL) onFrameCallback(frame);

        // reset and wait for another frame
        _pmsBufferReset();
        _pmsFrameState = READING_HEADER;
      }
    break;
  }
}
