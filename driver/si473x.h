/* SI4732 Radio Receiver Driver
 * Ported from fagci/uvk5-fagci-reborn (https://github.com/fagci/uvk5-fagci-reborn)
 * Original work Copyright (c) Dual Tachyon / fagci contributors
 * Licensed under the Apache License, Version 2.0
 */

#ifndef SI473X_H
#define SI473X_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  SI47XX_FM,
  SI47XX_AM,
  SI47XX_LSB,
  SI47XX_USB,
  SI47XX_CW,
} SI47XX_MODE;

typedef enum {
  SI47XX_BW_6_kHz,
  SI47XX_BW_4_kHz,
  SI47XX_BW_3_kHz,
  SI47XX_BW_2_kHz,
  SI47XX_BW_1_kHz,
  SI47XX_BW_1_8_kHz,
  SI47XX_BW_2_5_kHz,
} SI47XX_FilterBW;

typedef enum {
  SI47XX_SSB_BW_1_2_kHz,
  SI47XX_SSB_BW_2_2_kHz,
  SI47XX_SSB_BW_3_kHz,
  SI47XX_SSB_BW_4_kHz,
  SI47XX_SSB_BW_0_5_kHz,
  SI47XX_SSB_BW_1_0_kHz,
} SI47XX_SsbFilterBW;

typedef enum {
  CMD_POWER_UP = 0x01,
  CMD_GET_REV = 0x10,
  CMD_POWER_DOWN = 0x11,
  CMD_SET_PROPERTY = 0x12,
  CMD_GET_PROPERTY = 0x13,
  CMD_GET_INT_STATUS = 0x14,
  CMD_PATCH_ARGS = 0x15,
  CMD_PATCH_DATA = 0x16,
  CMD_FM_TUNE_FREQ = 0x20,
  CMD_FM_SEEK_START = 0x21,
  CMD_FM_TUNE_STATUS = 0x22,
  CMD_FM_RSQ_STATUS = 0x23,
  CMD_FM_RDS_STATUS = 0x24,
  CMD_FM_AGC_STATUS = 0x27,
  CMD_FM_AGC_OVERRIDE = 0x28,
  CMD_AM_TUNE_FREQ = 0x40,
  CMD_AM_SEEK_START = 0x41,
  CMD_AM_TUNE_STATUS = 0x42,
  CMD_AM_RSQ_STATUS = 0x43,
  CMD_AM_AGC_STATUS = 0x47,
  CMD_AM_AGC_OVERRIDE = 0x48,
} SI47XX_Commands;

typedef enum {
  FLG_CTSIEN = 0x80,
  FLG_GPO2IEN = 0x40,
  FLG_PATCH = 0x20,
  FLG_XOSCEN = 0x10,
  FLG_FREEZE = 0x02,
  FLG_FAST = 0x01,
  FLG_SEEKUP = 0x08,
  FLG_WRAP = 0x04,
  FLG_CANCEL = 0x02,
  FLG_INTACK = 0x01,
  FLG_BLETHA_0 = 0x00,
  FLG_BLETHA_12 = 0x40,
  FLG_BLETHA_35 = 0x80,
  FLG_BLETHA_U = FLG_BLETHA_12 | FLG_BLETHA_35,
  FLG_BLETHB_0 = FLG_BLETHA_0,
  FLG_BLETHB_12 = 0x10,
  FLG_BLETHB_35 = 0x20,
  FLG_BLETHB_U = FLG_BLETHB_12 | FLG_BLETHB_35,
  FLG_BLETHC_0 = FLG_BLETHA_0,
  FLG_BLETHC_12 = 0x04,
  FLG_BLETHC_35 = 0x08,
  FLG_BLETHC_U = FLG_BLETHC_12 | FLG_BLETHC_35,
  FLG_BLETHD_0 = FLG_BLETHA_0,
  FLG_BLETHD_12 = 0x01,
  FLG_BLETHD_35 = 0x02,
  FLG_BLETHD_U = FLG_BLETHD_12 | FLG_BLETHD_35,
  FLG_RDSEN = 0x01,
  FLG_AMPLFLT = 0x01,
} SI47XX_Flags;

typedef enum {
  FUNC_FM = 0x00,
  FUNC_AM = 0x01,
} SI47XX_FunctionModes;

typedef enum {
  OUT_RDS = 0x00,
  OUT_ANALOG = 0x05,
} SI47XX_OutputModes;

typedef union {
  struct {
    uint8_t AUDIOBW : 4;
    uint8_t SBCUTFLT : 4;
    uint8_t AVC_DIVIDER : 4;
    uint8_t AVCEN : 1;
    uint8_t SMUTESEL : 1;
    uint8_t DUMMY1 : 1;
    uint8_t DSP_AFCDIS : 1;
  } param;
  uint8_t raw[2];
} SsbMode;

typedef enum {
  STATUS_CTS = 0x80,
  STATUS_ERR = 0x40,
  STATUS_RSQINT = 0x08,
  STATUS_RDSINT = 0x04,
  STATUS_STCINT = 0x01,
  STATUS_VALID = 0x01,
} SI47XX_StatusFlagMasks;

