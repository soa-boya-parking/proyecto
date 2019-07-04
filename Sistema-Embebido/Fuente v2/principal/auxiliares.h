#include "timerC.h"

//Si queda tiempo ver esta opcion.
void coordenadasATexto(const char* lat, const char* longg)
{
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;
    char base[] = "https://api.opencagedata.com/geocode/v1/json?key=6b6a6897b5ad4a56a6d47f16d4908577&q=";
    strcat(base, lat);
    strcat(base, "+");
    strcat(base, longg);
    http.begin(base); 
    int httpResponseCode = http.GET();
    Serial.println(base);
    if (httpResponseCode > 0) {
 
        String payload = http.getString();
        String ciudad = payload.substring(payload.indexOf("state"), payload.indexOf("state_code"));
        ciudad.replace("state", "");
        ciudad.replace(":", "");
        ciudad.replace(",", "");
        ciudad.replace('"', '*');
        ciudad.replace("*", "");
        Serial.println(ciudad); //SEGUIR. EL NOMBRE DE LA CIUDAD IRIA A http://api.apixu.com/v1/current.json?key=35a01956527c4df5941170247190906&q=Buenos%20Aires0
      }
 
    else 
    {
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end(); //Free the resources
  }
}

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

//Funcion para enviar un mensaje a la pantalla.
void mostrar(int x, int y, char* msg)
{
  //lcd.clear();
  lcd.setCursor(x,y);
  lcd.print(msg);
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

//Detencion del motor.
void stopMotor()
{
  analogWrite(BIA, 0);
  analogWrite(BIB, 0);
}

//Sentido de rotacion del motor. Sentido de Cierre de Techo de Pileta.
void backward()
{
  if(techocerrado == 0)
  {
    analogWrite(BIA, 0);
    analogWrite(BIB, speed);
    delay(5000); //ESTE DELAY ESTA CORRIENDO SOBRE EL THREAD2-NUCLEO2 QUE SIRVE DE ENTRADAS/SALIDAS NO SOBRE EL THREAD1-NUCLEO1 DE MEDICION DE SALIDAS.
    stopMotor();
    techocerrado = 1;
  }
}

//Sentido de rotacion del motor. Sentido de Apertura de Techo de Pileta.
void forward()
{
  if(techocerrado == 1)
  {
    analogWrite(BIA, speed);
    analogWrite(BIB, 0);
    delay(5000); //ESTE DELAY ESTA CORRIENDO SOBRE EL THREAD2-NUCLEO2 QUE SIRVE DE ENTRADAS/SALIDAS NO SOBRE EL THREAD1-NUCLEO1 DE MEDICION DE SALIDAS.
    stopMotor();
    techocerrado = 0;
  }
}

//Esta funcion es llamada por el thread2 y comprueba si el sensor de lluvia detecta la presencia de lluvia, cierra el techo (motor).
void llueve()
{
  if(lluvia == 1 && techocerrado == 0)
    backward();
}

//Esta funcion corre sobre el thread 2 (nucleo 2) que se ejecuta cada 1 segundo y actualiza la hora siempre y cuando se haya seteado via bluetooth una hora inicial.
void actualizarHora()
{
  //Si el reloj fue previamente activado por Bluetooth.
  if(relojActivado == 1)
  {
    horaenSegundos += 1;
    __secs_to_tm(horaenSegundos, &horasActuales, &minutosActuales, &segundosActuales);
    char horas[4], minutos[4], segundos[4];
    itoa(horasActuales, horas, 10); itoa(minutosActuales, minutos, 10); itoa(segundosActuales, segundos, 10);
    strcpy(l5, horas); strcat(l5, ":"); strcat(l5, minutos); strcat(l5, ":"); strcat(l5, segundos);
  }
}

//Esta funcion activa el relee de la electrovalvula un determinado tiempo a una hora especificada por el usuario.
void cuandoDispensarCloro()
{
  //Por cada litro que tenga la pileta se anade un segundo de dispensado de cloro que equivale a 0,0016L
  int deltaPorCapacidad = capacidadPileta * 1000;
  //Si estoy dentro del horario a dispensar.
  if(segundosProgramadosDispensarCloro == segundosActuales && minutosProgramadosDispensarCloro == minutosActuales && horasProgramadosDispensarCloro == horasActuales)
  {
    //Si no hay nada en la pileta (humano, objeto, animal).
    if(hayAlgoEnPileta == 0 && concurrenciaPileta[horasActuales] < LIMITE_CONCURRENCIA)
    {
      if(FRIA)
      {
        digitalWrite(PINRELE, HIGH);
        delay(UNSEGUNDO / 2 + deltaPorCapacidad);
        digitalWrite(PINRELE, LOW);
      }
      else if (NORMAL)
      {
        digitalWrite(PINRELE, HIGH);
        delay(UNSEGUNDO + deltaPorCapacidad);
        digitalWrite(PINRELE, LOW);
      }
      else if (CALIENTE)
      {
        digitalWrite(PINRELE, HIGH);
        delay(UNSEGUNDO * 1.5 + deltaPorCapacidad);
        digitalWrite(PINRELE, LOW);
      }
    }
    //Si hay algo en la pileta, registro que en ese horario (hora solamente) hubo una concurrencia.
    else
    {
      concurrenciaPileta[horasActuales] +=1;
      horasProgramadosDispensarCloro = (horasProgramadosDispensarCloro + 1) % 24;
    }
  }
}

//Simula una llamada a una API del clima que devuelve en base a coordenadas, el nombre de la localizacion y si esta lloviendo o no.
void obtenerClima()
{
  if (coordenadas == 1) return;
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.begin(WIFI_API_CLIMA); 
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) 
    {
        String payload = http.getString(); //Todo el string que recibo.
        char datos[3][30]; //Variable auxiliar.
        char aux[100]; // Esta variable auxiliar es porque el request trabaja con un String C++ y el metodo separarPorPuntoYComa con un char*
        payload.toCharArray(aux, 99);
        separarPorPuntoYComa(datos, aux);
        strcpy(l6, datos[0]); // Exhibo la ubicacion por pantalla.
        strcat(l6, datos[1]); // Exhibo el clima por pantalla.
        if(strcmp(datos[1], "Lluvia") == 0) //Si la llamada a la API del clima detecta lluvia.
          lluvia = 1;
    }
    else 
    {
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end(); //Free the resources
  }
}
