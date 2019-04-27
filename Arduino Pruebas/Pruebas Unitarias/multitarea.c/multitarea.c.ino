#include "WiFi.h"
#include "HTTPClient.h"

//Tareas que se ejecutaran en nucleos diferentes.
TaskHandle_t Task1;
TaskHandle_t Task2;

//Parametros de la conexion wifi.
const char* ssid = "JORGE 1";
const char* password = "ONICONARO1";

void setup() {

  //Conexion a la red WIFI.
  
  WiFi.disconnect(true);
  Serial.begin(115200); 
  WiFi.begin(ssid, password);

  Serial.println("Conectando");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Conectado!");

  //Asignacion de la tarea para el core 1.
  
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //Asignacion de la tarea para el core 2.
  
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 
}

void informarEstado()
{
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("http://206.189.213.239/"); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");         

    int httpResponseCode = http.POST("usuario=1&password=CVjt49kOENnxNAFVpfv7&estado=1");   

    if(httpResponseCode>0)
    {
      String response = http.getString();   
      Serial.println(httpResponseCode);
      Serial.println(response);          
    }
    else
    {
      Serial.print("Error on sending POST Request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else
  {
    Serial.println("Error in WiFi connection");
  }
}

//Tarea que manda el estado del ESP32.
void Task1code( void * pvParameters )
{
  //Serial.print("Task1 running on core ");
  //Serial.println(xPortGetCoreID());

  for(;;)
  {
    informarEstado();
    delay(10000);
  } 
}

//Tarea de prueba por ahora.
void Task2code( void * pvParameters )
{
  //Serial.print("Task2 running on core ");
  //Serial.println(xPortGetCoreID());

  for(;;)
  {
    Serial.println("Ok Core!");
    delay(5000);
  }
}

void loop() 
{
}
