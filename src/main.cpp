#include "config.h"
#include "interface.h"
#include "player_audio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void uiTask(void* parameter) {
  (void)parameter;
  for (;;) {
    ui.update();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  audioPlayer.begin();
  ui.begin();

  xTaskCreatePinnedToCore(
    uiTask,
    "uiTask",
    4096,
    nullptr,
    1,
    nullptr,
    1
  );
}

void loop() {
  audioUpdate();
  vTaskDelay(pdMS_TO_TICKS(1));
}
