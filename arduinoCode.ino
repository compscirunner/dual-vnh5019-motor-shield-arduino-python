#include <DualVNH5019MotorShield.h>

DualVNH5019MotorShield md;
unsigned long lastCommandTime = 0;
const unsigned long TIMEOUT = 1000; // Timeout in milliseconds
const int MAX_SPEED = 400; // Maximum motor speed
const int MIN_SPEED = -400; // Minimum motor speed
const int SPEED_STEP = 20; // Speed change step to smooth transitions
const int BRAKE_LEVEL = 400; // Maximum brake level

int currentM1Speed = 0;
int currentM2Speed = 0;

void setup() {
  md.init();
  Serial.begin(9600);
}

void stopMotorsGradually() {
  // Gradually decrease speed and apply brakes for smooth stops
  while (currentM1Speed != 0 || currentM2Speed != 0) {
    if (currentM1Speed > 0) {
      currentM1Speed = max(0, currentM1Speed - SPEED_STEP);
    } else if (currentM1Speed < 0) {
      currentM1Speed = min(0, currentM1Speed + SPEED_STEP);
    }

    if (currentM2Speed > 0) {
      currentM2Speed = max(0, currentM2Speed - SPEED_STEP);
    } else if (currentM2Speed < 0) {
      currentM2Speed = min(0, currentM2Speed + SPEED_STEP);
    }

    md.setSpeeds(currentM1Speed, currentM2Speed);
    md.setBrakes(abs(currentM1Speed), abs(currentM2Speed)); // Apply proportional braking
    delay(50); // Short delay to allow motors to slow down

    // Read and print motor currents
    unsigned int m1Current = md.getM1CurrentMilliamps();
    unsigned int m2Current = md.getM2CurrentMilliamps();
    Serial.print("M1 current: ");
    Serial.print(m1Current);
    Serial.print(" mA, M2 current: ");
    Serial.println(m2Current);
    
    // Check for motor faults and print messages
    if (md.getM1Fault()) {
      Serial.println("Motor 1 fault detected!");
    }
    if (md.getM2Fault()) {
      Serial.println("Motor 2 fault detected!");
    }
  }
  md.setBrakes(BRAKE_LEVEL, BRAKE_LEVEL); // Full brake after stop
}

void processCommand() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'L' || command == 'R') {
      int speed = Serial.parseInt();
      speed = constrain(speed, MIN_SPEED, MAX_SPEED); // Constrain speed

      if (command == 'L') {
        currentM1Speed = speed;
      } else {
        currentM2Speed = speed;
      }
      md.setSpeeds(currentM1Speed, currentM2Speed); // Set speeds simultaneously
      Serial.print("Motor speeds set: ");
      Serial.println(speed);
      lastCommandTime = millis();
    } else {
      Serial.println("Invalid command.");
    }
  }
}

void checkTimeout() {
  if (millis() - lastCommandTime > TIMEOUT) {
    stopMotorsGradually();
    Serial.println("Motors stopped due to timeout.");
  }
}

void loop() {
  processCommand();
  checkTimeout();
}
