#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ESP32 - OLED
// GND  → GND
// 3V3  → VDD
// D22  → SCK / SCL
// D21  → SDA


// ESP32 - LoRa
// GND  → GND
// 3V3  → 3V3
// D14  → RST
// D2   → DIO0
// D5   → NSS / CS
// D23  → MOSI
// D19  → MISO
// D18  → SCK

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- LoRa ----------
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_SS     5
#define LORA_RST   14
#define LORA_DIO0   2


//#define LORA_BAND 868E6
 #define LORA_BAND 433E6

void printOLED(String line1, String line2 = "", String line3 = "", String line4 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(line1);

  display.setCursor(0, 16);
  display.println(line2);

  display.setCursor(0, 32);
  display.println(line3);

  display.setCursor(0, 48);
  display.println(line4);

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Avvio RICEVITORE LoRa + OLED");

  // ---------- OLED ----------
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERRORE: OLED non trovato a 0x3C");
    while (true) {
      delay(1000);
    }
  }

  printOLED("Ricevitore", "OLED OK", "Avvio LoRa...");

  // ---------- LoRa ----------
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("ERRORE: LoRa non inizializzato");
    printOLED("ERRORE", "LoRa non OK", "Controlla pin", "e frequenza");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("LoRa OK");
  Serial.println("In ascolto...");
  printOLED("Ricevitore OK", "LoRa in ascolto", "Aspetto messaggi...");
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String received = "";

    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    int rssi = LoRa.packetRssi();

    Serial.println("----------------------");
    Serial.print("Messaggio ricevuto: ");
    Serial.println(received);
    Serial.print("RSSI: ");
    Serial.println(rssi);

    if (received.startsWith("GPS,")) {
      // Formato: GPS,lat,lng
      int firstComma = received.indexOf(',');
      int secondComma = received.indexOf(',', firstComma + 1);

      String lat = received.substring(firstComma + 1, secondComma);
      String lng = received.substring(secondComma + 1);

      printOLED("GPS RICEVUTO", "Lat: " + lat, "Lng: " + lng, "RSSI: " + String(rssi));

    } else if (received.startsWith("NO_FIX")) {
      printOLED("Segnalatore OK", "GPS senza fix", received, "RSSI: " + String(rssi));

    } else {
      printOLED("Messaggio:", received, "RSSI: " + String(rssi));
    }
  }
}