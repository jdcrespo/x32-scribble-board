#ifndef __X32_SCRIBBLE_H
#define __X32_SCRIBBLE_H
#include <Arduino.h>

#include <FastLED.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "X32Utils.h"

#define MAX_CHANNELS 16
#define REDUCED_CHANNELS 8

#define SCREEN_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_START_X 29
#define SCREEN_START_Y 25

class X32Scribble
{
private:
  const uint8_t LCD_INDEX_MAP[16] = {2, 1, 0, 3, 7, 6, 5, 4, 10, 9, 8, 11, 15, 14, 13, 12};

  String channelName[MAX_CHANNELS];
  X32_SOURCE channelSource[MAX_CHANNELS];
  X32_COLOR channelColor[MAX_CHANNELS];
  uint8_t maxAvailableChannels;

  Adafruit_SSD1306 display;
  CRGB leds[MAX_CHANNELS];

  uint8_t muxAddrs[2];
  uint8_t ledPin;

  void writeToMux(uint8_t addr, uint8_t cmd);
  void selectMux(uint8_t channel);
  Color getMappedColor(X32_COLOR x32Color);
  void updateChannel(uint8_t channel);

public:
  X32Scribble(uint8_t LED_DATA_PIN, uint8_t MUX_ADDR_A, uint8_t MUX_ADDR_B);
  ~X32Scribble();

  /**
   * Initialize all displays and leds
   */
  void init();

  /**
   * Set channel info
   * @param channel Scribble channel number (0-15) max 12 characters
   * @param name Label or text to show in channel
   * @param source Input source of channel (0-64)
   * @param color Color of channel (0, 15)
   */
  void setChannel(uint8_t channel, String name, X32_SOURCE source, X32_COLOR color);

  /**
   * Set channel name and refresh
   * @param name Label or text to show in channel
   */
  void setChannelName(uint8_t channel, String name);
  /**
   * Set channel source and refresh
   * @param source Input source of channel (0-64)
   */
  void setChannelSource(uint8_t channel, X32_SOURCE source);
  /**
   * Set channel color and refresh
   */
  void setChannelColor(uint8_t channel, X32_COLOR color);
  /**
   * Print and set all channels info and colors
   * @param color Color of channel (0, 15)
   */
  void refresh();
};

#endif