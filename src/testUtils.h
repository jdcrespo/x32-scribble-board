#include <Arduino.h>
#include <Wire.h>

void scanBus()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void TCA9548A(uint8_t bus)
{
  uint8_t buff = 0x00;
  buff = 1 << bus;
  Wire.beginTransmission(0x70);
  Wire.write(buff);
  Wire.endTransmission();
}

void TCA9548A_closeAll()
{
  Wire.beginTransmission(0x70);
  Wire.write(0x00);
  Wire.endTransmission();
}

void scanFullBus()
{
  for (int i = 0; i < 8; i++)
  {
    TCA9548A(i);
    Serial.print("Multiplexer ch");
    Serial.print(i);
    Serial.print(" -> ");
    scanBus();
  }
}