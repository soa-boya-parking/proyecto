//Tratamiento de los mensajes recibidos por Bluetooth.
void tratarMensajeRecibidoBluetooth(const char* msg)
{
  int const comando = msg[0] - '0';
  char datos[5][30];
  separarPorPuntoYComa(datos, msg); // Al recibir datos EJ: 1;xxxx;xxxxxxx;xxxxxx.
  if(comando == ORDEN_CLORO)
  {
    Serial.println("ENTER CLORO");
    (strcmp(datos[1], "ON") == 0) ? digitalWrite(PINRELE, HIGH) : digitalWrite(PINRELE, LOW);
  } else if(comando == ORDEN_MOTOR){
    Serial.println("ENTER SERVO");
    (strcmp(datos[1], "ON") == 0) ? servoDerecha() : servoIzquierda();
  } else if(comando == ORDEN_WIFI){
    Serial.println(msg);
  } else if(comando == ORDEN_GPS){
    Serial.println(msg);
  } else if (comando == ORDEN_WIFIGPSCOORD){
    WiFi.begin(datos[1], datos[2]);
    Serial.println("Conectando al WIFI");
    delay(4000);
    obtenerClima();
    //coordenadasATexto(datos[3], datos[4]);
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