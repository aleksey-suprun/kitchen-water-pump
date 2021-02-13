#include <Arduino.h>
#include <OneButton.h>

// Duration in ms for pouring a single unit of volume
#define UNIT_VOLUME_DURATION 7000

// Main button pin
#define BTN_PIN 2

// Output pin for connecting to the pump circuit
#define PUMP_PIN 3

byte volume = 0;
unsigned long pouringStartTime = 0;
OneButton btn = OneButton(BTN_PIN, true, true);

void onBtnClick();
void onLongPressStart();
void onLongPressStop();
void onDoubleClick();
void onMultiClick();
void startPouring(byte volume);
void stopPouring();
bool isPouring();
bool isDone();

void setup()
{
  pinMode(PUMP_PIN, OUTPUT);

  digitalWrite(PUMP_PIN, LOW);

  btn.attachClick(onBtnClick);
  btn.attachLongPressStart(onLongPressStart);
  btn.attachLongPressStop(onLongPressStop);
  btn.attachDoubleClick(onDoubleClick);
  btn.attachMultiClick(onMultiClick);

  Serial.begin(9600);
  Serial.println("Setup finished");
}

void loop()
{
  btn.tick();

  if (isDone())
  {
    stopPouring();
  }
}

void onBtnClick()
{
  if (isPouring())
  {
    stopPouring();
  }
  else
  {
    startPouring(1);
  }
}

void onLongPressStart()
{
  startPouring(100);
}

void onLongPressStop()
{
  stopPouring();
}

void onMultiClick()
{
  int volume = btn.getNumberClicks();
  startPouring(volume);
}

void onDoubleClick()
{
  startPouring(2);
}

void startPouring(byte vol)
{
  pouringStartTime = millis();
  volume = vol;
  digitalWrite(PUMP_PIN, HIGH);
}

void stopPouring()
{
  pouringStartTime = 0;
  digitalWrite(PUMP_PIN, LOW);
}

bool isPouring()
{
  return pouringStartTime > 0;
}

bool isDone()
{
  return isPouring() && millis() - pouringStartTime >= UNIT_VOLUME_DURATION * volume;
}