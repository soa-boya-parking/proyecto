#include "OneWire.h"
#include "DallasTemperature.h"
  
OneWire oneWire(32);
DallasTemperature tempSensor(&oneWire);
  
void setup(void)
{
  
  Serial.begin(115200);
  tempSensor.begin();
}
  
void loop(void)
{
  tempSensor.requestTemperaturesByIndex(0);
  
  Serial.print("Temperature: ");
  Serial.print(tempSensor.getTempCByIndex(0));
  Serial.println(" C");
  
  delay(2000);
}
