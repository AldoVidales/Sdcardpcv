#include "DHT.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Definir el pin donde está conectado el DHT11
#define DHTPIN 15  // Cambia este pin si conectaste a otro
#define SD_CS 5    // Definir el pin CS de la tarjeta SD

// Pantalla OLED
#define SCREEN_WIDTH 128  // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 64  // Alto de la pantalla OLED
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RTC
RTC_DS3231 rtc;

// Definir el tipo de sensor DHT (DHT11 o DHT22)
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

// Variables globales
String nombreArchivoActual = "";
int minutoAnterior = -1;  // Para guardar el minuto de la última lectura
int horaAnterior = -1;    // Para controlar la creación de nuevos archivos por hora

void setup() {
  Serial.begin(115200);

  // Inicializar el RTC
  if (!rtc.begin()) {
    Serial.println("RTC module is NOT found");
    while (1);
  }

  // Inicializar pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se pudo encontrar la pantalla OLED"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();
  
  // Inicializar sensor DHT
  dht.begin();

  // Inicializar la tarjeta SD
  if (!SD.begin(SD_CS)) {
    Serial.println("Error: No se pudo inicializar la tarjeta SD");
    return;
  }
  Serial.println("Tarjeta SD inicializada correctamente");
}

void loop() {
  DateTime now = rtc.now();
  int minutoActual = now.minute();
  int horaActual = now.hour();

  // Crear un nuevo archivo si ha cambiado la hora
  if (horaActual != horaAnterior) {
    horaAnterior = horaActual;
    nombreArchivoActual = "/datos" + String(now.year()) + String(now.month()) + String(now.day()) + "-" + String(horaActual) + ".csv";
    File archivo = SD.open(nombreArchivoActual.c_str(), FILE_WRITE);
    if (archivo) {
      archivo.println("Fecha y Hora,Temperatura (°C),Humedad (%)");  // Encabezado del archivo CSV con columnas separadas
      archivo.close();
      Serial.println("Nuevo archivo creado: " + nombreArchivoActual);
    } else {
      Serial.println("Error al crear el archivo.");
    }
  }

  // Tomar lecturas del sensor cada minuto
  if (minutoActual != minutoAnterior) {
    minutoAnterior = minutoActual;

    float humedad = dht.readHumidity();
    float temperatura = dht.readTemperature();

    if (isnan(humedad) || isnan(temperatura)) {
      Serial.println(F("Error al leer el sensor DHT!"));
      return;
    }

    // Mostrar en OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Temp: " + String(temperatura) + " C");
    display.println("Humedad: " + String(humedad) + " %");
    display.display();

    // Guardar los datos en el archivo CSV con cada valor en su columna
    File archivo = SD.open(nombreArchivoActual.c_str(), FILE_APPEND);  // Usar FILE_APPEND para agregar a las filas
    if (archivo) {
      archivo.print(now.year());
      archivo.print("/");
      archivo.print(now.month());
      archivo.print("/");
      archivo.print(now.day());
      archivo.print(" ");
      archivo.print(now.hour());
      archivo.print(":");
      archivo.print(now.minute());
      archivo.print(":");
      archivo.print(now.second());
      archivo.print(",");  // Separador para la columna
      archivo.print(temperatura);  // Temperatura en su propia columna
      archivo.print(",");  // Separador para la columna
      archivo.println(humedad);  // Humedad en su propia columna
      archivo.close();
      Serial.println("Datos guardados en: " + nombreArchivoActual);
    } else {
      Serial.println("Error al escribir en el archivo.");
    }
  }

  delay(1000);  // Espera 1 segundo antes de volver a verificar
}
