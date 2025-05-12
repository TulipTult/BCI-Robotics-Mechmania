#include <BluetoothSerial.h>

// Motor control pins definitions
#define MOTOR1_RPWM 25
#define MOTOR1_LPWM 26
#define MOTOR1_R_EN 32
#define MOTOR1_L_EN 33
#define MOTOR2_RPWM 27
#define MOTOR2_LPWM 14
#define MOTOR2_R_EN 12
#define MOTOR2_L_EN 13

// PWM properties
#define MOTOR_FREQ 5000
#define MOTOR_RESOLUTION 8
#define MOTOR_SPEED 255  // Full speed for sumo bot

// Motor control channels
#define MOTOR1_RPWM_CHANNEL 0
#define MOTOR1_LPWM_CHANNEL 1
#define MOTOR2_RPWM_CHANNEL 2
#define MOTOR2_LPWM_CHANNEL 3

BluetoothSerial SerialBT;
char command;
char lastCommand = 'S'; // Store the last command, 'S' for stop initially

void stopMotors() {  // Renamed from stop() to avoid conflicts
  ledcWriteChannel(MOTOR1_RPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR1_LPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR2_RPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR2_LPWM_CHANNEL, 0);
}

void setupMotors() {
  // Enable motor driver control pins
  pinMode(MOTOR1_R_EN, OUTPUT);
  pinMode(MOTOR1_L_EN, OUTPUT);
  pinMode(MOTOR2_R_EN, OUTPUT);
  pinMode(MOTOR2_L_EN, OUTPUT);

  digitalWrite(MOTOR1_R_EN, HIGH);
  digitalWrite(MOTOR1_L_EN, HIGH);
  digitalWrite(MOTOR2_R_EN, HIGH);
  digitalWrite(MOTOR2_L_EN, HIGH);
  // Configure PWM using the correct API
  ledcAttachChannel(MOTOR1_RPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR1_RPWM_CHANNEL);
  ledcAttachChannel(MOTOR1_LPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR1_LPWM_CHANNEL);
  ledcAttachChannel(MOTOR2_RPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR2_RPWM_CHANNEL);
  ledcAttachChannel(MOTOR2_LPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR2_LPWM_CHANNEL);

  stopMotors();
}

void forward() {
  ledcWriteChannel(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWriteChannel(MOTOR1_LPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWriteChannel(MOTOR2_LPWM_CHANNEL, 0);
}

void backward() {
  ledcWriteChannel(MOTOR1_RPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
  ledcWriteChannel(MOTOR2_RPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}

void left() {
  ledcWriteChannel(MOTOR1_RPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
  ledcWriteChannel(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWriteChannel(MOTOR2_LPWM_CHANNEL, 0);
}

void right() {
  ledcWriteChannel(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWriteChannel(MOTOR1_LPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR2_RPWM_CHANNEL, 0);
  ledcWriteChannel(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("SumoBot");  // Bluetooth device name
  setupMotors();
}

void loop() {
  if (SerialBT.available()) {
    command = SerialBT.read();
    Serial.print("Received command: ");
    Serial.println(command);
    
    // Only execute if the command has changed
    if (command != lastCommand) {
      // First stop motors only if changing direction
      if (command == 'F' || command == 'B' || command == 'L' || command == 'R' || command == 'S') {
        stopMotors();
        
        // Then execute the new command
        switch (command) {
          case 'F': forward(); break;
          case 'B': backward(); break;
          case 'L': left(); break;
          case 'R': right(); break;
          case 'S': /* already stopped */ break;
        }
        
        // Update the last command
        lastCommand = command;
      }
    }
  }
}
