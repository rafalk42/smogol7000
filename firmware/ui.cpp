#include <LCD_ST7032.h>

#include "ui.h"
#include "log.h"
#include "version.h"


char keepAliveIndicator[] =
{
  (char)0b00101101,
  (char)0b11111111,
  (char)0b00101101,
  (char)0b01011111
};

enum UiViews { UIV_LIVE, UIV_MINUTE, UIV_HOUR, UIV_DAY };
UiViews &operator++ (UiViews& view)
{
  switch (view)
  {
    case UIV_LIVE: return view = UIV_MINUTE;
    case UIV_MINUTE: return view = UIV_HOUR;
    case UIV_HOUR: return view = UIV_DAY;
    case UIV_DAY: return view = UIV_LIVE;
  }

  return view;
};

static LCD_ST7032 lcd;
static UiViews _uiView;
static bool _uiDirty;
static int _uiDataPageOffset;
static unsigned long _uiKeepAlive = 0;

void uiInitialize()
{
  lcd.begin();
  lcd.setcontrast(38); //contrast value range is 0-63, try 25@5V or 50@3.3V as a starting value
  
  lcd.clear();
  lcd.setCursor(0, 5);
  lcd.print("SMOGOL");
  lcd.setCursor(1, 4);
  lcd.print("7000 v");
  lcd.print(VERSION_MAJOR, DEC);
  lcd.print(".");
  lcd.print(VERSION_MINOR, DEC);

  _uiView = UIV_LIVE;
  _uiDirty = true;
  _uiDataPageOffset = 0;
}

void uiRender()
{
  if (!_uiDirty) return;
  _uiDirty = false;
  
  lcd.clear();

  lcd.setCursor(0, 15);
  lcd.print(keepAliveIndicator[_uiKeepAlive % sizeof(keepAliveIndicator)]);
  lcd.setCursor(0, 0);
  switch (_uiView)
  {
    case UIV_LIVE:
    {
      switch(_uiDataPageOffset)
      {
        case 0:
        {
          bool valid = logSampleLatestIsValid();
          PmsData data = logSampleGetLatest();
    
          lcd.print("LIVE");
          
          lcd.setCursor(1, 1);
          if (valid) lcd.print(data.atmPm01, DEC);
          else lcd.print('?');
          
          lcd.setCursor(1, 6);
          if (valid) lcd.print(data.atmPm25, DEC);
          else lcd.print('?');
          
          lcd.setCursor(1, 11);
          if (valid) lcd.print(data.atmPm10, DEC);
          else lcd.print('?');
        }
        break;

        case 1:
        {
          unsigned long now = millis();
          char text[16];
          sprintf(text, "%lud %luh%02lum%02lus",
            now / 1000 / 86400,
            (now / 1000 / 60 / 60) % 24,
            (now / 1000 / 60) % 60,
            (now / 1000) % 60);

          lcd.print("UPTIME");

          lcd.setCursor(1, 1);
          lcd.print(text);
        }
        break;

        case 2:
        {
          lcd.print("FIRMWARE v");
          lcd.print(VERSION_MAJOR, DEC);
          lcd.print(".");
          lcd.print(VERSION_MINOR, DEC);

          lcd.setCursor(1, 1);
          lcd.print(VERSION_BUILD_DATE);
        }
        break;

        case 3:
        {
          lcd.print("FIRMWARE v");
          lcd.print(VERSION_MAJOR, DEC);
          lcd.print(".");
          lcd.print(VERSION_MINOR, DEC);

          lcd.setCursor(1, 1);
          lcd.print(VERSION_BUILD_TIME);
        }
        break;
      }
    }
    break;

    case UIV_MINUTE:
    {
      bool valid = logAverage1mIsValid(_uiDataPageOffset);
      PmsData data = logGetAverage1m(_uiDataPageOffset);

      lcd.print("MINUTE");

      if (_uiDataPageOffset > 0)
      {
        lcd.print(-_uiDataPageOffset, DEC);
      }
      
      lcd.setCursor(1, 1);
      if (valid) lcd.print(data.atmPm01, DEC);
      else lcd.print('?');
      
      lcd.setCursor(1, 6);
      if (valid) lcd.print(data.atmPm25, DEC);
      else lcd.print('?');
      
      lcd.setCursor(1, 11);
      if (valid) lcd.print(data.atmPm10, DEC);
      else lcd.print('?');
    }
    break;
    
    case UIV_HOUR:
    {
      bool valid = logAverage1hIsValid(_uiDataPageOffset);
      PmsData data = logGetAverage1h(_uiDataPageOffset);

      lcd.print("HOUR");
 
      if (_uiDataPageOffset > 0)
      {
        lcd.print(-_uiDataPageOffset, DEC);
      }
     
      lcd.setCursor(1, 1);
      if (valid) lcd.print(data.atmPm01, DEC);
      else lcd.print('?');
      
      lcd.setCursor(1, 6);
      if (valid) lcd.print(data.atmPm25, DEC);
      else lcd.print('?');
      
      lcd.setCursor(1, 11);
      if (valid) lcd.print(data.atmPm10, DEC);
      else lcd.print('?');
    }
    break;
    
    case UIV_DAY:
      bool valid = logAverage1dIsValid();
      PmsData data = logGetAverage1d();

      lcd.print("DAY");
     
      lcd.setCursor(1, 1);
      if (valid) lcd.print(data.atmPm01, DEC);
      else lcd.print('?');
      
      lcd.setCursor(1, 6);
      if (valid) lcd.print(data.atmPm25, DEC);
      else lcd.print('?');
      
      lcd.setCursor(1, 11);
      if (valid) lcd.print(data.atmPm10, DEC);
      else lcd.print('?');
    break;
  }
}

void uiDataChange()
{
  _uiDirty = true;
}

void uiKeepAlive()
{
  _uiDirty = true;
  _uiKeepAlive++;
}

void uiViewNext()
{
  _uiDirty = true;
  ++_uiView;
  _uiDataPageOffset = 0;
}

void uiPageNext()
{
  _uiDirty = true;
  _uiDataPageOffset++;

  switch (_uiView)
  {
    case UIV_LIVE:
      if (_uiDataPageOffset >= 4) _uiDataPageOffset = 3;
    break;
    case UIV_MINUTE:
      if (_uiDataPageOffset >= 60) _uiDataPageOffset = 59;
    break;
    case UIV_HOUR:
      if (_uiDataPageOffset >= 24) _uiDataPageOffset = 23;
    break;
    case UIV_DAY:
      _uiDataPageOffset = 0;
    break;
  }
}

void uiPagePrevious()
{
  _uiDirty = true;
  _uiDataPageOffset--;

  if (_uiDataPageOffset < 0) _uiDataPageOffset = 0;
}
