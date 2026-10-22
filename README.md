# Hand-Gesture Controlled Robotic Arm

This branch contains the first version of the robotic-arm project: a hand-gesture controller built with an **Arduino Nano 33 IoT**, its onboard **LSM6DS3 inertial measurement unit (IMU)**, and a **PCA9685 PWM servo driver**.

Tilting the controller moves the robotic arm in real time. One hand axis controls the arm's up/down movement, while the other controls its left/right twist. The controller calibrates its neutral position at startup, filters sensor readings to reduce servo jitter, and includes a dead zone to prevent unintended movement.

> The improved ESP32 web-controlled version, including motion recording and repeated playback, is available on the [`webapp-control`](../../tree/webapp-control) branch.

## Features

- Natural hand-gesture control using the onboard IMU
- Up/down arm movement and left/right rotation
- Automatic neutral-position calibration at startup
- Low-pass filtering for smoother servo motion
- Adjustable gesture dead zone and movement sensitivity
- Optional direction reversal for either servo
- PCA9685-based PWM control for stable servo signals

## Hardware

- Arduino Nano 33 IoT
- PCA9685 16-channel PWM servo driver
- Two compatible servo motors
- Robotic-arm mechanism
- External regulated servo power supply
- Jumper wires

## Wiring

| Arduino Nano 33 IoT | PCA9685 |
|---|---|
| 3.3V | VCC |
| GND | GND |
| SDA | SDA |
| SCL | SCL |

Connect the up/down servo to PCA9685 channel **3** and the twist servo to channel **4**. Power the servos through the PCA9685 external power input, and connect the power-supply ground to the Arduino ground.

> Do not power multiple servos directly from the Arduino. Use an appropriately rated external supply and verify your servos' voltage requirements.

## Software requirements

Install the following libraries through the Arduino IDE Library Manager:

- `Arduino_LSM6DS3`
- `Adafruit PWM Servo Driver Library`

The sketch also uses the built-in `Wire` library.

## Getting started

1. Open [`GestureRoboticArm/GestureRoboticArm.ino`](GestureRoboticArm/GestureRoboticArm.ino) in the Arduino IDE.
2. Select **Arduino Nano 33 IoT** as the board.
3. Install the required libraries.
4. Connect the Arduino and upload the sketch.
5. Open Serial Monitor at **115200 baud**.
6. Hold the controller in a comfortable neutral position while it calibrates.
7. Tilt your hand to move the arm.

## Calibration and tuning

The main settings are near the top of the sketch:

- `REVERSE_UP_DOWN` and `REVERSE_TWIST` change movement direction.
- `MAX_GESTURE_ANGLE` sets the hand tilt required for full movement.
- `GESTURE_DEAD_ZONE` controls the neutral region.
- `SERVO_MIN_US` and `SERVO_MAX_US` define the servo pulse range.

Start with conservative pulse limits. If a servo buzzes, strains, or pushes against a mechanical stop, disconnect power and reduce its range before testing again.

## Project evolution

This gesture-controlled prototype established the arm's basic motion control. The next version expanded the arm to six servo channels, added Wi-Fi control through a mobile-friendly web app, and introduced a teach-and-repeat workflow for moving objects between locations. See the [`webapp-control`](../../tree/webapp-control) branch for that implementation.

## Repository structure

```text
GestureRoboticArm/
└── GestureRoboticArm.ino   # Complete gesture-control firmware
```

## License

No license has been selected yet. Unless a license file is added, all rights are reserved by the repository owner.
