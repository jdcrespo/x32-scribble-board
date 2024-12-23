#include <Arduino.h>
#include <ETH.h>

#include <WebServer.h>
#include <ElegantOTA.h>

#include "X32Comm.h"
#include "testUtils.h"

const char *X32IP = "192.168.100.2";
X32Comm x32comm = X32Comm(X32IP);

void WiFiEvent(WiFiEvent_t event)
{

  switch (event)
  {
  case ARDUINO_EVENT_ETH_START:
    Serial.println("ETH Started");
    // set eth hostname here
    ETH.setHostname("esp32-ethernet");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex())
    {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    // eth_connected = true;
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    // eth_connected = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
    // eth_connected = false;
    break;
  default:
    Serial.print("Unknow event: ");
    Serial.println(event);
    break;
  }
}

WebServer server(80);

IPAddress local_IP(192, 168, 100, 1);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);

unsigned long ota_progress_millis = 0;

void setup()
{
  Serial.begin(256000);
  while (!Serial)
  {
    ; // Wait for serial to connect
  }

  WiFi.onEvent(WiFiEvent);

  if (!ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720))
  {
    Serial.println("Failed to configure Ethernet");
  }
  else
  {
    ETH.setHostname("x32-scribble");
    if (ETH.localIP().toString().equals("0.0.0.0"))
    {
      Serial.println("IP is empty!");
      if (!ETH.config(local_IP, gateway, subnet, primaryDNS, primaryDNS))
      {
        Serial.println("Failed to configure Static IP");
      }
      else
      {
        Serial.print(" Static IP assigned");
        Serial.println(local_IP);
      }
    }
    else
    {
      Serial.print("  DHCP assigned IP ");
      Serial.println(ETH.localIP());
    }
  }

  server.on("/", []()
            { server.send(200, "text/plain", "Hi! This is ElegantOTA Demo."); });

  server.onNotFound([]()
                    { server.send(404, "text/plain", "Not found!"); });
  ElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");

  x32comm.init();

  Serial.println("Everyting ready!");
}

void loop(void)
{
  server.handleClient();
  ElegantOTA.loop();
  x32comm.handle();
}