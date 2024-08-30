#include "X32Comm.h"

// #define DEBUG

X32Comm::X32Comm(const char *X32IP)
{
    this->X32IP = X32IP;
    socket = WiFiUDP();
    initialized = false;
    queue = List<String>();
    // scribble = new X32Scribble(4, 0x70, 0x71);
}

X32Comm::~X32Comm()
{
}

void X32Comm::init()
{
    socket.begin(LOCAL_UDP_PORT);
    msg.empty().setAddress(XREMOTE);
    grpFaderBank = -1;
    chFaderBank = -1;
    sendMsg(msg);
    lastXremote = millis();
    lastInitialMsg = millis();
    lastPing = millis();
    scribble.init();
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
        if (!initialized)
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
            int queueIndex = findQueueIndx(msg.getAddress());
#ifdef DEBUG
            Serial.println("command recv: " + String(msg.getAddress()));
#endif

            if (queueIndex != -1)
            {
                queue.remove(queueIndex);
#ifdef DEBUG
                Serial.println("removed from queue size: " + String(queue.getSize()));
#endif
            }

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

void X32Comm::handleQueueMessage(String msg)
{
    if (findQueueIndx(msg) == -1)
    {
        queue.add(msg);
    }
}

int X32Comm::findQueueIndx(String msg)
{
    for (int i = 0; i < queue.getSize(); i++)
    {
        if (queue[i].equals(msg))
        {
            return i;
        }
    }

    return -1;
}

void X32Comm::handleQueueSender()
{
    unsigned long currentMillis;

    currentMillis = micros();

    if ((currentMillis - lastQueueSend) > QUEUE_SENDER_RATE_MS)
    {
        if (currentSenderIndex >= queue.getSize())
        {
            currentSenderIndex = 0;
        }

        if (queue.getSize() > 0)
        {
#ifdef DEBUG
            Serial.println("sending index [" + String(currentSenderIndex) + "] [" + queue.get(currentSenderIndex) + "]");
#endif
            msg.empty().setAddress(queue.get(currentSenderIndex).c_str());
            sendMsg(msg);
        }
        currentSenderIndex++;

        lastQueueSend = micros();
    }
}

void X32Comm::debug()
{
    for (int i = 0; i < queue.getSize(); i++)
    {
        Serial.println(queue[i]);
    }
}

void X32Comm::handleFaderBank(OSCMessage &msg)
{
    uint8_t originalChFaderBank = chFaderBank;
    uint8_t originalGrpFaderBank = grpFaderBank;

    if (strcmp(msg.getAddress(), CHFADERBANK) == 0)
    {
        chFaderBank = msg.getInt(0);
    }
    if (strcmp(msg.getAddress(), GRPFADERBANK) == 0)
    {
        grpFaderBank = msg.getInt(0);
    }

    if (chFaderBank != -1 && grpFaderBank != -1 && !initialized)
    {
        initialized = true;
    }

    if (originalChFaderBank != chFaderBank)
    {
        handleReloadChBank();
        scribble.setChFaderBank(chFaderBank);
    }

    if (originalGrpFaderBank != grpFaderBank)
    {
        handleReloadGrpBank();
        scribble.setGrpFaderBank(grpFaderBank);
    }

    // Serial.println("faderbank: " + String(msg.getAddress()) + " -> " + String(msg.getInt(0)));
}

int X32Comm::getChannelInCurrentBank(String address)
{
    bool found = false;
    const char **channels;
    const char **groups;
    uint8_t chbank = -1;
    uint8_t grpbank = -1;
    bool isGroupBank = false;

    if (chFaderBank == -1 && grpFaderBank == -1)
    {
        return -1;
    }

    for (unsigned int i = 0; i < sizeof(channelBanks) / sizeof(ChFaderBank); i++)
    {
        if (channelBanks[i].bank == chFaderBank)
        {
            chbank = channelBanks[i].bank;
            channels = channelBanks[i].channels;
            break;
        }
    }

    for (unsigned int i = 0; i < sizeof(groupBanks) / sizeof(GrpFaderBank); i++)
    {
        if (groupBanks[i].bank == grpFaderBank)
        {
            grpbank = groupBanks[i].bank;
            groups = groupBanks[i].channels;
            isGroupBank = true;
            break;
        }
    }
#ifdef DEBUG
    Serial.println(
        "currentfader end" + String(found) +
        " chbank:" + String(chbank) +
        " grpbank:" + String(grpbank) +
        " isGroup: " + String(isGroupBank));
#endif
    if (chbank > -1)
    {
        for (int j = 0; j < CHANNELS_PER_FADER_BANK; j++)
        {
            if (address.indexOf(channels[j]) == 0)
            {
                return j;
            }
        }
    }

    if (grpbank > -1)
    {
        for (int j = 0; j < CHANNELS_PER_FADER_BANK; j++)
        {
            if (address.indexOf(groups[j]) == 0)
            {
                return j + 8;
            }
        }
    }

    return -1;
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

    int channelIndex = getChannelInCurrentBank(address);

    if (channelIndex != -1)
    {
        if (address.indexOf("/config/name") > 0)
        {
            int length = msg.getDataLength(0);
            char str[length];
            msg.getString(0, str, length);
            // channelInfo[internalChannelIndex].name = String(str);
            scribble.setChannelName(channelIndex, str);
        }
        else if (address.indexOf("/config/color") > 0)
        {
            uint8_t color = msg.getInt(0);
            // channelInfo[internalChannelIndex].color = (X32_COLOR)color;
            scribble.setChannelColor(channelIndex, (X32_COLOR)color);
        }
        else if (address.indexOf("/config/source") > 0)
        {
            uint8_t source = msg.getInt(0);
            // channelInfo[internalChannelIndex].source = (X32_SOURCE)source;
            scribble.setChannelSource(channelIndex, (X32_SOURCE)source);
        }
        scribble.updateChannel(channelIndex);
    }
}

void X32Comm::handleReloadChBank()
{

    const char **channels;
    uint8_t chbank = -1;
    for (unsigned int i = 0; i < sizeof(channelBanks) / sizeof(ChFaderBank); i++)
    {
        if (channelBanks[i].bank == chFaderBank)
        {
            chbank = channelBanks[i].bank;
            channels = channelBanks[i].channels;
            break;
        }
    }

    if (chbank > -1)
    {
        for (int j = 0; j < CHANNELS_PER_FADER_BANK; j++)
        {

            handleQueueMessage((String(channels[j]) + "/config/name"));

            handleQueueMessage((String(channels[j]) + "/config/color"));
            // msg.empty().setAddress((String(channels[j]) + "/config/color").c_str());

            handleQueueMessage((String(channels[j]) + "/config/source"));

            // msg.empty().setAddress((String(channels[j]) + "/config/source").c_str());
        }
    }
}

void X32Comm::handleReloadGrpBank()
{
    const char **channels;
    uint8_t grpBank = -1;
    for (unsigned int i = 0; i < sizeof(groupBanks) / sizeof(GrpFaderBank); i++)
    {
        if (groupBanks[i].bank == grpFaderBank)
        {
            grpBank = groupBanks[i].bank;
            channels = groupBanks[i].channels;
            break;
        }
    }

    if (grpBank > -1)
    {
        for (int j = 0; j < CHANNELS_PER_FADER_BANK; j++)
        {
            if (!(String(channels[j]).isEmpty()))
            {
                handleQueueMessage((String(channels[j]) + "/config/name"));

                handleQueueMessage((String(channels[j]) + "/config/color"));
                // msg.empty().setAddress((String(channels[j]) + "/config/color").c_str());
                if (String(channels[j]).indexOf("/ch") == 0)
                {
                    handleQueueMessage((String(channels[j]) + "/config/source"));
                }
                // msg.empty().setAddress((String(channels[j]) + "/config/source").c_str());
            }
        }
    }
}

void X32Comm::handle()
{
    handleXremote();
    handlePing();

    if (connected)
    {
        handleInitialMsg();
    }

    handleQueueSender();

    handleOSCRecv();
}

void X32Comm::sendMsg(OSCMessage &msg)
{

    socket.beginPacket("192.168.0.255", 10023);
    msg.send(socket);
    socket.endPacket();
}