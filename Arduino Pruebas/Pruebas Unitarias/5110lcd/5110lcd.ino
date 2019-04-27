#include <WiFi.h>
#include <PCD8544.h>
#include "driver/gpio.h"
static PCD8544 lcd=PCD8544(14,13,27,26,15);
void setup(){
  gpio_set_direction( GPIO_NUM_23, GPIO_MODE_OUTPUT);
  gpio_set_level( GPIO_NUM_23, 1);
  lcd.begin(84, 48);
  lcd.clear();
  delay(500);
}
void loop() 
{
      mostrarPantalla();
}
void mostrarPantalla()
{
  lcd.clear();
  // digital clock display of the time
  lcd.setCursor(5,0);
  lcd.print("Hola Mundo :)");
  lcd.setCursor(10,10);
  lcd.print("eiou eiou");
  delay(10000);
 }
