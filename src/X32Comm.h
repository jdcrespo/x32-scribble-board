#ifndef __X32_COMM_H
#define __X32_COMM_H
#include <Arduino.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include "X32Utils.h"
#include "X32Scribble.h"
#include <X32State/X32State.hpp>

#define LOCAL_UDP_PORT 10023

#define PING_TIME_MS 5000
#define FAST_PING_TIME_MS 1000

#define QUEUE_SENDER_RATE_MS 20

#define XREMOTE "/xremote"
#define INFO "/info"
#define CHFADERBANK "/-stat/chfaderbank"
#define GRPFADERBANK "/-stat/grpfaderbank"

class X32Comm
{
private:
  X32State state;

  unsigned long lastXremote;
  unsigned long lastInitialMsg;
  unsigned long lastPing;
  unsigned long lastPingReponse;
  const char *X32IP;
  X32Scribble scribble = X32Scribble(4, 0x70, 0x71);
  WiFiUDP socket;
  OSCMessage msg;
  OSCErrorCode error;
  bool connected;
  bool firstRefreshDone;

  void handleRefreshScribble();

  // Internal communication functions
  void handleXremote();
  void handleInitialMsg();
  void handlePing();

  void handleInfo();
  void handleFaderBank(OSCMessage &msg);
  void handleChannelConfig(OSCMessage &msg);
  void handleOSCRecv();

public:
  X32Comm(const char *X32IP);
  ~X32Comm();

  void init();
  void sendMsg(OSCMessage &msg);
  bool isConnected();
  void handle();
};

#endif