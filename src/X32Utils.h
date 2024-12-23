#ifndef __X32_UTILS_H
#define __X32_UTILS_H
#include "Arduino.h"

#define EMPTY_STR "EMPTY_STR"

enum X32_COLOR
{
  OFF,
  RD,
  GN,
  YE,
  BL,
  MG,
  CY,
  WH,
  OFFi,
  RDi,
  GNi,
  YEi,
  BLi,
  MGi,
  CYi,
  WHi,
  COLOR_EMPTY
};

typedef struct
{
  uint8_t hue;
  uint8_t sat;
  uint8_t bright;
} Color;

#define DEFAULT_BRIGHTNESS 40
#define COLOR_OFF \
  (Color) { 0, 0, 0 }
#define COLOR_RD \
  (Color) { 96, 255, DEFAULT_BRIGHTNESS }
#define COLOR_GN \
  (Color) { 0, 255, DEFAULT_BRIGHTNESS }
#define COLOR_YE \
  (Color) { 70, 255, DEFAULT_BRIGHTNESS }
#define COLOR_BL \
  (Color) { 160, 255, DEFAULT_BRIGHTNESS }
#define COLOR_MG \
  (Color) { 122, 255, DEFAULT_BRIGHTNESS }
#define COLOR_CY \
  (Color) { 213, 255, DEFAULT_BRIGHTNESS }
#define COLOR_WH \
  (Color) { 255, 0, DEFAULT_BRIGHTNESS }

struct ColorMap
{
  X32_COLOR originalColor;
  Color ledColor;
};

const ColorMap mappedColors[16] = {
    {OFF, COLOR_OFF},
    {OFFi, COLOR_OFF},
    {RD, COLOR_RD},
    {RDi, COLOR_RD},
    {GN, COLOR_GN},
    {GNi, COLOR_GN},
    {YE, COLOR_YE},
    {YEi, COLOR_YE},
    {BL, COLOR_BL},
    {BLi, COLOR_BL},
    {MG, COLOR_MG},
    {MGi, COLOR_MG},
    {CY, COLOR_CY},
    {CYi, COLOR_CY},
    {WH, COLOR_WH},
    {WHi, COLOR_WH},
};

enum X32_SOURCE
{
  SOURCE_OFF,
  In01,
  In02,
  In03,
  In04,
  In05,
  In06,
  In07,
  In08,
  In09,
  In10,
  In11,
  In12,
  In13,
  In14,
  In15,
  In16,
  In17,
  In18,
  In19,
  In20,
  In21,
  In22,
  In23,
  In24,
  In25,
  In26,
  In27,
  In28,
  In29,
  In30,
  In31,
  In32,
  Aux1,
  Aux2,
  Aux3,
  Aux4,
  Aux5,
  Aux6,
  USB_L,
  USB_R,
  Fx_1L,
  Fx_1R,
  Fx_2L,
  Fx_2R,
  Fx_3L,
  Fx_3R,
  Fx_4L,
  Fx_4R,
  Bus_01,
  Bus_02,
  Bus_03,
  Bus_04,
  Bus_05,
  Bus_06,
  Bus_07,
  Bus_08,
  Bus_09,
  Bus_10,
  Bus_11,
  Bus_12,
  Bus_13,
  Bus_14,
  Bus_15,
  Bus_16,
  SOURCE_EMPTY
};

#define CHANNELS_PER_FADER_BANK 8

enum ChFaderBanks
{
  CH_1_8,
  CH_9_16,
  CH_17_24,
  CH_25_32,
  AUX_IN_USB,
  FX_RET,
  BUS_MASTER_1_8,
  BUS_MASTER_9_16,
  EMPTY_CH_BANK
};

enum GrpFaderBanks
{
  DCA_1_8,
  BUS_1_8,
  BUS_9_16,
  MATRIX_MAIN_C,
  NONE,
  CH_1_16_SPREAD,
  EMPTY_GRP_BANK
};

struct ChFaderBank
{
  ChFaderBanks bank;
  const char *channels[CHANNELS_PER_FADER_BANK];
};

struct GrpFaderBank
{
  GrpFaderBanks bank;
  const char *channels[CHANNELS_PER_FADER_BANK];
};

extern const char *inputMap[65];
extern ChFaderBank channelBanks[4];
extern GrpFaderBank groupBanks[5];

struct ChannelInfo
{
  char *address;
  bool hasSourceField;
};

struct InternalChannelInfo
{
  ChannelInfo channel;
  String name;
  X32_COLOR color;
  X32_SOURCE source;
  bool initialized;
};

#define TOTAL_CHANNEL_COUNT 80

enum X32ChannelsEnum
{
  CH_1,
  CH_2,
  CH_3,
  CH_4,
  CH_5,
  CH_6,
  CH_7,
  CH_8,
  CH_9,
  CH_10,
  CH_11,
  CH_12,
  CH_13,
  CH_14,
  CH_15,
  CH_16,
  CH_17,
  CH_18,
  CH_19,
  CH_20,
  CH_21,
  CH_22,
  CH_23,
  CH_24,
  CH_25,
  CH_26,
  CH_27,
  CH_28,
  CH_29,
  CH_30,
  CH_31,
  CH_32,
  AUX_IN_1,
  AUX_IN_2,
  AUX_IN_3,
  AUX_IN_4,
  AUX_IN_5,
  AUX_IN_6,
  AUX_IN_7,
  AUX_IN_8,
  FX_RTN_1,
  FX_RTN_2,
  FX_RTN_3,
  FX_RTN_4,
  FX_RTN_5,
  FX_RTN_6,
  FX_RTN_7,
  FX_RTN_8,
  BUS_1,
  BUS_2,
  BUS_3,
  BUS_4,
  BUS_5,
  BUS_6,
  BUS_7,
  BUS_8,
  BUS_9,
  BUS_10,
  BUS_11,
  BUS_12,
  BUS_13,
  BUS_14,
  BUS_15,
  BUS_16,
  MTX_1,
  MTX_2,
  MTX_3,
  MTX_4,
  MTX_5,
  MTX_6,
  MAINT_ST,
  MAIN_M,
  DCA_1,
  DCA_2,
  DCA_3,
  DCA_4,
  DCA_5,
  DCA_6,
  DCA_7,
  DCA_8,
};

extern const ChannelInfo X32Channels[80];

#endif