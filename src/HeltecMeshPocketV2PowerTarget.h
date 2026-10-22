#ifndef HELTEC_MESHPOCKET_V2_POWER_TARGET_H
#define HELTEC_MESHPOCKET_V2_POWER_TARGET_H

// This archive is built for the pinned Mesh Pocket V2 firmware toolchain.
#if !defined(__GNUC__) || defined(__clang__) || __GNUC__ != 12 || \
    __GNUC_MINOR__ != 3 || __GNUC_PATCHLEVEL__ != 1
#error "HeltecMeshPocketV2Power requires GCC 12.3.1."
#endif

#if !defined(__cplusplus) || __cplusplus < 201703L
#error "HeltecMeshPocketV2Power requires C++17 or later."
#endif

#if !defined(ARDUINO_ARCH_NRF52) || !defined(NRF52840_XXAA) || \
    !defined(__arm__) || !defined(__thumb__) || \
    !defined(__ARM_ARCH_7EM__)
#error "HeltecMeshPocketV2Power requires nRF52840 Cortex-M4 Thumb."
#endif

#if !defined(__ARM_PCS_VFP) || !defined(__ARM_FP) || __ARM_FP != 4 || \
    defined(__SOFTFP__)
#error "HeltecMeshPocketV2Power requires hard-float FPv4-SP-D16."
#endif

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_LITTLE_ENDIAN__) || \
    __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "HeltecMeshPocketV2Power requires little-endian byte order."
#endif

#if !defined(__SIZEOF_POINTER__) || __SIZEOF_POINTER__ != 4
#error "HeltecMeshPocketV2Power requires 32-bit pointers."
#endif

#include <Arduino.h>

// A precompiled package without source files must expose the core's USB
// dependency to PlatformIO LDF, including for HLT331-only UART consumers.
#if defined(USE_TINYUSB)
#include <Adafruit_TinyUSB.h>
#endif

#endif
