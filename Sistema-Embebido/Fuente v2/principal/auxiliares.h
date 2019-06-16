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

void servoDerecha()
{
  for (pos = 0; pos <= 180; pos += 1) 
  {
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
}

void servoIzquierda()
{
  for (pos = 180; pos >= 0; pos -= 1) 
  {
    servo.write(pos);
    delay(5);
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
        strcpy(l5, datos[0]); // Exhibo la ubicacion por pantalla.
        strcpy(l6, datos[1]); // Exhibo el clima por pantalla.
        if(strcmp(datos[1], "Lluvia") == 0) //Si la llamada a la API del clima detecta lluvia.
          servoDerecha(); //Activo el servo para cerrar la pileta (lona).
    }
    else 
    {
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end(); //Free the resources
  }
}
