#include <Arduino.h>
#include <OneButton.h>

#define BTN_PIN 2          // main button pin
#define BUZZER_PIN 3       // PWM pin for buzzer
#define PUMP_PIN 7         // output pin for connecting to the pump circuit
#define FLOW_SENSOR_PIN A5 // input pin for ACS712 sensor to mesure pump flow level

#define UNIT_VOLUME_DURATION 7000           // duration in ms for pouring a single unit of volume
#define EXPLICIT_PUMP_WARMUP_DURATION 0     // additional time to "warmup" the pump (stabilize the flow)
#define FLOW_LEVEL_BUFFER_SIZE 10           // number of the values to average
#define EMPTY_TANK_FLOW_LEVEL_THRESHOLD 530 // flow level threshold to identify when tank is empty

byte flowLevelBufferIdx = 0;                          // actual smoothing buffer index
unsigned int flowLevelBuffer[FLOW_LEVEL_BUFFER_SIZE]; // buffer array for flow level values to calculate avg
unsigned int flowLevelBufferSum = 0;                  // actual buffer sum
bool flowLevelBufferFull = false;                     // shows if the buffer contains all values
byte volume = 0;                                      // number of unit volumes to pour
unsigned long pumpStartTime = 0;                      // pump start timestamp
OneButton btn = OneButton(BTN_PIN, true, true);       // button object

void onBtnClick();
void onLongPressStart();
void onLongPressStop();
void onDoubleClick();
void onMultiClick();
void startPouring(byte volume);
void stopPouring();
bool isPouring();
bool isDone();
bool isTankEmpty();
int currentValue();
void resetCurrentBuffer();
void notifyEmptyTank();

void setup()
{
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

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

  if (isPouring() && isDone())
  {
    Serial.println("Done");
    stopPouring();
  }
  else if (isPouring() && isTankEmpty())
  {
    Serial.println("Tank is emtpy");
    stopPouring();
    notifyEmptyTank();
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
  resetCurrentBuffer(); // reset current sensor buffer

  pumpStartTime = millis();
  volume = vol;
  digitalWrite(PUMP_PIN, HIGH);
}

void stopPouring()
{
  pumpStartTime = 0;
  digitalWrite(PUMP_PIN, LOW);
}

bool isPouring()
{
  return pumpStartTime > 0;
}

bool isDone()
{
  return millis() - pumpStartTime >= UNIT_VOLUME_DURATION * volume;
}

bool isTankEmpty()
{
  return currentValue() < EMPTY_TANK_FLOW_LEVEL_THRESHOLD && millis() - pumpStartTime >= EXPLICIT_PUMP_WARMUP_DURATION && flowLevelBufferFull;
}

int currentValue()
{
  flowLevelBufferSum -= flowLevelBuffer[flowLevelBufferIdx];
  flowLevelBuffer[flowLevelBufferIdx] = analogRead(FLOW_SENSOR_PIN);
  Serial.print("current=");
  Serial.println(flowLevelBuffer[flowLevelBufferIdx]);
  flowLevelBufferSum += flowLevelBuffer[flowLevelBufferIdx];

  if (++flowLevelBufferIdx == FLOW_LEVEL_BUFFER_SIZE)
  {
    flowLevelBufferIdx = 0;
    flowLevelBufferFull = true;
  }

  return flowLevelBufferSum / FLOW_LEVEL_BUFFER_SIZE;
}

void resetCurrentBuffer()
{
  flowLevelBufferSum = 0;
  flowLevelBufferIdx = 0;
  flowLevelBufferFull = false;

  for (int i = 0; i < FLOW_LEVEL_BUFFER_SIZE; i++)
  {
    flowLevelBuffer[i] = 0;
  }
}

void notifyEmptyTank()
{
  for (int i = 0; i < 3; i++)
  {
    tone(BUZZER_PIN, 2000);
    delay(1000);
    noTone(BUZZER_PIN);
    delay(500);
  }
}