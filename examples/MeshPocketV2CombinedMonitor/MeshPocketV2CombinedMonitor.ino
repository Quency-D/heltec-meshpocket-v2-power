#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <CW221X.h>
#include <HLT331Serial.h>
#include <Wire.h>

#if !defined(HELTEC_MESH_POCKET_V2)
#error "MeshPocketV2CombinedMonitor is intended for Mesh Pocket V2 (HT-mesh-pocket-v2)."
#endif

#define SERIAL_BAUDRATE 115200
#define DISPLAY_SPI_FREQUENCY 4000000
// Match the effective orientation in the Meshtastic Mesh Pocket V2 display setup.
#define DISPLAY_ROTATION 1
#define DISPLAY_WIDTH TFT_WIDTH
#define DISPLAY_HEIGHT TFT_HEIGHT
#define DISPLAY_IPS true
#define HLT331_STALE_MS 3000
#define HLT331_IO1_READ_WINDOW_MS 120

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, &SPI1, true);
Arduino_GFX *gfx = new Arduino_NV3001B(bus, TFT_RST, DISPLAY_ROTATION,
                                       DISPLAY_IPS, DISPLAY_WIDTH,
                                       DISPLAY_HEIGHT, 0, 0, 0, 0);

CW221X gauge;
HLT331Serial hlt331;
CW221X::Status cwStatus = {};
HLT331Serial::Status hltStatus = {};

bool displayReady = false;
uint32_t lastGaugeReadMs = 0;
uint32_t lastRenderMs = 0;
char renderedLines[14][38] = {};
uint16_t renderedColors[14] = {};

void onHlt331Io1Pulse() {
  hlt331.markIo1PulseFromIsr();
}

void drawLine(uint8_t row, const char *text, uint16_t color) {
  if (!displayReady || row >= 14) {
    return;
  }

  if ((renderedColors[row] == color) &&
      (strncmp(renderedLines[row], text, sizeof(renderedLines[row])) == 0)) {
    return;
  }

  gfx->fillRect(0, row * 9, gfx->width(), 9, RGB565_BLACK);
  gfx->setCursor(2, row * 9);
  gfx->setTextColor(color);
  gfx->println(text);

  strncpy(renderedLines[row], text, sizeof(renderedLines[row]) - 1);
  renderedLines[row][sizeof(renderedLines[row]) - 1] = '\0';
  renderedColors[row] = color;
}

