#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define PIN_D5   5
#define PIN_D18  18

const int M1_IN1 = 25;
const int M1_IN2 = 26;
const int M2_IN1 = 27;
const int M2_IN2 = 14;

const int PWM_FREQ = 20000;
const int PWM_RES  = 8;

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData;

const int maxSpeed = 255;
const int rotateSpeed = 145;

// Stability counters
int detectCounter = 0;
// int lostCounter = 0;

// const int detectThreshold = 3;   // frames needed to attack
// const int lostThreshold   = 3;   // frames needed to rotate

bool robotActive = false;
unsigned long startDelayTimer = 0;

// -------- MOTOR --------
void driveMotors(int speedA, int speedB)
{
  speedA = constrain(speedA, -255, 255);
  speedB = constrain(speedB, -255, 255);

  if (speedA > 0) {
    ledcWrite(M1_IN1, speedA);
    ledcWrite(M1_IN2, 0);
  } else {
    ledcWrite(M1_IN1, 0);
    ledcWrite(M1_IN2, abs(speedA));
  }

  if (speedB > 0) {
    ledcWrite(M2_IN1, speedB);
    ledcWrite(M2_IN2, 0);
  } else {
    ledcWrite(M2_IN1, 0);
    ledcWrite(M2_IN2, abs(speedB));
  }
}

// -------- DETECTION --------
// Returns the closest valid distance in mm within the scanned zones,
// or -1 if no valid target was found.
int rawObjectDistance()
{
  if (!myImager.isDataReady())
    return -1;

  if (!myImager.getRangingData(&measurementData))
    return -1;

  int minDist = 9999;
  int validZones = 0;

  for (int row = 3; row < 7; row++)
  {
    for (int col = 2; col <= 7; col++)
    {
      int i = row * 8 + col;
      int d = measurementData.distance_mm[i];
      int status = measurementData.target_status[i];

      if (status == 5 || status == 9)
      {
        validZones++;
        if (d > 0 && d < minDist)
          minDist = d;

      }
    }
  }

  if (validZones == 0)
    return -1;
  Serial.println(minDist);
  return minDist;
}

// -------- SETUP --------
void setup()
{
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_D5, OUTPUT);
  digitalWrite(PIN_D5, HIGH);

  pinMode(PIN_D18, INPUT);

  ledcAttach(M1_IN1, PWM_FREQ, PWM_RES);
  ledcAttach(M1_IN2, PWM_FREQ, PWM_RES);
  ledcAttach(M2_IN1, PWM_FREQ, PWM_RES);
  ledcAttach(M2_IN2, PWM_FREQ, PWM_RES);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  if (!myImager.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }

  myImager.setResolution(8 * 8);
  myImager.setRangingFrequency(15);
  myImager.startRanging();

  Serial.println("Ready.");
}

// -------- LOOP --------
void loop()
{
  digitalWrite(PIN_D5, HIGH);

  // STOP immediately
  if (digitalRead(PIN_D18) == LOW)
  {
    robotActive = false;
    detectCounter = 0;
    startDelayTimer = 0;
    driveMotors(0, 0);
    return;
  }

  // 5-second delay
  if (!robotActive)
  {
    if (startDelayTimer == 0)
      startDelayTimer = millis();

    if (millis() - startDelayTimer >= 5000)
    {
      robotActive = true;
      startDelayTimer = 0;
    }
    else
    {
      driveMotors(0, 0);
      return;
    }
  }

  // -------- ACTIVE MODE --------
  int minDist = rawObjectDistance();
  bool detected = (minDist > 0 && minDist < 950);

  if (detected)
  {
    detectCounter++;
  }
  else
  {
    detectCounter = 0;
  }

  if (detected)
  {
    // Attack
    driveMotors(-maxSpeed, maxSpeed);
    if (minDist < 120){
      driveMotors(-maxSpeed, maxSpeed);
      return;
    }
  }
  else
  {
    // Always rotate when not confidently detecting
    driveMotors(-rotateSpeed, -rotateSpeed);
  }
}