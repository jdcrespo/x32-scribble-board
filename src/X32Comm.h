#ifndef __X32_COMM_H
#define __X32_COMM_H
#include <Arduino.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include "X32Utils.h"
#include "X32Scribble.h"
#include "List.hpp"

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
  unsigned long lastXremote;
  unsigned long lastInitialMsg;
  unsigned long lastPing;
  unsigned long lastPingReponse;
  unsigned long lastQueueSend;
  uint8_t chFaderBank;
  uint8_t grpFaderBank;
  const char *X32IP;
  List<String> queue;
  X32Scribble scribble = X32Scribble(4, 0x70, 0x71);
  WiFiUDP socket;
  OSCMessage msg;
  OSCErrorCode error;
  bool initialized;
  bool connected;
  int currentSenderIndex = 0;

  // Internal communication functions
  void handleXremote();
  void handleInitialMsg();
  void handlePing();

  void handleInfo();
  void handleFaderBank(OSCMessage &msg);
  int getChannelInCurrentBank(String address);
  void handleChannelConfig(OSCMessage &msg);
  void handleOSCRecv();
  void handleQueueMessage(String msg);

  int findQueueIndx(String msg);

  void handleReloadChBank();
  void handleReloadGrpBank();

public:
  X32Comm(const char *X32IP);
  ~X32Comm();

  void init();
  void sendMsg(OSCMessage &msg);
  bool isConnected();
  void handle();
  void handleQueueSender();

  void debug();
};

#endif