//-------LIBRERIAS

//Wifi y HTTPClient
#include "WiFi.h"
#include "HTTPClient.h"

//Bluetooth
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

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
#include "ColorConverterLib.h"

//C
#include "stdlib.h"
#include "string.h"

//Constantes para mensajes Bluetooth
int const ORDEN_CLORO = 1;
int const ORDEN_MOTOR = 2;
int const ORDEN_WIFI = 3;
int const ORDEN_GPS = 4;
int const ORDEN_GPSAUTO = 5;


//-------CONFIGURACIONES GLOBALES

//Parametros de la conexion wifi.
char* ssid = "JORGE 1";
char* password = "ONICONARO1";

//Bluetooth
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

//Estos numeros se frutean. https://www.uuidgenerator.net/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//Pines de sensores analogicos.
const int PINTEMPERATURA = 32;
const int PINAGUA = 25;

//-------VARIABLES GLOBALES SENSORES
double temperatura;
int x, y, z;
uint16_t r, g, b, c, colorTemp, lux;
char nombreColor[13];
int valorSensorAgua;

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

//Variables que corresponden a las lineas de la pantalla.
char l1[21] = "";
char l2[21] = "";
char l3[21] = "";
char l4[21] = "";
char l5[21] = "";
char l6[21] = "";

//Variable con los datos que se van a enviar por Bluetooth.
char datosBluetooth[51] = "";

//Deteccion movimiento beta jaja.
int xyz;

void separarPorPuntoYComa(char datos[][30], const char* msg)
{
    int i = 0;
    char *pt;
    pt = strtok((char*)msg,";");
    while (pt != NULL) 
    {
        strcpy(datos[i], pt);
        pt = strtok (NULL, ";");
        i++;
    }
}

//Tratamiento de los mensajes recibidos por Bluetooth.
void tratarMensajeRecibidoBluetooth(const char* msg)
{
  int const comando = msg[0] - '0';
  char datos[5][30];
  separarPorPuntoYComa(datos, msg); // Al recibir datos EJ: 1;xxxx;xxxxxxx;xxxxxx.
  if(comando == ORDEN_CLORO)
  {
    Serial.println(msg);
    strcpy(l6, &msg[2]);
  } else if(comando == ORDEN_MOTOR){
    Serial.println(msg);
  } else if(comando == ORDEN_WIFI){
    Serial.println(msg);
  } else if(comando == ORDEN_GPS){
    Serial.println(msg);
  } else if (comando == ORDEN_GPSAUTO){
    WiFi.begin(datos[1], datos[2]);
    Serial.println("Wifi conectado");
    //Ver esto https://opencagedata.com/ para pasar de latitud longitud a texto.
  }
  
}

//Clase para el envío de datos por bluetooth.
class Servidor: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) 
    {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) 
    {
      deviceConnected = false;
    }
};

//Clase para la recepción de datos por bluetooth.
class Cliente: public BLECharacteristicCallbacks 
{
    void onWrite(BLECharacteristic *pCharacteristic) 
    {
      //El valor a recibir es un string, si mando otro valor que no sea string aparece raro.
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) 
      {
        //Serial.print("Valor recibido: ");
        const char* valor = rxValue.c_str();
        tratarMensajeRecibidoBluetooth(valor);
      }
    }
};

//Inicializaciones.
void setup() 
{
  //-------INICIALIZACION DE PANTALLA.
  gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_23, 1);
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

  //-------INICIALIZACION DEL BLUETOOTH.
  // Inicializacion del BLE con nombre.
  BLEDevice::init("ESP32");

  // Creacion del servidor.
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new Servidor());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX,BLECharacteristic::PROPERTY_NOTIFY);          
  pTxCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new Cliente());
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Esperando la conexion de un cliente.");

  adaptacionColor(r, g, b, c);
  
  //Asignacion de la tarea para el core 1.
  xTaskCreatePinnedToCore(Task1Core1, "Task1", 10000, NULL, 1, &Task1, 0);     

  //Asignacion de la tarea para el core 1.
  xTaskCreatePinnedToCore(Task2Core1, "Task2", 10000, NULL, 1, &Task2, 0);  
  delay(2000); // Para observar los ON.
}

//https://www.luisllamas.es/arduino-sensor-color-rgb-tcs34725/
void adaptacionColor(uint16_t red, uint16_t green, uint16_t blue, uint16_t c)
{
  uint32_t sum = c;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
 
  // Escalar rgb a bytes
  r *= 256; g *= 256; b *= 256;

  double hue, saturation, value;
  ColorConverter::RgbToHsv(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), hue, saturation, value);
  hue*=360;
  if (hue < 15)
    strcpy(nombreColor, "Rojo");
  else if (hue < 45)
    strcpy(nombreColor, "Naranja");
  else if (hue < 90)
    strcpy(nombreColor, "Amarillo");
  else if (hue < 150)
    strcpy(nombreColor, "Verde");
  else if (hue < 210)
    strcpy(nombreColor, "Cyan");
  else if (hue < 270)
    strcpy(nombreColor, "Azul");
  else if (hue < 330)
    strcpy(nombreColor, "Magenta");
  else
    strcpy(nombreColor, "Rojo");
  //Serial.println(hue);
}

