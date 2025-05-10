// V1.3a - Sumo Bot with ESP32 Motor Drivers
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

// Ultrasonic sensor pins
#define TRIG_PIN 18
#define ECHO_PIN 19

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
// Function to set up motors

void stopMotors() {
    ledcWrite(MOTOR1_RPWM_CHANNEL, 0);
    ledcWrite(MOTOR1_LPWM_CHANNEL, 0);
    ledcWrite(MOTOR2_RPWM_CHANNEL, 0);
    ledcWrite(MOTOR2_LPWM_CHANNEL, 0);
  }
  
void setupMotors() {
    // Configure motor control pins
    pinMode(MOTOR1_R_EN, OUTPUT);
    pinMode(MOTOR1_L_EN, OUTPUT);
    pinMode(MOTOR2_R_EN, OUTPUT);
    pinMode(MOTOR2_L_EN, OUTPUT);
    
    // Enable all motor drivers
    digitalWrite(MOTOR1_R_EN, HIGH);
    digitalWrite(MOTOR1_L_EN, HIGH);
    digitalWrite(MOTOR2_R_EN, HIGH);
    digitalWrite(MOTOR2_L_EN, HIGH);
    
    // Configure PWM for motor control
    ledcAttach(MOTOR1_RPWM_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttach(MOTOR1_LPWM_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttach(MOTOR2_RPWM_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttach(MOTOR2_LPWM_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    
    // Initially stop all motors
    stopMotors();
  }
void setup() {
  Serial.begin(115200);
  SerialBT.begin("SumoBot"); // Bluetooth device name
  
  // Configure motor control pins
  setupMotors();
  
  // Configure ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  stop();
}

void loop() {
  // Check distance from ultrasonic sensor
  long duration, distance;
  
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  
  // Auto charge when object detected within 20cm
  if (distance > 0 && distance <= 20) {
    Serial.println("Object detected! Charging...");
    forward();
    delay(2000);
    stop();
  }
  
  // Bluetooth control
  if (SerialBT.available()) {
    command = SerialBT.read();
    stop(); // Stop before new command
    
    switch (command) {
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'L': left(); break;
      case 'R': right(); break;
      case 'S': stop(); break;
    }
  }
}

void forward() {
  ledcWrite(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR1_LPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_LPWM_CHANNEL, 0);
}

void backward() {
  ledcWrite(MOTOR1_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}

void left() {
  ledcWrite(MOTOR1_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_LPWM_CHANNEL, 0);
}

void right() {
  ledcWrite(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR1_LPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}

void stop() {
  ledcWrite(MOTOR1_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR1_LPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_LPWM_CHANNEL, 0);
}