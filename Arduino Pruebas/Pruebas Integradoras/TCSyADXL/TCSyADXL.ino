#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <SparkFun_ADXL345.h> 

 
/* Example code for the Adafruit TCS34725 breakout library */
 
/* Connect SCL to analog 5
Connect SDA to analog 4
Connect VDD to 3.3V DC
Connect GROUND to common ground */
 
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();
 
/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

ADXL345 adxl = ADXL345();
 
void setup(void) {
Serial.begin(115200);

if (tcs.begin()) 
{
  Serial.println("Found sensor");
} 
else 
{
  Serial.println("No TCS34725 found ... check your connections");
  while (1);
}

adxl.powerOn();            
adxl.setRangeSetting(16);       //Definir el rango, valores 2, 4, 8 o 16
 
// Now we're ready to get readings!
}
 
void loop(void) {
uint16_t r, g, b, c, colorTemp, lux;
 
tcs.getRawData(&r, &g, &b, &c);
colorTemp = tcs.calculateColorTemperature(r, g, b);
lux = tcs.calculateLux(r, g, b);
 
Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
Serial.println(" ");

//leer los valores e imprimirlos
int x, y, z;
adxl.readAccel(&x, &y, &z);  
Serial.print(x);
Serial.print(", ");
Serial.print(y);
Serial.print(", ");
Serial.println(z); 

}
