#include <Wire.h>
#include <Arduino_LSM6DS3.h>
#include <Adafruit_PWMServoDriver.h>
#include <math.h>

// Gesture-controlled two-servo robotic arm
// Board: Arduino Nano 33 IoT
// Driver: PCA9685 at its default I2C address (0x40)

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

const uint8_t UP_DOWN_CHANNEL = 3;
const uint8_t TWIST_CHANNEL   = 4;

// Arm positions described in the project specification.
const int UP_DOWN_UP_ANGLE   = 0;
const int UP_DOWN_REST_ANGLE = 90;
const int UP_DOWN_DOWN_ANGLE = 180;

const int TWIST_LEFT_ANGLE   = 0;
const int TWIST_REST_ANGLE   = 0;
const int TWIST_RIGHT_ANGLE  = 180;

// Adjust these two values if a gesture moves a servo the wrong way.
const bool REVERSE_UP_DOWN = false;
const bool REVERSE_TWIST   = false;

// Maximum hand tilt used to command a full servo movement.
const float MAX_GESTURE_ANGLE = 45.0f;
const float GESTURE_DEAD_ZONE = 4.0f;

// Typical pulse range for 180-degree hobby servos. Reduce this range if a
// servo strains or buzzes at an endpoint.
const uint16_t SERVO_MIN_US = 500;
const uint16_t SERVO_MAX_US = 2500;
const float SERVO_FREQUENCY = 50.0f;

float ax = 0.0f;
float ay = 0.0f;
float az = 1.0f;
float neutralPitch = 0.0f;
float neutralRoll = 0.0f;
float filteredPitch = 0.0f;
float filteredRoll = 0.0f;

float clampFloat(float value, float low, float high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

float applyDeadZone(float value) {
  if (fabs(value) <= GESTURE_DEAD_ZONE) return 0.0f;

  if (value > 0.0f) {
    return value - GESTURE_DEAD_ZONE;
  }
  return value + GESTURE_DEAD_ZONE;
}

void calculateTilt(float &pitch, float &roll) {
  // Accelerometer tilt angles in degrees. These are stable while the hand is
  // still or moving slowly, which suits position control better than raw gyro.
  pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;
  roll = atan2(ay, az) * 180.0f / PI;
}

void setServoAngle(uint8_t channel, int angle) {
  angle = constrain(angle, 0, 180);

  const uint16_t pulseUs = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
  const uint16_t pulseCount = (uint32_t)pulseUs * SERVO_FREQUENCY * 4096.0f / 1000000.0f;
  pwm.setPWM(channel, 0, pulseCount);
}

int gestureToServoAngle(float gesture, bool reverseDirection) {
  gesture = applyDeadZone(gesture);
  gesture = clampFloat(gesture, -MAX_GESTURE_ANGLE, MAX_GESTURE_ANGLE);

  if (reverseDirection) gesture = -gesture;

  // A neutral hand position gives 90 degrees. Full negative/positive tilt
  // gives 0/180 degrees respectively.
  return (int)(90.0f + (gesture / MAX_GESTURE_ANGLE) * 90.0f + 0.5f);
}

int twistGestureToServoAngle(float gesture) {
  gesture = applyDeadZone(gesture);
  if (REVERSE_TWIST) gesture = -gesture;

  // The twist mechanism rests at 0 degrees. Tilting in the commanded
  // direction progressively turns it right, reaching 180 degrees at the
  // maximum gesture angle. Tilting the other way keeps it at rest.
  gesture = clampFloat(gesture, 0.0f, MAX_GESTURE_ANGLE);
  return (int)((gesture / MAX_GESTURE_ANGLE) * 180.0f + 0.5f);
}

void calibrateNeutralPosition() {
  const int sampleCount = 100;
  float pitchTotal = 0.0f;
  float rollTotal = 0.0f;
  int samplesRead = 0;

  Serial.println("Hold the controller in its comfortable REST position...");

  while (samplesRead < sampleCount) {
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(ax, ay, az);

      float pitch;
      float roll;
      calculateTilt(pitch, roll);
      pitchTotal += pitch;
      rollTotal += roll;
      samplesRead++;
    }
    delay(10);
  }

  neutralPitch = pitchTotal / sampleCount;
  neutralRoll = rollTotal / sampleCount;
  filteredPitch = neutralPitch;
  filteredRoll = neutralRoll;

  Serial.println("Calibration complete.");
}

void setup() {
  Serial.begin(115200);

  unsigned long startTime = millis();
  while (!Serial && millis() - startTime < 3000) {
  }

  Wire.begin();

  if (!IMU.begin()) {
    Serial.println("ERROR: IMU initialization failed!");
    while (1) delay(1000);
  }

  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQUENCY);
  delay(10);

  setServoAngle(UP_DOWN_CHANNEL, UP_DOWN_REST_ANGLE);
  setServoAngle(TWIST_CHANNEL, TWIST_REST_ANGLE);
  delay(1000);

  calibrateNeutralPosition();
  Serial.println("Gesture control started.");
}

void loop() {
  if (!IMU.accelerationAvailable()) return;

  IMU.readAcceleration(ax, ay, az);

  float pitch;
  float roll;
  calculateTilt(pitch, roll);

  // Low-pass filtering reduces servo jitter.
  const float filterStrength = 0.15f;
  filteredPitch += filterStrength * (pitch - filteredPitch);
  filteredRoll += filterStrength * (roll - filteredRoll);

  float upDownGesture = filteredPitch - neutralPitch;
  float twistGesture = filteredRoll - neutralRoll;

  int upDownAngle = gestureToServoAngle(upDownGesture, REVERSE_UP_DOWN);
  int twistAngle = twistGestureToServoAngle(twistGesture);

  upDownAngle = constrain(upDownAngle, UP_DOWN_UP_ANGLE, UP_DOWN_DOWN_ANGLE);
  twistAngle = constrain(twistAngle, TWIST_LEFT_ANGLE, TWIST_RIGHT_ANGLE);

  setServoAngle(UP_DOWN_CHANNEL, upDownAngle);
  setServoAngle(TWIST_CHANNEL, twistAngle);

  // Slow serial output enough to remain readable without slowing control.
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime >= 200) {
    lastPrintTime = millis();
    Serial.print("Up/down: ");
    Serial.print(upDownAngle);
    Serial.print(" deg, twist: ");
    Serial.print(twistAngle);
    Serial.println(" deg");
  }

  delay(10);
}
