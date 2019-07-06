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
static PCD8544 lcd = PCD8544(14,13,27,26,15); //Pines SPI de la pantalla.

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
int const UNSEGUNDO = 1000;

//Constantes para mensajes Bluetooth
int const BLUETOOTH_LONGITUD_MENSAJE = 51;
int const ORDEN_CLORO = 1;
int const ORDEN_MOTOR = 2;
int const ORDEN_WIFIGPS = 3;
int const ORDEN_WIFIGPSCOORD = 5;
int const ORDEN_WIFIGPSCOORDRELOJ = 7;
int const ORDEN_PROGRAMARCLORO = 9;

//Constantes para el puerto serie.
int const SERIE_VELOCIDAD = 115200;

//Constantes para la pantalla.
int const PANTALLA_CONTRASTE = 50;
int const PANTALLA_COLUMNAS = 84;
int const PANTALLA_FILAS = 48;
int const PANTALLA_CARACTERES_POR_LINEA = 21;

//Constantes para la multitarea.
int const STACK_SIZE_10000 = 10000;
int const STACK_SIZE_5000 = 5000;
int const X_CORE_ID_1 = 1;
int const X_CORE_ID_0 = 0;

//Constantes para el sensor de lluvia.
int const SENSOR_LLUVIA_TOLERANCIA_SECO_LLOVIZNA = 4000;
int const SENSOR_LLUVIA_TOLERANCIA_LLOVIZNA_LLUVIA = 2500;

//Constantes para el colorimetro.
const int COLORIMETRO_ESTADO_AGUA_LIMPIA = 1;
const int COLORIMETRO_ESTADO_AGUA_ALGOSUCIA = 2;
const int COLORIMETRO_ESTADO_AGUA_SUCIA = 3;
const int COLORIMETRO_TOLERANCIA_COLOR_TEMP_LIMPIA = 12500;
const int COLORIMETRO_TOLERANCIA_COLOR_TEMP_ALGOSUCIA = 14500;
const int COLORIMETRO_TOLERANCIA_COLOR_LUX_LIMPIA = 12500;
const int COLORIMETRO_TOLERANCIA_COLOR_LUX_ALGOSUCIA = 12500;

//Constantes / Macros para el dispensado de cloro.
const int LIMITE_CONCURRENCIA = 3; //Si en 3 dias seguidos a la hora del dispensado de cloro la pileta estaba concurrida, ya no se podra seleccionar ese horario.
int errorPorConcurrencia = 0; //Si esta variable esta en una, el usuario debe seleccionar otro horario para el dispensado de cloro.
int concurrenciaPileta[24]= {0}; //Este vector guarda si para cada hora del dia, los anteriores dias estuvo muy concurrido a la hora de dispensar cloro.
#define FRIA (temperatura > 0 && temperatura < 15)
#define NORMAL (temperatura > 15 && temperatura < 20)
#define CALIENTE (temperatura > 20)

//-------CONFIGURACIONES GLOBALES (VARIABLES)

//Parametros de la conexion wifi.
char ssid[50] = "";
char password[50] = "";
const int WIFI_DELAY = 4000;
const char* WIFI_API_CLIMA = "http://191.238.213.18/obtenerClima.php?password=asd1234";

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
int hayAlgoEnPileta = 0;
char nombreColor[13];
int valorSensorAgua;

//Constructor del acelerometro.
ADXL345 adxl = ADXL345();
const int SENSIBILIDAD_ACELEROMETRO = 5;

//Tareas, concepto similar a Thread, el ESP32 tiene 2 nucleos, por lo tanto puedo tener threads en mas de un nucleo.
TaskHandle_t Task1;
TaskHandle_t Task2;

//Constructor del termometro.
OneWire oneWire(PINTEMPERATURA);
DallasTemperature tempSensor(&oneWire);

//Constructor del colorimetro.
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

//Variables que corresponden a las lineas de la pantalla.
char l1[PANTALLA_CARACTERES_POR_LINEA] = "";
char l2[PANTALLA_CARACTERES_POR_LINEA] = "";
char l3[PANTALLA_CARACTERES_POR_LINEA] = "";
char l4[PANTALLA_CARACTERES_POR_LINEA] = "";
char l5[PANTALLA_CARACTERES_POR_LINEA] = "";
char l6[PANTALLA_CARACTERES_POR_LINEA] = "";

//Variable con los datos que se van a enviar por Bluetooth.
char datosBluetooth[BLUETOOTH_LONGITUD_MENSAJE] = "";

//Deteccion movimiento.
int xyz;

#include "auxiliares.h"
#include "colorimetro.h"
#include "bluetooth.h"
