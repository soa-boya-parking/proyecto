# Proyecto Sistemas Operativos Avanzados 1C 2019 ![Inch-CI](https://inch-ci.org/assets/badge-example-b71f9e833318f66f64b3f23877113051.svg)

### Integrantes

- Ezequiel Laurenti - 39245012
- Hector Rojas Stoll - 94448064
- Juan Jose Tocino - 38617339
- Damian Vinci - 30603450


## Proyecto Pileta ![Inch-CI](https://img.shields.io/badge/Proyecto-Actual-brigthgreen.svg)  

El proyecto consiste en un administrador de pileta inteligente, que brindará lo siguiente:
- Detección del ingreso de un elemento a la misma y alerta del suceso.  
Esto es útil, por ejemplo, para detectar la caída accidental de una mascota a la pileta.  
- Detección de las condiciones de la pileta (temperatura) y por consiguiente dispensar la cantidad de cloro necesario
- Detección de lluvia:
  - De forma anticipada: Mediante la consulta a un servicio climatológico.
  - De forma nativa: Mediante la detección de gotas de lluvia.  
  En ambos casos será para activar un motor que despliegue una lona sobre la pileta.
- Detección de suciedad de la pileta.  

En cualquiera de los casos anteriores, se notificará mediante algunas de las conexiones disponibles (WiFi / Bluetooth)   
del suceso ocurrido.  
De cualquier manera, el usuario podrá configurar distintos parámetros manualmente desde su celular para modificar el funcionamiento, por ejemplo, la cantidad de cloro dispensada dependiendo de la cantidad de personas en la pileta.

## Especificaciones Técnicas ![Inch-CI](https://img.shields.io/badge/Proyecto-Especificaciones-blue.svg)

**Las herramientas y tecnologías a utilizar serán:**

- SoC SparkFun ESP32
- BLE (Bluetooth Low Energy)
- Wifi (802.11 b/g/n/e/i WLAN)
- iPhone (con iOS 12.2)

**Sensores:**
 - Sensor de temperatura (DS18B20) A Prueba de agua.
 - Sensor de lluvia (FC-37 / YL-83).
 - Sensor de color (TCS3200 / TCS34725).
 - Acelerómetro (ADXL335).
 
 **Actuadores:**
  - Valvula Solenoide.
  - Buzzer.
  - Motor.

## Cerradura Electrónica  ![Inch-CI](https://img.shields.io/badge/Proyecto-Descartado-red.svg)

El proyecto consiste en el control de las puertas de un hogar / establecimiento sin la utilización de llaves físicas.  
Entre las principales funciones del control de las puertas del hogar se encuentran:
- Detección del estado de la/s puerta/s (abierta - cerrada) 
- Apertura y/o cierre de la/s puerta/s 
- Programación del cierre y/o apertura de la/s puerta/s a un determinado horario.
- Apertura de la puerta por proximidad mediante un dispositivo asociado.
- Informe sobre un intento no autorizado y/o violento.

Todas las acciones anteriores serán accedidas mediante un dispositivo asociado (celular)   
de forma inalámbricamente (Wifi y Bluetooth)
