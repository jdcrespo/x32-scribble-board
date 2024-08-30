#include "X32Scribble.h"
// #define DEBUG

X32Scribble::X32Scribble(uint8_t LED_DATA_PIN, uint8_t MUX_ADDR_A, uint8_t MUX_ADDR_B)
{
  display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
  muxAddrs[0] = MUX_ADDR_A;
  muxAddrs[1] = MUX_ADDR_B;

  maxAvailableChannels = MUX_ADDR_B > 0 ? MAX_CHANNELS : REDUCED_CHANNELS;

  ledPin = LED_DATA_PIN;
}

X32Scribble::~X32Scribble()
{
}

void X32Scribble::init()
{

  Wire.begin();

  // TODO assign ledPin from class variable
  FastLED.addLeds<WS2812, 4, RGB>(leds, maxAvailableChannels);

  for (int i = 0; i < maxAvailableChannels; i++)
  {
    selectMux(i);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
      Serial.print(F("SSD1306 allocation failed. BUS: "));
      Serial.println(i);
    }

    channelName[i] = "";
    channelSource[i] = SOURCE_OFF;
    channelColor[i] = OFF;

    display.clearDisplay();
    display.display();

    leds[i].setHSV(0, 0, 0);
    FastLED.show();
  }
}

void X32Scribble::setChannel(uint8_t channel, String name, X32_SOURCE source, X32_COLOR color)
{
  if (channel >= 0 && channel < MAX_CHANNELS)
  {
    channelName[channel] = name;
    channelSource[channel] = source;
    channelColor[channel] = color;
  }
}

void X32Scribble::setChannelName(uint8_t channel, String name)
{
  if (channel >= 0 && channel < MAX_CHANNELS)
  {
    channelName[channel] = name;
  }
}

void X32Scribble::setChannelSource(uint8_t channel, X32_SOURCE source)
{
  if (channel >= 0 && channel < MAX_CHANNELS)
  {
    channelSource[channel] = source;
  }
}

void X32Scribble::setChannelColor(uint8_t channel, X32_COLOR color)
{
  if (channel >= 0 && channel < MAX_CHANNELS)
  {
    channelColor[channel] = color;
  }
}

void X32Scribble::writeToMux(uint8_t addr, uint8_t cmd)
{
  uint8_t buff = cmd >= 0 ? (1 << cmd) : 0x00;
  Wire.beginTransmission(addr);
  Wire.write(buff);
  Wire.endTransmission();
}

void X32Scribble::selectMux(uint8_t channel)
{
  if (channel >= 0 && channel < maxAvailableChannels)
  {
    uint8_t selectedAddrs = channel >= REDUCED_CHANNELS ? muxAddrs[1] : muxAddrs[0];
    uint8_t unselectedAddr = channel >= REDUCED_CHANNELS ? muxAddrs[0] : muxAddrs[1];
    uint8_t selectedChannel = channel >= REDUCED_CHANNELS ? channel - REDUCED_CHANNELS : channel;
#ifdef DEBUG
    Serial.println(
        "channel: " + String(channel) +
        " selectedAddrs: " + String(selectedAddrs, HEX) +
        " unselectedAddr: " + String(unselectedAddr, HEX) +
        " selectedChannel: " + String(selectedChannel, BIN));
#endif
    writeToMux(selectedAddrs, selectedChannel);
    writeToMux(unselectedAddr, -1);
  }
}

void X32Scribble::refresh()
{
  for (uint8_t i = 0; i < maxAvailableChannels; i++)
  {
    updateChannel(i);
  }
}

void X32Scribble::setChFaderBank(uint8_t bank)
{
  chFaderBank = bank;
}

void X32Scribble::setGrpFaderBank(uint8_t bank)
{
  grpFaderBank = bank;
}

Color X32Scribble::getMappedColor(X32_COLOR x32Color)
{
  for (uint8_t i = 0; i < 16; i++)
  {
    if (mappedColors[i].originalColor == x32Color)
    {
      return mappedColors[i].ledColor;
    }
  }
  return COLOR_OFF;
}

void X32Scribble::updateChannel(uint8_t channel)
{
  /*   for (uint8_t i = 0; i < maxAvailableChannels; i++)
    {
      Serial.println(String(i) + " -> [" +
                     String(channelName[i]) + "," +
                     String(channelColor[i]) + "," +
                     String(channelSource[i]) + "]");
    } */

  selectMux(LCD_INDEX_MAP[channel]);

  X32_COLOR originalColor = channelColor[channel];
  Color color = getMappedColor(originalColor);

  // Clear the buffer
  display.clearDisplay();
  display.invertDisplay(originalColor >= 8);
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(SCREEN_START_X, SCREEN_START_Y);

  String input = String(inputMap[channelSource[channel]]);

  String channelStrip = "";

  if (channel > 1)
  {
    int spaceCount = 6 - input.length();
    for (int i = 0; i < spaceCount; i++)
    {
      channelStrip += " ";
    }

    channelStrip += input;
  }
  else
  {
    display.setTextSize(1);
    channelStrip = (channel == 0 ? "chFB: " : "grpFB: ") +
                   String(channel == 0 ? chFaderBank : grpFaderBank);
  }
  display.write(channelStrip.c_str());

  String label = channelName[channel];
  display.setTextSize(3);
  display.setCursor(SCREEN_START_X, SCREEN_START_Y + 17);
  display.write(label.c_str());

  if (label.length() > 4)
  {
    display.startscrollleft(0x05, 0x07);
  }
  else
  {

    display.stopscroll();
  }
  display.display();
#ifdef DEBUG
  Serial.println("recvColor: " + String(channelColor[bus]) +
                 " foundColor: " + color.hue + "," + color.sat + "," + color.bright);
#endif
  leds[channel].setHSV(color.hue, color.sat, color.bright);
  FastLED.show();
}
