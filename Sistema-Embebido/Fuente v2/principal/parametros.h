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
static PCD8544 lcd = PCD8544(14,13,27,26,15);

//Colorimetro
#include "Adafruit_TCS34725.h"
#include "ColorConverterLib.h"

//Motor - Puente H
#include <analogWrite.h>
const int BIA = 0;
const int BIB = 4;
byte speed = 255;

//C
#include "stdlib.h"
#include "string.h"

//RELOJ
int relojActivado = 0;
unsigned long horaenSegundos = 0;
int segundosActuales = 0;
int minutosActuales = 0;
int horasActuales = 0;
//Por defecto a las 23:59:59 si no especifican.
int segundosProgramadosDispensarCloro = 23;
int minutosProgramadosDispensarCloro = 59;
int horasProgramadosDispensarCloro = 59;
int capacidadPileta = 0; // Capacidad de la pileta introducida por el usuario.
int lluvia = 0; //Si el sensor de lluvia detecta lluvia, esta variable cambia a 1 y viceversa.
int techocerrado = 0; //Si el techo ya fue cerrado no se vuelve a cerrar.
int coordenadas = 0; //Si la ubicacion es en base a las coordenadas del celular esta variable cambia a 1.

//Constantes para mensajes Bluetooth
int const ORDEN_CLORO = 1;
int const ORDEN_MOTOR = 2;
int const ORDEN_WIFIGPS = 3;
int const ORDEN_WIFIGPSCOORD = 5;
int const ORDEN_WIFIGPSCOORDRELOJ = 7;
int const ORDEN_PROGRAMARCLORO = 9;


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
const int PINAGUA = 35;

//Pin del relee (electrovalvula).
const int PINRELE = 23;

//-------VARIABLES GLOBALES SENSORES
double temperatura;
int x, y, z;
uint16_t r, g, b, c, colorTemp, lux;
char nombreColor[13];
int valorSensorAgua;

//Constructor del acelerometro.
ADXL345 adxl = ADXL345();
const int SENSIBILIDAD_ACELEROMETRO = 10;

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

#include "auxiliares.h"
#include "sensores.h"
#include "bluetooth.h"
