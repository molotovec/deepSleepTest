#include "esp_http_client.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "Arduino.h"

#include"soc/rtc_cntl_reg.h"
#include"soc/rtc.h"
#include"driver/rtc_io.h"

//Custom ####################################################################
#define CAMERA_MODEL_M5STACK_ESP32CAM_NO_PSRAM //Camera Module definition
#include "camera_pins.h"
#include "myWiFiconfig.h"
//###########################################################################


bool internet_connected = false;
long current_millis;
long last_capture_millis = 0;

RTC_DATA_ATTR int bootCount=0;

//WebServer URL
const char *post_url = "https://managemeters.000webhostapp.com/wakeUps.php?"; // POSTED boot amount

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60          /* Time ESP32 will go to sleep (in seconds) - 86400s=1day */


// Status and illumination LED's
#ifdef LAMP_PIN 
  int lampVal = 0; // Current Lamp value, range 0-100, Start off
#else 
  int lampVal = -1; // disable Lamp
#endif         
int lampChannel = 7;     // a free PWM channel (some channels used by camera)
const int pwmfreq = 50000;     // 50K pwm frequency
const int pwmresolution = 9;   // duty cycle bit range
// https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
const int pwmIntervals = 100;  // The number of Steps between the output being on and off
float lampR;                   // The R value in the PWM graph equation (calculated in setup)
//##############################################################################################
