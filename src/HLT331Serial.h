#ifndef HLT331_SERIAL_H
#define HLT331_SERIAL_H

#include <HeltecMeshPocketV2PowerTarget.h>

#include <Arduino.h>
#include <HardwareSerial.h>
#include <stddef.h>
#include <stdint.h>

class HLT331Serial {
public:
  static const uint32_t DEFAULT_BAUD = 115200;
  static const uint8_t FRAME_HEADER_0 = 0xA5;
  static const uint8_t FRAME_HEADER_1 = 0x5A;
  static const uint8_t DATA_LENGTH = 13;

  enum Error {
    ERROR_NONE = 0,
    ERROR_NOT_BEGUN,
    ERROR_INVALID_ARGUMENT,
    ERROR_BAD_LENGTH,
    ERROR_BAD_CRC
  };

  enum PortState {
    PORT_NOT_CONNECTED = 0,
    PORT_CHARGING = 1,
    PORT_DISCHARGING = 2,
    PORT_RESERVED = 3
  };

  enum WirelessProtocol {
    WIRELESS_NULL = 0,
    WIRELESS_BPP = 1,
    WIRELESS_IPHONE_7P_5W = 2,
    WIRELESS_IPHONE_7P_PLUS = 3,
    WIRELESS_EPP = 4,
    WIRELESS_SAMSUNG = 5,
    WIRELESS_HUAWEI = 6,
    WIRELESS_MPP_RESTRICTED = 7,
    WIRELESS_MPP_FULL = 8
  };

  enum FastChargeMode {
    FAST_CHARGE_NONE = 0,
    FAST_CHARGE_QC2 = 1,
    FAST_CHARGE_QC3 = 2,
    FAST_CHARGE_QC3_PLUS = 3,
    FAST_CHARGE_FCP = 4,
    FAST_CHARGE_SCP = 5,
    FAST_CHARGE_PD_FIXED = 6,
    FAST_CHARGE_PD_PPS = 7,
    FAST_CHARGE_PE_1_1 = 8,
    FAST_CHARGE_PE_2_0 = 9,
    FAST_CHARGE_AFC = 11
  };

  struct Status {
    bool valid;
    bool charging;
    uint8_t socPercent;
    uint8_t c1State;
    uint8_t c2State;
    bool ntcOverTemp;
    bool wirelessWork;
    bool wirelessFod;
    bool wirelessFull;
    uint8_t wirelessProtocol;
    uint8_t fastCharge;
    int8_t ntc1TemperatureC;
    int8_t ntc2TemperatureC;
    uint16_t batteryMv;
    uint16_t busMv;
    uint16_t busMa;
    uint32_t wirelessMw;
    uint8_t raw[DATA_LENGTH];
    uint32_t goodFrames;
    uint32_t badCrcFrames;
    uint32_t badLengthFrames;
    uint32_t lastFrameMs;
    uint32_t ageMs;
    bool io1Active;
    bool io1Pending;
    uint32_t io1PulseCount;
    uint32_t lastIo1PulseMs;
    bool io2Working;
    bool notifyActive;
    bool systemWorking;
    Error error;
  };

  HLT331Serial();

  bool begin(HardwareSerial &serial, int rxPin, int txPin,
             uint32_t baud = DEFAULT_BAUD);
  void configureIoPins(int io1InterruptPin, int io2WorkPin, int csaPin = -1);
  void configurePins(int notifyPin, int io1Pin, int io2Pin, int csaPin = -1);

  bool poll();
  bool feed(uint8_t byte);
  bool readStatus(Status &status);
  void markIo1PulseFromIsr();
  bool consumeIo1Pending();

  bool isBegun() const;
  Error lastError() const;
  uint32_t goodFrameCount() const;
  uint32_t badCrcCount() const;
  uint32_t badLengthCount() const;
  uint32_t lastFrameAgeMs() const;
  uint32_t io1PulseCount() const;
  uint32_t lastIo1PulseMs() const;
  bool io1Active() const;
  bool io2Working() const;
  bool isSystemWorking() const;
  bool notifyActive() const;

  static uint8_t crc8Maxim(uint8_t len, const uint8_t *data);
  static const char *portStateName(uint8_t state);
  static const char *wirelessProtocolName(uint8_t protocol);
  static const char *fastChargeName(uint8_t fastCharge);

private:
  enum ParserState {
    STATE_HEADER_0 = 0,
    STATE_HEADER_1,
    STATE_LENGTH,
    STATE_DATA,
    STATE_CRC
  };

  static uint8_t crc8MaximUpdate(uint8_t crc, uint8_t data);
  static uint16_t littleEndianWord(uint8_t low, uint8_t high);

  void resetParser();
  void parseData();
  void setError(Error error);

  HardwareSerial *_serial;
  int _rxPin;
  int _txPin;
  int _io1InterruptPin;
  int _io2WorkPin;
  int _csaPin;
  uint32_t _baud;
  bool _begun;
  ParserState _parserState;
  uint8_t _frameLen;
  uint8_t _dataIndex;
  uint8_t _data[DATA_LENGTH];
  Status _status;
  Error _lastError;
  volatile bool _io1Pending;
  volatile uint32_t _io1PulseCount;
  volatile uint32_t _lastIo1PulseMs;
};

#endif
