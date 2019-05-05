//Termometro
#include "OneWire.h"
#include "DallasTemperature.h"

//Acelerometro
#include "SPI.h"
#include "Wire.h"
#include "SparkFun_ADXL345.h"

//Pantalla
#include "PCD8544.h"
#include "driver/gpio.h"
static PCD8544 lcd=PCD8544(14,13,27,26,15);

//Colorimetro
#include "Adafruit_TCS34725.h"

//C
#include "stdlib.h"

//Pines de sensores analogicos.
const int PINTEMPERATURA = 32;
const int PINAGUA = 36;

//Constructor del acelerometro.
ADXL345 adxl = ADXL345();

//Tareas, concepto similar a Thread, el ESP32 tiene 2 nucleos, por lo tanto puedo tener threads en mas de un nucleo.
TaskHandle_t Task1;
TaskHandle_t Task2;

//Constructor del termometro.
OneWire oneWire(PINTEMPERATURA);
DallasTemperature tempSensor(&oneWire);

//Constructor del colorimetro.
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

//Variables que corresponden a las lineas d la pantalla.
char l1[21] = "";
char l2[21] = "";
char l3[21] = "";
char l4[21] = "";
char l5[21] = "";
char l6[21] = "";

//Inicializaciones.
void setup() 
{

  //-------INICIALIZACION DE PANTALLA.
  gpio_set_direction( GPIO_NUM_23, GPIO_MODE_OUTPUT);
  gpio_set_level( GPIO_NUM_23, 1);
  lcd.begin(84, 48);
  lcd.clear();
  
  //Velocidad puerto serial.
  Serial.begin(115200);

  //-------INICIALIZACION DEL TERMOMETRO. 
  tempSensor.begin();
  mostrar(0, 0, "DS18B20 ON");

  //-------INICIALIZACION DEL COLORIMETRO.
  tcs.begin();
  mostrar(0, 1, "TCS34725 ON");

  //-------INICIALIZACION DE ACELEROMETRO.
  adxl.powerOn();            
  adxl.setRangeSetting(16);

  mostrar(0, 2, "ADXL345 ON");

  //Asignacion de la tarea para el core 1.
  xTaskCreatePinnedToCore(Task1Core1, "Task1", 10000, NULL, 1, &Task1, 0);     

  //Asignacion de la tarea para el core 1.
  xTaskCreatePinnedToCore(Task2Core1, "Task2", 10000, NULL, 1, &Task2, 0);  
  delay(2000); // Para observar los ON.
}

//Esta funcion actualiza la informacion de los sensores, y la envia a las variables de pantalla.
void getSensores()
{
    //Obtencion de la informacion del termometro, adaptacion y envio hacia las variables de pantalla.
    tempSensor.requestTemperatures();
    char tempchar[9]; char tempmsg[20] = "Temp: ";
    dtostrf(tempSensor.getTempCByIndex(0), 6, 2, tempchar);
    strcat(tempmsg, tempchar);
    strcat(tempmsg, " °C");
    strcpy(l1, tempmsg);

    //Obtencion de la informacion del acelerometro, adaptacion y envio hacia las variables de pantalla.
    int x, y, z;
    char xx[5]; char yy[5]; char zz[5]; char acelmsg[20] = "X:";
    adxl.readAccel(&x, &y, &z);
    itoa(x, xx, 10); itoa(y, yy, 10); itoa(z, zz, 10);
    strcat(acelmsg, xx); strcat(acelmsg, " Y:"); strcat(acelmsg, yy); strcat(acelmsg, " Z:"); strcat(acelmsg, zz); 
    strcpy(l2, acelmsg);

    //Obtencion de la informacion del colorimetro, adaptacion y envio hacia las variables de pantalla.
    uint16_t r, g, b, c, colorTemp, lux;
    tcs.getRawData(&r, &g, &b, &c);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
    //Variables que contendran todo el mensaje.
    char ctemp[21] = "Color Temp: "; 
    char luxx[11] = "Lux: "; 
    char rr[21] = "R:"; char gg[11] = " G:"; char bb[11]=" B:";
    //Variables aux para el itoa.
    char a1[9]; char a2[9]; char a3[9]; char a4[9]; char a5[9];
    itoa(colorTemp, a1, 10); itoa(lux, a2, 10); itoa(r, a3, 10); itoa(g, a4, 10); itoa(b, a5, 10);
    strcat(ctemp, a1); strcat(ctemp, " K"); strcat(luxx, a2); strcat(rr, a3); strcat(gg, a4); strcat(bb, a5);
    strcpy(l3, ctemp); strcpy(l4, luxx); strcat(rr, gg); strcat(rr, bb); strcpy(l5, rr);

    //Obtencion de la informacion del aguametro (?)
    int valorSensor = analogRead(PINAGUA);
    char agua[11] = "Agua: "; char a6[7];
    itoa(valorSensor, a6, 10);
    strcat(agua, a6);
    strcpy(l6, agua);
}

//Esta funcion refresca la pantalla, con la informacion de las variables L.
void actualizarPantalla()
{
  lcd.clear();
  mostrar(0, 0, l1);
  mostrar(0, 1, l2);
  mostrar(0, 2, l3);
  mostrar(0, 3, l4);
  mostrar(0, 4, l5);
  mostrar(0, 5, l6);
}

//Thread1-Core1
void Task1Core1(void* pvParameters)
{
  TickType_t momentoDoInicio;
  momentoDoInicio = xTaskGetTickCount();
  for(;;)
  {
    vTaskDelayUntil(&momentoDoInicio,pdMS_TO_TICKS(1));
    getSensores();
  }
      
}

//Thread2-Core1
void Task2Core1(void* pvParameters)
{
  TickType_t momentoDoInicio;
  momentoDoInicio = xTaskGetTickCount();
  for (;;)
  {
      vTaskDelayUntil(&momentoDoInicio,pdMS_TO_TICKS(10));
      actualizarPantalla();
  }
}

//Funcion para enviar un mensaje a la pantalla.
void mostrar(int x, int y, char* msg)
{
  //lcd.clear();
  lcd.setCursor(x,y);
  lcd.print(msg);
}

//Sin uso por el uso de los dos nucleos.
void loop() 
{
}
