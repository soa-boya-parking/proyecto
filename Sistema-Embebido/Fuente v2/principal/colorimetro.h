//https://www.luisllamas.es/arduino-sensor-color-rgb-tcs34725/
void adaptacionColor(uint16_t red, uint16_t green, uint16_t blue, uint16_t c)
{
  uint32_t sum = c;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
 
  // Escalar rgb a bytes
  r *= 256; g *= 256; b *= 256;

  double hue, saturation, value;
  ColorConverter::RgbToHsv(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), hue, saturation, value);
  hue*=360;
  if (hue < 15)
    strcpy(nombreColor, "Rojo");
  else if (hue < 45)
    strcpy(nombreColor, "Naranja");
  else if (hue < 90)
    strcpy(nombreColor, "Amarillo");
  else if (hue < 150)
    strcpy(nombreColor, "Verde");
  else if (hue < 210)
    strcpy(nombreColor, "Cyan");
  else if (hue < 270)
    strcpy(nombreColor, "Azul");
  else if (hue < 330)
    strcpy(nombreColor, "Magenta");
  else
    strcpy(nombreColor, "Rojo");
}

//Metodo en base a pruebas y utilizando solo dos variables (sin tomar en cuenta el color)
int corroborarSuciedad(uint16_t colorTemp, uint16_t lux)
{
  if(colorTemp < COLORIMETRO_TOLERANCIA_COLOR_TEMP_LIMPIA && lux > COLORIMETRO_TOLERANCIA_COLOR_LUX_LIMPIA)
    return COLORIMETRO_ESTADO_AGUA_LIMPIA;
  else if(colorTemp < COLORIMETRO_TOLERANCIA_COLOR_TEMP_ALGOSUCIA && lux > COLORIMETRO_TOLERANCIA_COLOR_LUX_ALGOSUCIA)
    return COLORIMETRO_ESTADO_AGUA_ALGOSUCIA;
  else
    return COLORIMETRO_ESTADO_AGUA_SUCIA;
}