void formatTemperatureX10(char *buffer, size_t len, int16_t valueX10) {
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

void readGauge() {
  if (!gauge.readStatus(cwStatus)) {
    cwStatus.ready = false;
    cwStatus.error = gauge.lastError();
  }
}

void readHlt331() {
  hlt331.poll();
  hlt331.readStatus(hltStatus);
}

void drainHlt331ForWindow(uint32_t windowMs) {
  uint32_t startMs = millis();
  do {
    hlt331.poll();
    delay(1);
  } while ((millis() - startMs) < windowMs);
  hlt331.readStatus(hltStatus);
}

void printStatus() {
  Serial.print("CW ready=");
  Serial.print(cwStatus.ready ? "Y" : "N");
  Serial.print(" error=");
  Serial.print((int)cwStatus.error);
  Serial.print(" soc=");
  Serial.print(cwStatus.socPercent);
  Serial.print("% vb=");
  Serial.print(cwStatus.voltageMv);
  Serial.print("mV temp=");
  Serial.print(cwStatus.temperatureX10C / 10.0f, 1);
  Serial.print("C profile=");
  Serial.print(cwStatus.profile.matches ? "OK" : "MISMATCH");
  Serial.print(" HLT valid=");
  Serial.print(hltStatus.valid ? "Y" : "N");
  Serial.print(" soc=");
  Serial.print(hltStatus.socPercent);
  Serial.print(" bus=");
  Serial.print(hltStatus.busMv);
  Serial.print("mV/");
  Serial.print(hltStatus.busMa);
  Serial.print("mA ok=");
  Serial.print(hltStatus.goodFrames);
  Serial.print(" crc=");
  Serial.print(hltStatus.badCrcFrames);
  Serial.print(" ntc1=");
  Serial.print((int)hltStatus.ntc1TemperatureC);
  Serial.print("C ntc2=");
  Serial.print((int)hltStatus.ntc2TemperatureC);
  Serial.print("C");
  Serial.print(" irq=");
  Serial.print(hltStatus.io1PulseCount);
  Serial.print(" io2=");
  Serial.println(hltStatus.io2Working ? "WORK" : "IDLE");
}

void renderStatus() {
  if (!displayReady) {
    return;
  }

  char line[38];
  char tempText[10];

  drawLine(0, "CW221X + HLT331 Monitor", RGB565_CYAN);

  if (!cwStatus.ready) {
    snprintf(line, sizeof(line), "CW:ERR %d", (int)cwStatus.error);
    drawLine(1, line, RGB565_RED);
    drawLine(2, "CW:SOC --", RGB565_LIGHTGRAY);
    drawLine(3, "CW:VB --", RGB565_LIGHTGRAY);
    drawLine(4, "CW:T --", RGB565_LIGHTGRAY);
    drawLine(5, "CW:PROFILE --", RGB565_LIGHTGRAY);
  } else {
    drawLine(1, "CW:READY", RGB565_GREEN);
    snprintf(line, sizeof(line), "CW:SOC %u%%", cwStatus.socPercent);
    drawLine(2, line, RGB565_WHITE);
    snprintf(line, sizeof(line), "CW:VB %umV", cwStatus.voltageMv);
    drawLine(3, line, RGB565_WHITE);
    formatTemperatureX10(tempText, sizeof(tempText), cwStatus.temperatureX10C);
    snprintf(line, sizeof(line), "CW:T %sC", tempText);
    drawLine(4, line, RGB565_WHITE);
    drawLine(5, cwStatus.profile.matches ? "CW:PROFILE OK" : "CW:PROFILE MISMATCH",
             cwStatus.profile.matches ? RGB565_GREEN : RGB565_RED);
  }

  readHlt331();
  if (!hltStatus.valid) {
    snprintf(line, sizeof(line), "HLT:WAIT err %d", (int)hlt331.lastError());
    drawLine(6, line, RGB565_YELLOW);
    snprintf(line, sizeof(line), "UART RX%d TX%d 115200", PIN_9006_RX,
             PIN_9006_TX);
    drawLine(7, line, RGB565_LIGHTGRAY);
    snprintf(line, sizeof(line), "OK:%lu CRC:%lu LEN:%lu",
             (unsigned long)hlt331.goodFrameCount(),
             (unsigned long)hlt331.badCrcCount(),
             (unsigned long)hlt331.badLengthCount());
    drawLine(8, line, RGB565_LIGHTGRAY);
    drawLine(9, "HLT:BUS --", RGB565_LIGHTGRAY);
    drawLine(10, "HLT:WLC --", RGB565_LIGHTGRAY);
    drawLine(11, "HLT:PORT --", RGB565_LIGHTGRAY);
    drawLine(12, "HLT:T NTC1 -- NTC2 --", RGB565_LIGHTGRAY);
    snprintf(line, sizeof(line), "IRQ:%lu IO2:%s",
             (unsigned long)hltStatus.io1PulseCount,
             hltStatus.io2Working ? "WORK" : "IDLE");
    drawLine(13, line, hltStatus.io2Working ? RGB565_GREEN : RGB565_LIGHTGRAY);
    return;
  }

  bool stale = hltStatus.ageMs > HLT331_STALE_MS;

  snprintf(line, sizeof(line), "HLT:SOC %u%% CHG %s", hltStatus.socPercent,
           hltStatus.charging ? "Y" : "N");
  drawLine(6, line, stale ? RGB565_YELLOW : RGB565_GREEN);

  snprintf(line, sizeof(line), "HLT:BUS %umV %umA", hltStatus.busMv,
           hltStatus.busMa);
  drawLine(7, line, RGB565_WHITE);

  snprintf(line, sizeof(line), "HLT:BAT %umV NTC1 %dC NTC2 %dC",
           hltStatus.batteryMv, (int)hltStatus.ntc1TemperatureC,
           (int)hltStatus.ntc2TemperatureC);
  drawLine(8, line, hltStatus.ntcOverTemp ? RGB565_RED : RGB565_WHITE);

  snprintf(line, sizeof(line), "HLT:C1 %s C2 %s",
           HLT331Serial::portStateName(hltStatus.c1State),
           HLT331Serial::portStateName(hltStatus.c2State));
  drawLine(9, line, RGB565_WHITE);

  snprintf(line, sizeof(line), "HLT:W %s FOD %s FULL %s",
           hltStatus.wirelessWork ? "ON" : "OFF",
           hltStatus.wirelessFod ? "Y" : "N",
           hltStatus.wirelessFull ? "Y" : "N");
  drawLine(10, line, hltStatus.wirelessFod ? RGB565_RED : RGB565_WHITE);

  snprintf(line, sizeof(line), "HLT:%s %s %lumW",
           HLT331Serial::wirelessProtocolName(hltStatus.wirelessProtocol),
           HLT331Serial::fastChargeName(hltStatus.fastCharge),
           (unsigned long)hltStatus.wirelessMw);
  drawLine(11, line, RGB565_WHITE);

  snprintf(line, sizeof(line), "HLT:OK %lu CRC %lu LEN %lu",
           (unsigned long)hltStatus.goodFrames,
           (unsigned long)hltStatus.badCrcFrames,
           (unsigned long)hltStatus.badLengthFrames);
  drawLine(12, line, hltStatus.badCrcFrames ? RGB565_YELLOW : RGB565_LIGHTGRAY);

  snprintf(line, sizeof(line), "IRQ:%lu IO2:%s %lums",
           (unsigned long)hltStatus.io1PulseCount,
           hltStatus.io2Working ? "WORK" : "IDLE",
           (unsigned long)hltStatus.ageMs);
  drawLine(13, line,
           stale ? RGB565_YELLOW
                 : (hltStatus.io2Working ? RGB565_GREEN : RGB565_LIGHTGRAY));
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  uint32_t serialStart = millis();
  while (!Serial && (millis() - serialStart < 2000)) {
    delay(10);
  }

  Serial.println();
  Serial.println("Mesh Pocket V2 CW221X + HLT331 monitor");

  pinMode(PIN_IO_CSA, OUTPUT);
  digitalWrite(PIN_IO_CSA, HIGH);

  startDisplay();

  if (!gauge.begin(Wire)) {
    cwStatus.ready = false;
    cwStatus.error = gauge.lastError();
    Serial.print("CW221X begin failed, error=");
    Serial.println((int)cwStatus.error);
  } else {
    readGauge();
  }

  if (!hlt331.begin(Serial1, PIN_9006_RX, PIN_9006_TX)) {
    Serial.print("HLT331 begin failed, error=");
    Serial.println((int)hlt331.lastError());
  }
  hlt331.configureIoPins(PIN_9006_IO1, PIN_9006_IO2, PIN_IO_CSA);
  attachInterrupt(digitalPinToInterrupt(PIN_9006_IO1), onHlt331Io1Pulse,
                  RISING);

  renderStatus();
}

void loop() {
  hlt331.poll();

  uint32_t now = millis();
  bool io1Pulse = hlt331.consumeIo1Pending();
  if (now - lastGaugeReadMs >= 3000) {
    lastGaugeReadMs = now;
    readGauge();
  }

  if (io1Pulse) {
    drainHlt331ForWindow(HLT331_IO1_READ_WINDOW_MS);
    lastRenderMs = millis();
    printStatus();
    renderStatus();
  } else if (now - lastRenderMs >= 500) {
    lastRenderMs = now;
    readHlt331();
    printStatus();
    renderStatus();
  }
}