//Esta funcion actualiza la informacion de los sensores, y la envia a las variables de pantalla.
void getSensores()
{
    //Obtencion de la informacion del termometro, adaptacion y envio hacia las variables de pantalla.
    tempSensor.requestTemperatures();
    char tempchar[9]; char tempmsg[20] = "Temp: ";
    temperatura = tempSensor.getTempCByIndex(0);
    dtostrf(temperatura, 6, 2, tempchar);
    strcat(tempmsg, tempchar);
    strcat(tempmsg, " °C");
    strcpy(l1, tempmsg);
    Serial.println(tempmsg);
    
    //Obtencion de la informacion del acelerometro, adaptacion y envio hacia las variables de pantalla.
    char xx[5]; char yy[5]; char zz[5]; char acelmsg[20] = "X:";
    adxl.readAccel(&x, &y, &z);
    itoa(x, xx, 10); itoa(y, yy, 10); itoa(z, zz, 10);
    strcat(acelmsg, xx); strcat(acelmsg, " Y:"); strcat(acelmsg, yy); strcat(acelmsg, " Z:"); strcat(acelmsg, zz); 
    strcpy(l2, acelmsg);
    Serial.println(acelmsg);


    //Obtencion de la informacion del colorimetro, adaptacion y envio hacia las variables de pantalla.
    tcs.getRawData(&r, &g, &b, &c);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);

    adaptacionColor(r, g, b, c);
    
    //Variables que contendran todo el mensaje.
    char ctemp[21] = "Color Temp: "; 
    char luxx[11] = "Lux: "; 
    char rr[21] = "R:"; char gg[11] = " G:"; char bb[11]=" B:";
    //Variables aux para el itoa.
    char a1[9]; char a2[9]; char a3[9]; char a4[9]; char a5[9];
    itoa(colorTemp, a1, 10); itoa(lux, a2, 10); itoa(r, a3, 10); itoa(g, a4, 10); itoa(b, a5, 10);
    strcat(ctemp, a1); strcat(ctemp, " K"); strcat(luxx, a2); strcat(rr, a3); strcat(gg, a4); strcat(bb, a5);
    strcpy(l3, ctemp); strcpy(l4, luxx); strcat(rr, gg); strcat(rr, bb); /*strcpy(l5, rr);*/ strcpy(l5, nombreColor);
    Serial.println(ctemp);
    Serial.println(luxx);
    
    
    //Obtencion de la informacion del aguametro (?)
    valorSensorAgua = analogRead(PINAGUA);
    char agua[11] = "Agua: "; char a6[7];
    itoa(valorSensorAgua, a6, 10);
    strcat(agua, a6);
    //strcpy(l6, agua);
    Serial.println(agua);
    
    //Envio de los datos por Bluetooth.
    strcpy(datosBluetooth, tempchar);
    strcat(datosBluetooth, ";");
    (lux>8) ? strcat(datosBluetooth, "Sucia") : strcat(datosBluetooth, "Limpia");
    strcat(datosBluetooth, ";");
    strcat(datosBluetooth, a6);
    strcat(datosBluetooth, ";");
    (abs(x+y+z) - abs(xyz) > 2) ? strcat(datosBluetooth, "ALERTA") : strcat(datosBluetooth, "NADA");
    strcat(datosBluetooth, ";");
    xyz = x+y+z;
}

//Esta funcion envia los datos por Bluetooth hacia el celular.
void enviarDatosBluetooth()
{
   //Aca tengo que definir cada CUANTO TIEMPO le envio CIERTOS DATOS al dispositivo conectado.
    //Si el dispositivo esta conectado.
    if (deviceConnected) 
    {
        //Serial.println("Dispositivo conectado"); Como se repite cada 10 segundos spamea la pantalla.
        pTxCharacteristic->setValue((unsigned char*)datosBluetooth, sizeof(datosBluetooth));
        pTxCharacteristic->notify(); // Se lo mando.
    }

    //Si el dispositivo se desconecta.
    if (!deviceConnected && oldDeviceConnected) 
    {
        pServer->startAdvertising(); // restart advertising
        Serial.println("Dispositivo desconectado");
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {oldDeviceConnected = deviceConnected;}
}

void enviarDatosWifi()
{
  char request[120] = "";
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;
    //Envio de datos del termometro.
    http.begin("http://191.238.213.18/termometro.php"); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    char tempchar[9]; dtostrf(temperatura, 6, 2, tempchar);
    strcpy(request, "password=asd1234&temperatura="); strcat(request, tempchar);
    int httpResponseCode = http.POST(request);
    http.end();
    
    //Envio de datos del acelerometro.
    http.begin("http://191.238.213.18/acelerometro.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    char xx[5]; char yy[5]; char zz[5];
    strcpy(request, "password=asd1234&x="); strcat(request, tempchar);
    itoa(x, xx, 10); itoa(y, yy, 10); itoa(z, zz, 10);
    strcat(request, xx); strcat(request, "&y="); strcat(request, yy); strcat(request, "&z="); strcat(request, zz); 
    httpResponseCode = http.POST(request);
    http.end();
    
    //Envio de datos del colorimetro.
    http.begin("http://191.238.213.18/colorimetro.php"); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    char a1[9]; char a2[9]; char a3[9]; char a4[9]; char a5[9];
    itoa(colorTemp, a1, 10); itoa(lux, a2, 10); itoa(r, a3, 10); itoa(g, a4, 10); itoa(b, a5, 10);
    strcpy(request, "password=asd1234&temperatura="); strcat(request, a1); strcat(request, "&luz=");
    strcat(request, a2); strcat(request, "&r="); strcat(request, a3); strcat(request, "&g="); strcat(request, a4); strcat(request, "&b="); strcat(request, a5);
    httpResponseCode = http.POST(request);
    http.end();
    
    //Envio de datos del sensor de lluvia.
    http.begin("http://191.238.213.18/lluvia.php"); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    char aguachar[9]; itoa(valorSensorAgua, aguachar, 10);
    strcpy(request, "password=asd1234&valor="); strcat(request, aguachar);
    httpResponseCode = http.POST(request);
    http.end();
    
  }
  else
    Serial.println("Error WIFI");
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
      enviarDatosBluetooth();
      //enviarDatosWifi();
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
