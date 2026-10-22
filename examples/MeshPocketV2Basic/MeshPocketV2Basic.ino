#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <CW221X.h>
#include <Wire.h>

#if !defined(HELTEC_MESH_POCKET_V2)
#error "MeshPocketV2Basic is intended for Mesh Pocket V2 (HT-mesh-pocket-v2)."
#endif

#define SERIAL_BAUDRATE 115200
#define DISPLAY_SPI_FREQUENCY 4000000
// Match the effective orientation in the Meshtastic Mesh Pocket V2 display setup.
#define DISPLAY_ROTATION 1
#define DISPLAY_WIDTH TFT_WIDTH
#define DISPLAY_HEIGHT TFT_HEIGHT
#define DISPLAY_IPS true

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, &SPI1, true);
Arduino_GFX *gfx = new Arduino_NV3001B(bus, TFT_RST, DISPLAY_ROTATION,
                                       DISPLAY_IPS, DISPLAY_WIDTH,
                                       DISPLAY_HEIGHT, 0, 0, 0, 0);

CW221X gauge;
CW221X::Status status = {};

bool displayReady = false;
uint32_t lastReadMs = 0;
char renderedLines[9][36] = {};
uint16_t renderedColors[9] = {};

void drawLine(uint8_t row, const char *text, uint16_t color) {
  if (!displayReady || row >= 9) {
    return;
  }

  if ((renderedColors[row] == color) &&
      (strncmp(renderedLines[row], text, sizeof(renderedLines[row])) == 0)) {
    return;
  }

  gfx->fillRect(0, row * 14, gfx->width(), 14, RGB565_BLACK);
  gfx->setCursor(2, row * 14 + 2);
  gfx->setTextColor(color);
  gfx->println(text);

  strncpy(renderedLines[row], text, sizeof(renderedLines[row]) - 1);
  renderedLines[row][sizeof(renderedLines[row]) - 1] = '\0';
  renderedColors[row] = color;
}

void formatTemperature(char *buffer, size_t len, int16_t valueX10) {
  int16_t whole = valueX10 / 10;
  int16_t frac = valueX10 % 10;
  if (frac < 0) {
    frac = -frac;
  }

  if (valueX10 < 0 && valueX10 > -10) {
    snprintf(buffer, len, "-0.%d", frac);
  } else {
    snprintf(buffer, len, "%d.%d", whole, frac);
  }
}

void renderStatus() {
  if (!displayReady) {
    return;
  }

  char line[36];
  char tempText[10];
  gfx->setTextSize(1);
  gfx->setTextWrap(false);

  drawLine(0, "CW221X Mesh Pocket V2", RGB565_CYAN);

  if (!status.ready) {
    snprintf(line, sizeof(line), "State: ERROR %d", (int)status.error);
    drawLine(1, line, RGB565_RED);
    drawLine(2, "SOC: --", RGB565_LIGHTGRAY);
    drawLine(3, "VBAT: --", RGB565_LIGHTGRAY);
    drawLine(4, "TEMP: --", RGB565_LIGHTGRAY);
    drawLine(5, "PROFILE: --", RGB565_LIGHTGRAY);
    drawLine(6, "Check CW221X I2C", RGB565_YELLOW);
    drawLine(7, "", RGB565_LIGHTGRAY);
    drawLine(8, "", RGB565_LIGHTGRAY);
    return;
  }

  drawLine(1, "State: READY", RGB565_GREEN);
  snprintf(line, sizeof(line), "SOC: %u%%", status.socPercent);
  drawLine(2, line, RGB565_WHITE);
  snprintf(line, sizeof(line), "VBAT: %umV", status.voltageMv);
  drawLine(3, line, RGB565_WHITE);
  formatTemperature(tempText, sizeof(tempText), status.temperatureX10C);
  snprintf(line, sizeof(line), "TEMP: %sC", tempText);
  drawLine(4, line, RGB565_WHITE);
  drawLine(5, status.profile.matches ? "PROFILE: OK" : "PROFILE: MISMATCH",
           status.profile.matches ? RGB565_GREEN : RGB565_RED);
  drawLine(6, "", RGB565_LIGHTGRAY);
  drawLine(7, "", RGB565_LIGHTGRAY);
  drawLine(8, "", RGB565_LIGHTGRAY);
}

void printStatus() {
  if (!status.ready) {
    Serial.print("CW221X read failed, error=");
    Serial.println((int)status.error);
    return;
  }
  Serial.print("time=");
  Serial.print(millis() / 1000);
  Serial.print(" soc=");
  Serial.print(status.socPercent);
  Serial.print("% voltage=");
  Serial.print(status.voltageMv);
  Serial.print("mV temp=");
  Serial.print(status.temperatureX10C / 10.0f, 1);
  Serial.print("C profile=");
  Serial.println(status.profile.matches ? "OK" : "MISMATCH");
}

void readGauge() {
  if (!gauge.readStatus(status)) {
    status.ready = false;
    status.error = gauge.lastError();
  }
  printStatus();
}

void startDisplay() {
  pinMode(TFT_EN, OUTPUT);
  digitalWrite(TFT_EN, TFT_EN_ON);
  delay(100);

  pinMode(PIN_IO_CSA, OUTPUT);
  digitalWrite(PIN_IO_CSA, HIGH);
  delay(100);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  delay(100);

  if (!gfx->begin(DISPLAY_SPI_FREQUENCY)) {
    Serial.println("gfx->begin() failed!");
  }

  gfx->setRotation(DISPLAY_ROTATION);
  gfx->fillScreen(RGB565_BLACK);
  gfx->setTextSize(1);
  gfx->setTextWrap(false);
  displayReady = true;
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  uint32_t serialStart = millis();
  while (!Serial && (millis() - serialStart < 2000)) {
    delay(10);
  }

  Serial.println();
  Serial.println("CW221X Mesh Pocket V2 Basic");

  pinMode(PIN_IO_CSA, OUTPUT);
  digitalWrite(PIN_IO_CSA, HIGH);

  startDisplay();

  if (!gauge.begin(Wire)) {
    status.ready = false;
    status.error = gauge.lastError();
    Serial.print("CW221X begin failed, error=");
    Serial.println((int)status.error);
    renderStatus();
    return;
  }

  readGauge();
  renderStatus();
}

void loop() {
  uint32_t now = millis();
  if (now - lastReadMs >= 3000) {
    lastReadMs = now;
    readGauge();
    renderStatus();
  }
}
