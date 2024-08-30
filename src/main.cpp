#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "X32Comm.h"
#include "List.hpp"

#include "testUtils.h"

const char *ssid = "WIFI_abajo";
const char *password = "lossecanos";
const char *X32IP = "192.168.0.155";

unsigned long lastRefresh;

WiFiMulti wifiMulti;

TaskHandle_t X32CommTask;

void X32CommTaskCode(void *parameter)
{
  X32Comm x32comm = X32Comm(X32IP);

  x32comm.init();
  for (;;)
  {
    x32comm.handle();
  }
}

void setup()
{
  Serial.begin(256000);
  while (!Serial)
  {
    ; // Wait for serial to connect
  }

  // WIFI setup
  wifiMulti.addAP("WIFI_abajo", "lossecanos");
  wifiMulti.addAP("JDC_REMOTE", "directos");
  wifiMulti.addAP("JDC-REMOTE", "directos");

  WiFi.begin(ssid, password);
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.printf(" connected!\n");

  xTaskCreatePinnedToCore(
      X32CommTaskCode, /* Function to implement the task */
      "X32CommTask",   /* Name of the task */
      10000,           /* Stack size in words */
      NULL,            /* Task input parameter */
      0,               /* Priority of the task */
      &X32CommTask,    /* Task handle. */
      0);              /* Core where the task should run */

  lastRefresh = millis();
}

void loop()
{

  yield();
}
