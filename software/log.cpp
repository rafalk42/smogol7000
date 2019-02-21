#include <stddef.h>
#include <arduino.h>

#include "log.h"


class Log
{
  public:
    Log(const char *name, unsigned short size) :
      name(name),
      size(size),
      logNext(NULL)
    {
      init();
    };
    Log(const char *name, unsigned short size, Log *logNext) :
      name(name),
      size(size),
      logNext(logNext)
    {
      init();
    };

    void push(PmsData data)
    {
      buffer[index++] = data;
      count++;

      bool overflow = false;
      // clamp count to the size
      if (count > size) count = size;

      // wrap around
      if (index == size)
      {
        index = 0;
        overflow = true;
      }

      if (overflow && logNext != NULL)
      {
        logNext->push(average());
      }

      if (Serial)
      {
        Serial.print(name);
        Serial.print(" i:");
        Serial.print(index, DEC);
        Serial.print(" c:");
        Serial.print(count, DEC);
        Serial.print(" data:");
        Serial.print(data.atmPm01, DEC);
        Serial.print(", ");
        Serial.print(data.atmPm25, DEC);
        Serial.print(", ");
        Serial.print(data.atmPm10, DEC);
        Serial.println();
      }
    };

    unsigned short isSampleValid(unsigned short offset)
    {
      return (int)offset < count;
    }

    PmsData getSample(unsigned short offset)
    {
      return buffer[(index - 1 - offset + count) % count];
    }

    PmsData average()
    {
      unsigned long averagePm01 = 0;
      unsigned long averagePm25 = 0;
      unsigned long averagePm10 = 0;
      
      for(int i=0; i<count; i++)
      {
        averagePm01 += buffer[i].atmPm01;
        averagePm25 += buffer[i].atmPm25;
        averagePm10 += buffer[i].atmPm10;
      }
      
      averagePm01 /= count;
      averagePm25 /= count;
      averagePm10 /= count;
      
      return PmsData{(unsigned short)averagePm01, (unsigned short)averagePm25, (unsigned short)averagePm10};
    }

    bool averageIsValid()
    {
      return count > 0;
    }
    
  private:
    const char *name;
    const int size;
    int count;
    int index;
    PmsData *buffer;
    Log *logNext;

    void init()
    {
      buffer = new PmsData[size];
      count = 0;
      index = 0;
    };
};

static Log log1d("1d", 24);
static Log log1h("1h", 60, &log1d);
//static Log log1h("1h", 60);
static Log log1m("1m", 60, &log1h);

void logPush(PmsData frame)
{
 log1m.push(frame);
}

bool logSampleLatestIsValid()
{
  return log1m.isSampleValid(0);
}

PmsData logSampleGetLatest()
{
  return log1m.getSample(0);
}

bool logAverage1mIsValid(unsigned short offset)
{
  if (offset > 0)
  {
    return log1h.isSampleValid(offset - 1);
  }
  else
  {
    return log1m.averageIsValid();
  }
}

PmsData logGetAverage1m(unsigned short offset)
{
  if (offset > 0)
  {
    return log1h.getSample(offset - 1);
  }
  else
  {
    return log1m.average();
  }
}

bool logAverage1hIsValid(unsigned short offset)
{
  if (offset > 0)
  {
    return log1d.isSampleValid(offset - 1);
  }
  else
  {
    return log1h.averageIsValid();
  }
}

PmsData logGetAverage1h(unsigned short offset)
{
  if (offset > 0)
  {
    return log1d.getSample(offset - 1);
  }
  else
  {
    return log1h.average();
  }
}

bool logAverage1dIsValid()
{
  return log1d.averageIsValid();
}

PmsData logGetAverage1d()
{
 return log1d.average();
}
