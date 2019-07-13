#include "parametros.h"

//Inicializaciones.
void setup() 
{
  //-------INICIALIZACION DE PANTALLA.
  gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_23, 1);
  lcd.begin(PANTALLA_COLUMNAS, PANTALLA_FILAS);
  lcd.clear();
  lcd.setContrast(PANTALLA_CONTRASTE);
  
  //Velocidad puerto serial.
  Serial.begin(SERIE_VELOCIDAD);

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

  //-------INICIALIZACION DEL BUZZER.
  pinMode(PINBUZZER, OUTPUT);

  //-------INICIALIZACION DE LOS PINES DEL PUENTE H(MOTOR).
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
  mostrar(0, 3, "Puente H ON");

  //-------INICIALIZACION DE LOS PINES DEL SENSOR DE AGUA.
  pinMode(PINAGUA, INPUT);
  mostrar(0, 4, "FC-37 ON");

  //-------INICIALIZACION DE LOS PINES DEL RELEE.
  pinMode(PINRELE, OUTPUT);
  digitalWrite(PINRELE, LOW);
  mostrar(0, 5, "Valvula ON");
  

  //-------INICIALIZACION DEL BLUETOOTH.
  // Inicializacion del BLE con nombre.
  BLEDevice::init("ESP32");

  // Creacion del servidor BLE.
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new Servidor());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX,BLECharacteristic::PROPERTY_NOTIFY);          
  pTxCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new Cliente());
  pService->start();
  pServer->getAdvertising()->start();
  mostrar(0, 5, "Bluetooth ON");

  adaptacionColor(r, g, b, c);
  
  //Asignacion de la tarea para el core 1. Recoleccion de datos de sensores.
  xTaskCreatePinnedToCore(Task1Core1, "Task1", STACK_SIZE_10000, NULL, 1, &Task1, X_CORE_ID_0);     

  //Asignacion de la tarea para el core 2. Entradas y salidas pantalla, bluetooth, wifi.
  xTaskCreatePinnedToCore(Task2Core1, "Task2", STACK_SIZE_5000, NULL, 1, &Task2, X_CORE_ID_1);  

  //La primera linea de la pantalla es el nombre del proyecto.
  //strcpy(l1, "Boya-Parking");
  //Delay de cinco segundos para mostra los mensajes de inicializacion.
  delay(5000);
}

