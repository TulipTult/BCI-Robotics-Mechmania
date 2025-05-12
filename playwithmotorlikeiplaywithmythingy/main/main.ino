// ESP32 Connections:
// Ultrasonic
// - VCC --> 3.3V
// - GND --> GND
// - TRIG --> GPIO 18
// - ECHO --> GPIO 19
//
// Motor Driver 1
// - RPWM --> GPIO 25
// - LPWM --> GPIO 26
// - R_EN --> GPIO 32
// - L_EN --> GPIO 33
// - VCC --> 5V
// - GND --> GND
// Motor Driver 2
// - RPWM --> GPIO 27
// - LPWM --> GPIO 14
// - R_EN --> GPIO 12
// - L_EN --> GPIO 13
// - VCC --> 5V
// - GND --> GND

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
#define MOTOR_SPEED 200  // PWM value (0-255)

// Motor control channels
#define MOTOR1_RPWM_CHANNEL 0
#define MOTOR1_LPWM_CHANNEL 1
#define MOTOR2_RPWM_CHANNEL 2
#define MOTOR2_LPWM_CHANNEL 3

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
    
    // Configure PWM for motor control using correct API
    ledcAttachChannel(MOTOR1_RPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR1_RPWM_CHANNEL);
    ledcAttachChannel(MOTOR1_LPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR1_LPWM_CHANNEL);
    ledcAttachChannel(MOTOR2_RPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR2_RPWM_CHANNEL);
    ledcAttachChannel(MOTOR2_LPWM, MOTOR_FREQ, MOTOR_RESOLUTION, MOTOR2_LPWM_CHANNEL);
    
    // Initially stop all motors
    stopMotors();
}

void stopMotors() {
    ledcWriteChannel(MOTOR1_RPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR1_LPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR2_RPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR2_LPWM_CHANNEL, 0);
}
  
void turnLeft() {
    // Left motor backward, right motor forward
    ledcWriteChannel(MOTOR1_RPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
    ledcWriteChannel(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
    ledcWriteChannel(MOTOR2_LPWM_CHANNEL, 0);
}
  
void turnRight() {
    // Left motor forward, right motor backward
    ledcWriteChannel(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
    ledcWriteChannel(MOTOR1_LPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR2_RPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}
  
void moveForward() {
    // Both motors forward
    ledcWriteChannel(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
    delay(200);
    ledcWriteChannel(MOTOR1_LPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
    ledcWriteChannel(MOTOR2_LPWM_CHANNEL, 0);
}
  
void moveBackward() {
    // Both motors backward
    ledcWriteChannel(MOTOR1_RPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
    ledcWriteChannel(MOTOR2_RPWM_CHANNEL, 0);
    ledcWriteChannel(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}

void setup() {
    setupMotors();
    pinMode(26, OUTPUT);
    Serial.begin(115200); // Initialize serial communication
}

void loop() {
    Serial.println("Motor is spinning forward...");
    digitalWrite(26, HIGH);
    delay(1000); // Spin duration
    digitalWrite(26, LOW);
    Serial.println("Motor stopped.");
    delay(5000); // Wait for 5 seconds before spinning again
}