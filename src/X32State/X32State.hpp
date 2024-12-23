#ifndef __X32_STATE_H
#define __X32_STATE_H
#include "ChannelState.hpp"

class X32State
{
private:
  ChFaderBanks chFaderBank;
  GrpFaderBanks grpFaderBank;
  ChannelState *channels[TOTAL_CHANNEL_COUNT];

  int findIndexByAddress(const char *addr);

public:
  X32State();
  ~X32State();

  void setChFaderBank(ChFaderBanks selectedBank);
  void setGrpFaderBank(GrpFaderBanks selectedBank);
  const char *getNextEmptyChannelAddress();
  void setChannelNameByAddr(const char *addr, String name);
  void setChannelColorByAddr(const char *addr, X32_COLOR color);
  void setChannelSourceByAddr(const char *addr, X32_SOURCE source);
  bool isInitialized();
  int getPercentInitialized();
  bool isAddrInCurrentBanks(const char *addr);
  ChannelState *getChBankChannel(uint8_t ch);
  ChannelState *getGrpBankChannel(uint8_t ch);
  void print(Print &p);
};

inline int X32State::findIndexByAddress(const char *addr)
{
  for (uint16_t i = 0; i < TOTAL_CHANNEL_COUNT; i++)
  {
    if (strcmp(addr, channels[i]->getAddress()) == 0)
    {
      return i;
    }
  }
  return -1;
}

inline X32State::X32State()
{
  for (uint16_t i = 0; i < TOTAL_CHANNEL_COUNT; i++)
  {
    channels[i] = new ChannelState(X32Channels[i].address, X32Channels[i].hasSourceField);
  }
  chFaderBank = CH_1_8;
  grpFaderBank = DCA_1_8;
};
inline X32State::~X32State()
{
}

inline void X32State::setChFaderBank(ChFaderBanks selectedBank)
{
  chFaderBank = selectedBank;
}

inline void X32State::setGrpFaderBank(GrpFaderBanks selectedBank)
{
  grpFaderBank = selectedBank;
}

inline const char *X32State::getNextEmptyChannelAddress()
{
  for (uint16_t i = 0; i < TOTAL_CHANNEL_COUNT; i++)
  {
    if (!channels[i]->isInitialized())
    {
      return channels[i]->getNextEmptyAddr();
    }
  }
  return "";
}

inline void X32State::setChannelNameByAddr(const char *addr, String name)
{
  int index = findIndexByAddress(addr);
  if (index > -1)
  {
    Serial.println(name);
    channels[index]->print(Serial);
    channels[index]->setName(name);
    channels[index]->print(Serial);
  }
}

inline void X32State::setChannelColorByAddr(const char *addr, X32_COLOR color)
{
  int index = findIndexByAddress(addr);
  if (index > -1)
  {
    channels[index]->setColor(color);
  }
}

inline void X32State::setChannelSourceByAddr(const char *addr, X32_SOURCE source)
{
  int index = findIndexByAddress(addr);
  if (index > -1)
  {
    channels[index]->setSource(source);
  }
}

inline bool X32State::isInitialized()
{
  bool returnValue = true;
  for (uint16_t i = 0; i < TOTAL_CHANNEL_COUNT; i++)
  {
    returnValue = returnValue && channels[i]->isInitialized();
  }
  return returnValue;
}

inline int X32State::getPercentInitialized()
{
  uint16_t totalInitialized = 0;
  for (uint16_t i = 0; i < TOTAL_CHANNEL_COUNT; i++)
  {
    if (channels[i]->isInitialized())
    {
      totalInitialized++;
    }
  }

  return (int)(totalInitialized * 100) / TOTAL_CHANNEL_COUNT;
}

inline bool X32State::isAddrInCurrentBanks(const char *addr)
{
  ChFaderBank currentChBank = channelBanks[chFaderBank];

  Serial.println("Current chBank: " + String(currentChBank.bank));
  for (uint8_t i = 0; i < CHANNELS_PER_FADER_BANK; i++)
  {
    if (strcmp(currentChBank.channels[i], addr) == 0)
    {
      Serial.println("found in channel bank");
      return true;
    }
  }

  GrpFaderBank currentGrpBank = groupBanks[grpFaderBank];
  Serial.println("Current grpBank: " + String(currentGrpBank.bank));
  for (uint8_t i = 0; i < CHANNELS_PER_FADER_BANK; i++)
  {
    if (strcmp(currentGrpBank.channels[i], addr) == 0)
    {
      Serial.println("found in group bank");
      return true;
    }
  }
  return false;
}

inline ChannelState *X32State::getChBankChannel(uint8_t ch)
{
  ChFaderBank currentChBank = channelBanks[chFaderBank];
  const char *chAddr = currentChBank.channels[ch];
  Serial.println("addr: " + String(chAddr));
  int index = findIndexByAddress(chAddr);
  if (index > -1)
  {
    return channels[index];
  }
  return nullptr;
}

inline ChannelState *X32State::getGrpBankChannel(uint8_t ch)
{
  GrpFaderBank currentGrpBank = groupBanks[grpFaderBank];
  const char *chAddr = currentGrpBank.channels[ch];
  Serial.println("addr: " + String(chAddr));
  int index = findIndexByAddress(chAddr);
  if (index > -1)
  {
    return channels[index];
  }
  return nullptr;
}

inline void X32State::print(Print &p)
{
  p.printf("chFaderbank: %d grpFaderBank: %d\n", chFaderBank, grpFaderBank);
  p.println("Channel states:");
  for (uint16_t i = 0; i < TOTAL_CHANNEL_COUNT; i++)
  {
    channels[i]->print(p);
  }
};
#endif