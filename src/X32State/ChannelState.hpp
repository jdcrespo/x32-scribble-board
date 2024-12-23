#ifndef __CHANNEL_STATE_H
#define __CHANNEL_sTATE_H

#include <X32Utils.h>

class ChannelState
{
private:
  const char *addr;
  String name;
  bool hasSrc;
  X32_COLOR color;
  X32_SOURCE src;
  char state; // 0 empty 1 name 2 color 4 source

public:
  ChannelState(char *address, bool hasSource);
  ~ChannelState();

  void setName(String name);
  void setColor(X32_COLOR color);
  void setSource(X32_SOURCE source);
  const char *getAddress();
  const char *getNextEmptyAddr();
  bool isInitialized();
  String getName();
  bool hasSource();
  X32_COLOR getColor();
  X32_SOURCE getSource();
  void print(Print &p);
};

inline ChannelState::ChannelState(char *address, bool hasSource)
{
  addr = address;
  name = "";
  color = OFF;
  src = SOURCE_OFF;
  state = 0;
  hasSrc = hasSource;
}

inline ChannelState::~ChannelState()
{
}

inline void ChannelState::setName(String name)
{
  this->name = name;
  state = state | B001;
}

inline void ChannelState::setColor(X32_COLOR color)
{
  this->color = color;
  state = state | B010;
}

inline void ChannelState::setSource(X32_SOURCE source)
{
  if (hasSrc)
  {
    this->src = source;
    state = state | B100;
  }
}

inline const char *ChannelState::getAddress()
{
  return addr;
}

inline const char *ChannelState::getNextEmptyAddr()
{
  String baseAddr = String(addr) + "/config/";
  if (!bitRead(state, 0))
  {
    return (baseAddr + "name").c_str();
  }
  else if (!bitRead(state, 1))
  {
    return (baseAddr + "color").c_str();
  }
  else if (hasSrc && !bitRead(state, 2))
  {
    return (baseAddr + "source").c_str();
  }
  return "";
}

inline void ChannelState::print(Print &p)
{
  p.println("Status:");
  p.printf("addr: %s name: %s color: %d src: %d state: %d\n",
           addr,
           name,
           color,
           src,
           state);
}

inline bool ChannelState::isInitialized()
{
  return hasSrc ? state == B111 : state == B011;
}

inline String ChannelState::getName()
{
  return name;
}

inline bool ChannelState::hasSource()
{
  return hasSrc;
}

inline X32_COLOR ChannelState::getColor()
{
  return color;
}

inline X32_SOURCE ChannelState::getSource()
{
  return src;
}

#endif