typedef enum {
  PROP_FM_DEEMPHASIS = 0x1100,
  PROP_FM_SOFT_MUTE_MAX_ATTENUATION = 0x1302,
  PROP_FM_SEEK_BAND_BOTTOM = 0x1400,
  PROP_FM_SEEK_BAND_TOP = 0x1401,
  PROP_FM_SEEK_FREQ_SPACING = 0x1402,
  PROP_FM_SEEK_TUNE_RSSI_THRESHOLD = 0x1404,
  PROP_FM_RDS_INT_SOURCE = 0x1500,
  PROP_FM_RDS_INT_FIFO_COUNT = 0x1501,
  PROP_FM_RDS_CONFIG = 0x1502,
  PROP_AM_CHANNEL_FILTER = 0x3102,
  PROP_AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN = 0x3103,
  PROP_AM_SOFT_MUTE_MAX_ATTENUATION = 0x3302,
  PROP_AM_SEEK_BAND_BOTTOM = 0x3400,
  PROP_AM_SEEK_BAND_TOP = 0x3401,
  PROP_AM_SEEK_FREQ_SPACING = 0x3402,
  PROP_AM_SEEK_TUNE_RSSI_THRESHOLD = 0x3404,
  PROP_AM_FRONTEND_AGC_CONTROL = 0x3705,
  PROP_RX_VOLUME = 0x4000,
  PROP_SSB_BFO = 0x0100,
  PROP_SSB_MODE = 0x0101,
  PROP_SSB_SOFT_MUTE_MAX_ATTENUATION = 0x3302,
} SI47XX_PropertyCodes;

enum {
  RDS_STATUS_ARG1_CLEAR_INT = 0b001,
};

#define MAKE_WORD(h, l) (((uint16_t)(h) << 8) | (uint16_t)(l))
#define FLG_RDSRECV 0x01

typedef union {
  struct {
    uint8_t STCINT : 1;
    uint8_t DUMMY1 : 1;
    uint8_t RDSINT : 1;
    uint8_t RSQINT : 1;
    uint8_t DUMMY2 : 2;
    uint8_t ERR : 1;
    uint8_t CTS : 1;
    uint8_t RSSIILINT : 1;
    uint8_t RSSIHINT : 1;
    uint8_t SNRLINT : 1;
    uint8_t SNRHINT : 1;
    uint8_t MULTLINT : 1;
    uint8_t MULTHINT : 1;
    uint8_t DUMMY3 : 1;
    uint8_t BLENDINT : 1;
    uint8_t VALID : 1;
    uint8_t AFCRL : 1;
    uint8_t DUMMY4 : 1;
    uint8_t SMUTE : 1;
    uint8_t DUMMY5 : 4;
    uint8_t STBLEND : 7;
    uint8_t PILOT : 1;
    uint8_t RSSI;
    uint8_t SNR;
    uint8_t MULT;
    uint8_t FREQOFF;
  } resp;
  uint8_t raw[8];
} RSQStatus;

typedef union {
  struct {
    uint8_t AGCDIS : 1;
    uint8_t DUMMY : 7;
    uint8_t AGCIDX;
  } arg;
  uint8_t raw[2];
} SI47XX_AgcOverrride;

typedef union {
  struct {
    uint8_t INTACK : 1;
    uint8_t CANCEL : 1;
    uint8_t RESERVED2 : 6;
  } arg;
  uint8_t raw;
} SI47XX_TuneStatus;

typedef union {
  struct {
    uint8_t AMCHFLT : 4;
    uint8_t DUMMY1 : 4;
    uint8_t AMPLFLT : 1;
    uint8_t DUMMY2 : 7;
  } param;
  uint8_t raw[2];
} SI47XX_BW_Config;

void SI47XX_PowerUp(void);
void SI47XX_PatchPowerUp(void);
void SI47XX_PowerDown(void);
void SI47XX_SetFreq(uint16_t freq);
void SI47XX_ReadRDS(uint8_t buf[13]);
void SI47XX_SwitchMode(SI47XX_MODE mode);
bool SI47XX_IsSSB(void);
void RSQ_GET(void);
void SI47XX_SetAutomaticGainControl(uint8_t AGCDIS, uint8_t AGCIDX);
void SI47XX_Seek(bool up, bool wrap);
uint32_t SI47XX_getFrequency(bool *valid);
void SI47XX_SetBandwidth(SI47XX_FilterBW AMCHFLT, bool AMPLFLT);
void SI47XX_SetSsbBandwidth(SI47XX_SsbFilterBW bw);
void SI47XX_SetSeekFmLimits(uint32_t bottom, uint32_t top);
void SI47XX_SetSeekAmLimits(uint32_t bottom, uint32_t top);
void SI47XX_SetSeekFmSpacing(uint32_t spacing);
void SI47XX_SetSeekAmSpacing(uint32_t spacing);
void SI47XX_SetSeekFmRssiThreshold(uint16_t value);
void SI47XX_SetSeekAmRssiThreshold(uint16_t value);
void SI47XX_SetBFO(int16_t bfo);
void SI47XX_TuneTo(uint32_t f);
void SI47XX_SetVolume(uint8_t volume);
void SI47XX_SetAMFrontendAGC(uint8_t minGainIdx, uint8_t attnBackup);

extern SI47XX_MODE si4732mode;
extern RSQStatus rsqStatus;
extern uint16_t siCurrentFreq;
extern bool isSi4732On;

#endif /* SI473X_H */
