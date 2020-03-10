#include "sketchParam.h"


void setup() {
  Serial.begin(115200); 
  Serial.println("\n  Wake up!\n");
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  Serial.println(String(post_url));
  
  //Connect to the internet
  if (init_wifi()) { // Connected to WiFi
    internet_connected = true;
    Serial.println("Internet connected");
  }

  #ifdef LAMP_PIN
    ledcSetup(lampChannel, pwmfreq, pwmresolution); // configure LED PWM channel
    ledcWrite(lampChannel, lampVal);                // set initial value
    ledcAttachPin(LAMP_PIN, lampChannel);           // attach the GPIO pin to the channel 
    // Calculate the PWM scaling R factor: 
    lampR = (pwmIntervals * log10(2))/(log10(pow(2,pwmresolution)));
  #endif
  
  //blinkLed();
    //ON
  lampVal=20;
  int brightness = pow (2, (lampVal / lampR)) - 1;
  ledcWrite(lampChannel, brightness);

  //TakePicture
  take_send_request(); //tak picture and send to server
 
  //OFF
  lampVal=-1;
  brightness = pow (2, (lampVal / lampR)) - 1;
  ledcWrite(lampChannel, brightness);
  
  Serial.println("Going to sleep for " + String(TIME_TO_SLEEP)+ " sec");
  delay(500);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //se timeer to sleep
  Serial.flush();
  esp_deep_sleep_start(); //sending esp to deepsleep
  //M5stack supply only 32sec sleep start due to ip5306 buck IC (0.8mA)  
}

bool init_wifi()
{
  int connAttempts = 0;
  Serial.println("\r\nConnecting to: " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
    if (connAttempts > 10) return false;
    connAttempts++;
  }
  Serial.println("\r\nConnecting to: " + String(ssid));
  Serial.print("The IP is:");
  Serial.println(WiFi.localIP());
  
  return true;
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
  switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
      Serial.println("HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      Serial.println("HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      Serial.println("HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      Serial.println();
      Serial.printf("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
      break;
    case HTTP_EVENT_ON_DATA:
      Serial.println();
      Serial.printf("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      if (!esp_http_client_is_chunked_response(evt->client)) {
        // Write out data
        printf("%.*s", evt->data_len, (char*)evt->data);
      }
      break;
    case HTTP_EVENT_ON_FINISH:
      Serial.println("");
      Serial.println("HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      Serial.println("HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_OK;
}

static esp_err_t take_send_request()
{
  Serial.println("Sending boot...");

  esp_http_client_handle_t http_client;
  
  esp_http_client_config_t config_client = {0};
  config_client.url = post_url;
  config_client.event_handler = _http_event_handler;
  config_client.method = HTTP_METHOD_POST;

  http_client = esp_http_client_init(&config_client);

  esp_http_client_set_post_field(http_client, (const char *)bootCount, bootCount); 

  esp_http_client_set_header(http_client, "Content-Type", "text/plain"); 

  esp_err_t err = esp_http_client_perform(http_client);
  if (err == ESP_OK) {
    Serial.print("esp_http_client_get_status_code: ");
    Serial.println(esp_http_client_get_status_code(http_client));
  }
  esp_http_client_cleanup(http_client);

}

void loop() {
}
