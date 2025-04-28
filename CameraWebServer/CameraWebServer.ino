#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// CameraWebServer.ino

// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

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

// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE  // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_CAMS3_UNIT  // Has PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "";
const char *password = "";

void startCameraServer();
void setupLedFlash(int pin);

// Function to set up motors
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

void stopMotors() {
  ledcWrite(MOTOR1_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR1_LPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_LPWM_CHANNEL, 0);
}

void turnLeft() {
  // Left motor backward, right motor forward
  ledcWrite(MOTOR1_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_LPWM_CHANNEL, 0);
}

void turnRight() {
  // Left motor forward, right motor backward
  ledcWrite(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR1_LPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}

void moveForward() {
  // Both motors forward
  ledcWrite(MOTOR1_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR1_LPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_RPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_LPWM_CHANNEL, 0);
}

void moveBackward() {
  // Both motors backward
  ledcWrite(MOTOR1_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR1_LPWM_CHANNEL, MOTOR_SPEED);
  ledcWrite(MOTOR2_RPWM_CHANNEL, 0);
  ledcWrite(MOTOR2_LPWM_CHANNEL, MOTOR_SPEED);
}

// API endpoint handler to control the robot
static esp_err_t cmd_handler(httpd_req_t *req) {
  char buf[64];
  size_t buf_len = min(req->content_len, sizeof(buf) - 1);
  
  int ret = httpd_req_recv(req, buf, buf_len);
  if (ret <= 0) {
    return ESP_FAIL;
  }
  
  buf[buf_len] = '\0';
  
  // Process command
  if (strcmp(buf, "left") == 0) {
    turnLeft();
    Serial.print("Moving Left");
  } else if (strcmp(buf, "right") == 0) {
    turnRight();
    Serial.print("Moving Right");
  } else if (strcmp(buf, "forward") == 0) {
    moveForward();
    Serial.print("Moving Forward");
  } else if (strcmp(buf, "backward") == 0) {
    moveBackward();
    Serial.print("Moving Backwards");
  } else if (strcmp(buf, "stop") == 0) {
    stopMotors();
    Serial.print("Not Moving");
  }
  
  // Respond with success
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, "OK", 2);
}

// Register the control endpoint
void register_cmd_uri(httpd_handle_t server) {
  httpd_uri_t cmd_uri = {
    .uri = "/control",
    .method = HTTP_POST,
    .handler = cmd_handler,
    .user_ctx = NULL
  };
  httpd_register_uri_handler(server, &cmd_uri);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
  // Initialize motor control
  setupMotors();
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}
