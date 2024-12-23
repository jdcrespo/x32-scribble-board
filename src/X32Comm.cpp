#include "X32Comm.h"

// #define DEBUG

X32Comm::X32Comm(const char *X32IP)
{
    this->X32IP = X32IP;
    socket = WiFiUDP();
    this->firstRefreshDone = false;
    // scribble = new X32Scribble(4, 0x70, 0x71);
}

X32Comm::~X32Comm()
{
}

void X32Comm::init()
{
    socket.begin(LOCAL_UDP_PORT);
    state = X32State();

    msg.empty().setAddress(XREMOTE);
    sendMsg(msg);

    lastXremote = millis();
    lastInitialMsg = millis();
    lastPing = millis();
    scribble.init();
}

void X32Comm::handleRefreshScribble()
{
    for (uint8_t i = 0; i < CHANNELS_PER_FADER_BANK; i++)
    {
        ChannelState *chState = state.getChBankChannel(i);
        if (chState != nullptr)
        {
            chState->print(Serial);
            scribble.setChannel(
                i,
                chState->getName(),
                chState->hasSource() ? chState->getSource() : SOURCE_EMPTY,
                chState->getColor());
        }
    }

    for (uint8_t i = 0; i < CHANNELS_PER_FADER_BANK; i++)
    {
        ChannelState *chState = state.getGrpBankChannel(i);
        if (chState != nullptr)
        {
            chState->print(Serial);
            scribble.setChannel(
                i + CHANNELS_PER_FADER_BANK,
                chState->getName(),
                chState->hasSource() ? chState->getSource() : SOURCE_EMPTY,
                chState->getColor());
        }
    }

    scribble.refresh();
}

void X32Comm::handleXremote()
{
    unsigned long currentMillis;

    currentMillis = millis();
    if ((currentMillis - lastXremote) > 9000)
    {
#ifdef DEBUG
        Serial.println("Sending xremote");
#endif
        msg.empty().setAddress(XREMOTE);
        sendMsg(msg);
        lastXremote = millis();
    }
}

void X32Comm::handleInitialMsg()
{

    unsigned long currentMillis;

    currentMillis = millis();
    if ((currentMillis - lastInitialMsg) > 2000)
    {
        if (!state.isInitialized())
        {
            msg.empty().setAddress(CHFADERBANK);
            sendMsg(msg);
            msg.empty().setAddress(GRPFADERBANK);
            sendMsg(msg);
        }
        lastInitialMsg = millis();
    }
}

void X32Comm::handlePing()
{
    unsigned long currentMillis;

    currentMillis = millis();

    if ((currentMillis - lastPing) > (connected ? PING_TIME_MS : FAST_PING_TIME_MS))
    {
        connected = (lastPingReponse - lastPing) < PING_TIME_MS;

#ifdef DEBUG
        Serial.println("Sending ping");
        Serial.println("conected: " + String(connected));
#endif
        msg.empty().setAddress(INFO);
        sendMsg(msg);
        lastPing = millis();
    }
}

bool X32Comm::isConnected()
{
    return connected;
}

void X32Comm::handleInfo()
{
#ifdef DEBUG
    Serial.println(socket.remoteIP());
#endif
    lastPingReponse = millis();
}

void X32Comm::handleOSCRecv()
{
    OSCMessage msg;
    int size = socket.parsePacket();

    if (size > 0)
    {
        while (size--)
        {
            msg.fill(socket.read());
        }

        if (!msg.hasError())
        {
#ifdef DEBUG
            Serial.println("command recv: " + String(msg.getAddress()));
#endif

            if (msg.fullMatch(CHFADERBANK) || msg.fullMatch(GRPFADERBANK))
            {
                handleFaderBank(msg);
            }
            else if (msg.fullMatch(INFO))
            {
                handleInfo();
            }
            else if (String(msg.getAddress()).indexOf("/config/name") > 0 ||
                     String(msg.getAddress()).indexOf("/config/color") > 0 ||
                     String(msg.getAddress()).indexOf("/config/source") > 0)
            {
                handleChannelConfig(msg);
            }
            else
            {
                // Serial.println("command recv: " + String(msg.getAddress()));
            }
        }
        else
        {
            error = msg.getError();
            Serial.print("recv error: ");
            Serial.println(error);
        }
    }
    return;
}

void X32Comm::handleFaderBank(OSCMessage &msg)
{

    if (strcmp(msg.getAddress(), CHFADERBANK) == 0)
    {
        state.setChFaderBank(static_cast<ChFaderBanks>(msg.getInt(0)));
    }
    if (strcmp(msg.getAddress(), GRPFADERBANK) == 0)
    {
        state.setGrpFaderBank(static_cast<GrpFaderBanks>(msg.getInt(0)));
    }
    handleRefreshScribble();
}

void X32Comm::handleChannelConfig(OSCMessage &msg)
{

#ifdef DEBUG
    Serial.println("handleChannelConfig");
#endif
    String address = msg.getAddress();

#ifdef DEBUG
    Serial.println("handleChannelConfig address " + String(address));
#endif

    String addr = String(address);
    addr.replace("/config/name", "");
    addr.replace("/config/color", "");
    addr.replace("/config/source", "");
#ifdef DEBUG
    Serial.print("address to update: ");
    Serial.println(addr);
#endif

    if (address.indexOf("/config/name") > 0)
    {
        int length = msg.getDataLength(0);
        char str[length];
        msg.getString(0, str, length);
        Serial.println(str);
        state.setChannelNameByAddr(addr.c_str(), str);
    }
    else if (address.indexOf("/config/color") > 0)
    {
        uint8_t color = msg.getInt(0);
        state.setChannelColorByAddr(addr.c_str(), static_cast<X32_COLOR>(color));
    }
    else if (address.indexOf("/config/source") > 0)
    {
        uint8_t source = msg.getInt(0);
        state.setChannelSourceByAddr(addr.c_str(), static_cast<X32_SOURCE>(source));
    }

    if (state.isInitialized() && state.isAddrInCurrentBanks(addr.c_str()))
    {
        Serial.println("channel in current banks! updating scribble");
        handleRefreshScribble();
    }
    // scribble.updateChannel(channelIndex);
}

void X32Comm::handle()
{
    handleXremote();
    handlePing();

    if (connected)
    {
        handleInitialMsg();
    }

    if (!state.isInitialized())
    {
        Serial.println("Loading: " + String(state.getPercentInitialized()) + "%");
        const char *nextAddr = state.getNextEmptyChannelAddress();
        msg.empty().setAddress(nextAddr);
        sendMsg(msg);
    }
    else if (!firstRefreshDone)
    {
        handleRefreshScribble();
        firstRefreshDone = true;
    }

    handleOSCRecv();

    if (Serial.available() > 0)
    {
        char command = Serial.read();
        switch (command)
        {
        case 'd':
            state.print(Serial);
            break;

        default:
            Serial.println("Unknow command");
            break;
        }
        Serial.flush();
    }
}

void X32Comm::sendMsg(OSCMessage &msg)
{
#ifdef DEBUG
    Serial.print("Sending to: " + String(X32IP) + " msg: ");
    msg.send(Serial);
    Serial.println("");
#endif
    socket.beginPacket(X32IP, 10023);
    msg.send(socket);
    socket.endPacket();
}