#include "parametros.h"

//Inicializaciones.
void setup() 
{
  //-------INICIALIZACION DE PANTALLA.
  gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_23, 1);
  lcd.begin(84, 48);
  lcd.clear();
  lcd.setContrast(50);
  
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

  //-------INICIALIZACION DE SERVO.
  servo.attach(17);

  //-------INICIALIZACION DEL SENSOR DE AGUA.
  pinMode(PINAGUA, INPUT);

  //-------INICIALIZACION DE RELEE
  pinMode(PINRELE, OUTPUT);
  digitalWrite(PINRELE, LOW);

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
  xTaskCreatePinnedToCore(Task2Core1, "Task2", 5000, NULL, 1, &Task2, 0);  

  strcpy(l1, "Boya-Parking");
  delay(2000); // Para observar los ON.
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
    strcpy(l2, tempmsg);
    

    //Obtencion de la informacion del acelerometro, adaptacion y envio hacia las variables de pantalla.
    char xx[5]; char yy[5]; char zz[5]; char acelmsg[20] = "X:";
    adxl.readAccel(&x, &y, &z);
    /*itoa(x, xx, 10); itoa(y, yy, 10); itoa(z, zz, 10);
    strcat(acelmsg, xx); strcat(acelmsg, " Y:"); strcat(acelmsg, yy); strcat(acelmsg, " Z:"); strcat(acelmsg, zz); 
    strcpy(l2, acelmsg);*/


    //Obtencion de la informacion del colorimetro, adaptacion y envio hacia las variables de pantalla.
    tcs.getRawData(&r, &g, &b, &c);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
    int suciedad = corroborarSuciedad(colorTemp, lux);
    if(suciedad == LIMPIA)
      strcpy(l3, "Estado: Limpia");
    else if(suciedad == ALGOSUCIA)
      strcpy(l3, "Estado: Normal"); //No me alcanzan los caracteres en la pantalla para algo sucia
      
    else if(suciedad == SUCIA)
      strcpy(l3, "Estado: Sucia");
      
  
    adaptacionColor(r, g, b, c);
    
    //Variables que contendran todo el mensaje.
    /*char ctemp[21] = "Color Temp: "; 
    char luxx[11] = "Lux: "; 
    char rr[21] = "R:"; char gg[11] = " G:"; char bb[11]=" B:";
    //Variables aux para el itoa.
    char a1[9]; char a2[9]; char a3[9]; char a4[9]; char a5[9];
    itoa(colorTemp, a1, 10); itoa(lux, a2, 10); itoa(r, a3, 10); itoa(g, a4, 10); itoa(b, a5, 10);
    strcat(ctemp, a1); strcat(ctemp, " K"); strcat(luxx, a2); strcat(rr, a3); strcat(gg, a4); strcat(bb, a5);
    strcpy(l3, ctemp); strcpy(l4, luxx); strcat(rr, gg); strcat(rr, bb); /*strcpy(l5, rr); strcpy(l5, nombreColor);*/

    //Obtencion de la informacion del aguametro (?)
    valorSensorAgua = analogRead(PINAGUA);
    char agua[11] = "Agua: "; char a6[7];
    if(valorSensorAgua > 4000)
      strcpy(l4, "Seco");
    else if(valorSensorAgua > 2500)
      strcpy(l4, "Llovizna");
    else
      strcpy(l4, "Lluvia");
    /*itoa(valorSensorAgua, a6, 10);
    strcat(agua, a6);
    strcpy(l6, agua);*/

    //Preparacion de los datos por Bluetooth. (NO ENVIO)
    strcpy(datosBluetooth, tempchar);
    strcat(datosBluetooth, ";");
    
    if(suciedad == LIMPIA)
      strcat(datosBluetooth, "Limpia");
    else if(suciedad == ALGOSUCIA)
      strcat(datosBluetooth, "Algo Sucia");
    else if(suciedad == SUCIA)
      strcat(datosBluetooth, "Sucia");
      
    strcat(datosBluetooth, ";");
    
    if(valorSensorAgua > 4000)
      strcat(datosBluetooth, "Seco");
    else if(valorSensorAgua > 2500)
      strcat(datosBluetooth, "Llovizna");
    else
      strcat(datosBluetooth, "Lluvia");
    
    strcat(datosBluetooth, ";");
    
    (abs(x+y+z) - abs(xyz) > SENSIBILIDAD_ACELEROMETRO) ? strcat(datosBluetooth, "ALERTA") : strcat(datosBluetooth, "NADA");
    strcat(datosBluetooth, ";");
    strcat(datosBluetooth, nombreColor);
    strcat(datosBluetooth, ";");
    xyz = x+y+z;
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

//Thread1-Core1
void Task1Core1(void* pvParameters)
{
  TickType_t momentoDoInicio;
  momentoDoInicio = xTaskGetTickCount();
  for(;;)
  {
    vTaskDelayUntil(&momentoDoInicio,pdMS_TO_TICKS(10));
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

//Sin uso por el uso de los dos nucleos.
void loop() 
{
}