#include <SoftwareSerial.h>

#include "pms.h"
#include "log.h"
#include "ui.h"

#define PIN_ROTARY_DATA   4
#define PIN_ROTARY_CLOCK  7
#define PIN_ROTARY_BUTTON 8


static int rotaryLastClock;
static uint8_t rotaryMovementClockwise;
static uint8_t rotaryMovementCounterclockwise;
static int rotaryButtonLast;
void rotaryClockIsr()
{
  int nowData = digitalRead(PIN_ROTARY_DATA);
  int nowClock = digitalRead(PIN_ROTARY_CLOCK);

  if ((rotaryLastClock - nowClock) * (nowData * 2 - 1) > 0)
  { // clockwise
    rotaryMovementClockwise = 1;
  }
  else
  { // counter-clockwise
    rotaryMovementCounterclockwise = 1;
  }
  rotaryLastClock = nowClock;
}

int frameReceiveIndicator;
void pmsOnFrame(PmsData frame)
{
  frameReceiveIndicator++;
//  if (Serial) Serial.println("PMS on frame");
  logPush(frame);

  uiDataChange();

  if (Serial)
  {
    Serial.print("PMS frame received: ");
    Serial.print(frameReceiveIndicator, DEC);
    Serial.println();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  pmsRegisterFrameCallback(&pmsOnFrame);

  Serial1.begin(9600);

  pinMode(PIN_ROTARY_BUTTON, INPUT);
  pinMode(PIN_ROTARY_DATA, INPUT);
  pinMode(PIN_ROTARY_CLOCK, INPUT);

  rotaryLastClock = digitalRead(PIN_ROTARY_CLOCK);
  rotaryMovementClockwise = 0;
  rotaryMovementCounterclockwise = 0;
  rotaryButtonLast = 0;
  attachInterrupt(digitalPinToInterrupt(PIN_ROTARY_CLOCK), rotaryClockIsr, CHANGE); 

  uiInitialize();
}

void rotaryOnButton()
{
  if (Serial) Serial.println("Rotary BTN");
  uiViewNext();
}

void rotaryOnClockwise()
{
  if (Serial) Serial.println("Rotary CW");
  uiPageNext();
}

void rotaryOnCounterClockwise()
{
  if (Serial) Serial.println("Rotary CCW");
  uiPagePrevious();
}

void processRotary()
{
  int rotaryButtonState = !digitalRead(PIN_ROTARY_BUTTON);
  if ((!rotaryButtonLast) && rotaryButtonState)
  {
    rotaryOnButton();
  }
  rotaryButtonLast = rotaryButtonState;
  
  if (rotaryMovementClockwise)
  {
    rotaryMovementClockwise = 0;
    rotaryOnClockwise();
  }
  
  if (rotaryMovementCounterclockwise)
  {
    rotaryMovementCounterclockwise = 0;
    rotaryOnCounterClockwise();
  }
}

char thingy[] = {0b00101101, 0b11111111, 0b00101101, 0b01011111};
unsigned long previousMillis = 0;
void loop()
{
  while (Serial1.available()) pmsPushByte(Serial1.read());
  processRotary();
  uiRender();

  delay(1);
}
