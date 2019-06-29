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
  analogWrite(BIA, 0);
  analogWrite(BIB, speed);
  delay(5000); //ESTE DELAY ESTA CORRIENDO SOBRE EL THREAD2-NUCLEO2 QUE SIRVE DE ENTRADAS/SALIDAS NO SOBRE EL THREAD1-NUCLEO1 DE MEDICION DE SALIDAS.
  stopMotor();
}

//Sentido de rotacion del motor. Sentido de Apertura de Techo de Pileta.
void forward()
{
  analogWrite(BIA, speed);
  analogWrite(BIB, 0);
  delay(5000); //ESTE DELAY ESTA CORRIENDO SOBRE EL THREAD2-NUCLEO2 QUE SIRVE DE ENTRADAS/SALIDAS NO SOBRE EL THREAD1-NUCLEO1 DE MEDICION DE SALIDAS.
  stopMotor();
}

//Esta funcion corre sobre el thread 2 (nucleo 2) que se ejecuta cada 1 segundo y actualiza la hora siempre y cuando se haya seteado via bluetooth una hora inicial.
void actualizarHora()
{
  //Si el reloj fue previamente activado por Bluetooth.
  if(relojActivado == 1)
  {
    horaenSegundos += 1;
    __secs_to_tm(horaenSegundos, &horasActuales, &minutosActuales, &segundosActuales);
    char horas[3], minutos[3], segundos[3];
    itoa(horasActuales, horas, 10); itoa(minutosActuales, minutos, 10); itoa(segundosActuales, segundos, 10);
    strcpy(l5, horas); strcat(l5, ":"); strcat(l5, minutos); strcat(l5, ":"); strcat(l5, segundos);
  }
}

//Esta funcion activa el relee de la electrovalvula un determinado tiempo a una hora especificada por el usuario.
void cuandoDispensarCloro()
{
  //Por cada litro que tenga la pileta se anade un segundo de dispensado de cloro que equivale a 0,0016L
  int deltaPorCapacidad = capacidadPileta * 1000;
  if(segundosProgramadosDispensarCloro == segundosActuales && minutosProgramadosDispensarCloro == minutosActuales && horasProgramadosDispensarCloro == horasActuales)
  {
    if(temperatura > 0 && temperatura < 15)
    {
      digitalWrite(PINRELE, HIGH);
      delay(500 + deltaPorCapacidad);
      digitalWrite(PINRELE, LOW);
    }
    else if (temperatura > 15 && temperatura < 20)
    {
      digitalWrite(PINRELE, HIGH);
      delay(1000 + deltaPorCapacidad);
      digitalWrite(PINRELE, LOW);
    }
    else if (temperatura > 20)
    {
      digitalWrite(PINRELE, HIGH);
      delay(1500 + deltaPorCapacidad);
      digitalWrite(PINRELE, LOW);
    }
  }
}

//Simula una llamada a una API del clima que devuelve en base a coordenadas, el nombre de la localizacion y si esta lloviendo o no.
void obtenerClima()
{
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;
    char base[] = "http://191.238.213.18/obtenerClima.php?password=asd1234";
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.begin(base); 
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
          backward(); //Activo el servo para cerrar la pileta (techo).
    }
    else 
    {
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end(); //Free the resources
  }
}