//Esta funcion actualiza la informacion de los sensores, y la envia a las variables de pantalla.
void getSensores()
{
    //TEMPERATURA
    
    //Obtencion de la informacion del termometro, adaptacion y envio hacia las variables de pantalla.
    tempSensor.requestTemperatures();
    char tempchar[9]; char tempmsg[20] = "Temp: ";
    temperatura = tempSensor.getTempCByIndex(0);
    dtostrf(temperatura, 6, 2, tempchar);
    strcat(tempmsg, tempchar);
    strcat(tempmsg, " °C");
    strcpy(l1, tempmsg);
    
    //ACELEROMETRO
    
    //Obtencion de la informacion del acelerometro, adaptacion y envio hacia las variables de pantalla.
    char xx[5]; char yy[5]; char zz[5]; char acelmsg[20] = "X:";
    adxl.readAccel(&x, &y, &z);

    //COLORIMETRO

    //Obtencion de la informacion del colorimetro, adaptacion y envio hacia las variables de pantalla.
    tcs.getRawData(&r, &g, &b, &c);
    //Se calcula el colorTemp y lux en base a los parametros anteriores r, g, b.
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
    //Se calcula la suciedad mediante pruebas caseras realizadas, teniendo en cuenta el colorTemp y lux.
    int suciedad = corroborarSuciedad(colorTemp, lux);
    if(suciedad == COLORIMETRO_ESTADO_AGUA_LIMPIA)
      strcpy(l2, "Estado: Limpia");
    else if(suciedad == COLORIMETRO_ESTADO_AGUA_ALGOSUCIA)
      strcpy(l2, "Estado: Normal"); //No alcanzan los caracteres en la pantalla para algo sucia
    else if(suciedad == COLORIMETRO_ESTADO_AGUA_SUCIA)
      strcpy(l2, "Estado: Sucia");  
    adaptacionColor(r, g, b, c);

    //SENSOR AGUA

    //Obtencion de la informacion del sensor de agua.
    valorSensorAgua = analogRead(PINAGUA);
    char agua[11] = "Agua: "; char a6[7];
    if(valorSensorAgua > SENSOR_LLUVIA_TOLERANCIA_SECO_LLOVIZNA)
      strcpy(l3, "Seco");
    else if(valorSensorAgua > SENSOR_LLUVIA_TOLERANCIA_LLOVIZNA_LLUVIA)
      strcpy(l3, "Llovizna");
    else
      strcpy(l3, "Lluvia");

    //BLUETOOTH PREPARACION + LOGICA INDEPENDIENTE

    //Preparacion de los datos por Bluetooth. (NO ENVIO)
    strcpy(datosBluetooth, tempchar);
    strcat(datosBluetooth, ";");
    
    if(suciedad == COLORIMETRO_ESTADO_AGUA_LIMPIA)
      strcat(datosBluetooth, "Limpia");
    else if(suciedad == COLORIMETRO_ESTADO_AGUA_ALGOSUCIA)
      strcat(datosBluetooth, "Algo Sucia");
    else if(suciedad == COLORIMETRO_ESTADO_AGUA_SUCIA)
      strcat(datosBluetooth, "Sucia");
      
    strcat(datosBluetooth, ";");
    
    if(valorSensorAgua > SENSOR_LLUVIA_TOLERANCIA_SECO_LLOVIZNA)
    {
      strcat(datosBluetooth, "Seco");
      lluvia = 0; //Esta variable es leida por el thread aparte de los sensores que determina si cerrar o no el techo.
    } 
    else if(valorSensorAgua > SENSOR_LLUVIA_TOLERANCIA_LLOVIZNA_LLUVIA)
    {
      strcat(datosBluetooth, "Llovizna");
      lluvia = 1;
    }
    else
    {
      strcat(datosBluetooth, "Lluvia");
      lluvia = 1;
    }
    
    strcat(datosBluetooth, ";");
    
    if(abs(x+y+z) - abs(xyz) > SENSIBILIDAD_ACELEROMETRO)
    {
      strcat(datosBluetooth, "ALERTA");
      hayAlgoEnPileta = 1;
    }
    else
    {
      strcat(datosBluetooth, "NADA");
      hayAlgoEnPileta = 0;
    }
    strcat(datosBluetooth, ";");
    strcat(datosBluetooth, nombreColor);
    strcat(datosBluetooth, ";");
    //Si el horario seleccionado por el usuario es muy concurrido en base a dias pasados para dispensar cloro.
    if(errorPorConcurrencia == 1)
    {
      strcat(datosBluetooth, "NOK");
      errorPorConcurrencia = 0;
    }
    else
      strcat(datosBluetooth, "OK");
    strcat(datosBluetooth, ";");
    xyz = x+y+z;
}

//Thread1-Core1
void Task1Core1(void* pvParameters)
{
  for(;;)
  {
    getSensores();
  }
      
}

//Thread2-Core2
void Task2Core1(void* pvParameters)
{
  TickType_t momentoDoInicio;
  momentoDoInicio = xTaskGetTickCount();
  for (;;)
  {
      vTaskDelayUntil(&momentoDoInicio,pdMS_TO_TICKS(1000));
      actualizarPantalla();
      enviarDatosBluetooth();
      actualizarHora();
      cuandoDispensarCloro();
      cuandoSonar();
      llueve();
      obtenerClima();
  }
}

//Sin uso por el uso de los dos nucleos.
void loop() 
{
}
