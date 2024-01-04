#include <ConfigApp.hpp>
#include <GUILib.hpp>
#include <power.hpp>

void prepairShutdown() {
    #ifndef M5STICKCPLUS
    digitalWrite(ADC_EN, LOW);
    delay(10);
    //rtc_gpio_init(GPIO_NUM_14);
    //rtc_gpio_set_direction(GPIO_NUM_14, RTC_GPIO_MODE_OUTPUT_ONLY);
    //rtc_gpio_set_level(GPIO_NUM_14, 1);
    delay(500);
    #endif
    gui.setPowerSave(); 
}

void powerCompleteShutdown(){
    Serial.println("-->[POWR] Complete shutdown..");
    #ifndef M5STICKCPLUS
    esp_bluedroid_disable();
    esp_bt_controller_disable();
    esp_wifi_stop();
    esp_deep_sleep_disable_rom_logging();
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_deep_sleep_start();
    #endif
    #ifdef M5STICKCPLUS
    M5.Axp.PowerOff();
    #endif
}

void powerDeepSleepButton(){
    prepairShutdown();
    #ifdef TTGO_TDISPLAY
    //Disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    #endif
    powerCompleteShutdown();
}

void powerDeepSleepTimer(int seconds) {
    Serial.println(F("-->[POWR] == shutdown =="));
    Serial.flush();
    prepairShutdown();
    #ifdef M5STICKCPLUS
    M5.Axp.DeepSleep(seconds*1000000);
    #endif
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
    #ifdef TTGO_TDISPLAY
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    #endif
    #ifndef M5STICKCPLUS
    powerCompleteShutdown(); 
    #endif
}

void powerLightSleepTimer(int seconds) {
    #ifndef M5STICKCPLUS
    esp_sleep_enable_timer_wakeup(seconds * 1000000);
    esp_light_sleep_start();
    #endif
    #ifdef M5STICKCPLUS
    M5.Axp.LightSleep(seconds*1000000);
    #endif
}

void powerEnableSensors() {
    if(cfg.devmode) Serial.println("-->[POWR] == enable sensors ==");
    // init all sensors (step-up to 5V with enable pin)
    pinMode(MAIN_HW_EN_PIN, OUTPUT);
    digitalWrite(MAIN_HW_EN_PIN, HIGH);  // step-up on
}

void powerDisableSensors() {
    if(cfg.devmode) Serial.println("-->[POWR] == disable sensors ==");
    digitalWrite(MAIN_HW_EN_PIN, LOW);  // step-up off
}

void powerTempSensorInit() {
#ifdef CONFIG_IDF_TARGET_ESP32S3
  temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
  temp_sensor.dac_offset = TSENS_DAC_L2;  // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
  temp_sensor_set_config(temp_sensor);
  temp_sensor_start();
#endif
}

void powerInit() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // Disable Brownout Detector
  // set cpu speed low to save battery
  setCpuFrequencyMhz(240);
  Serial.printf("-->[POWR] CPU Speed:%i MHz\r\n", getCpuFrequencyMhz());
  powerEnableSensors();
  powerTempSensorInit();
}

float powerESP32TempRead(){
  float result = 0;
#ifdef CONFIG_IDF_TARGET_ESP32S3
  temp_sensor_read_celsius(&result);
#endif
  return result;
}

void powerLoop() {
  static uint32_t powerTimeStamp = 0;             // timestamp for check low power
  if ((millis() - powerTimeStamp > 30 * 1000)) {  // check it every 5 seconds
    powerTimeStamp = millis();
    float vbat = battery.getVoltage();
    if (vbat > 3.0 && vbat < BATTERY_MIN_V) {
      Serial.println("-->[POWR] Goto DeepSleep (VBat too low)");
      if (cfg.solarmode)
        powerDeepSleepTimer(cfg.deepSleep);
      else
        powerCompleteShutdown();
    }
  #ifdef CONFIG_IDF_TARGET_ESP32S3
    if (cfg.devmode) Serial.printf("-->[POWR] CPU Temperature\t: %02.1f°C\r\n", powerESP32TempRead());
  #endif
    log_i("[HEAP] Min:%d Max:%d\t: %d\r\n", ESP.getMinFreeHeap(), ESP.getMaxAllocHeap(), ESP.getFreeHeap());
  }
}
