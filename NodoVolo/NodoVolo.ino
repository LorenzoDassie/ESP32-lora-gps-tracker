#include <SPI.h>
#include <LoRa.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// ESP32 - LoRa
// GND  → GND
// 3V3  → 3V3
// D14  → RST
// D2   → DIO0
// D5   → NSS
// D23  → MOSI
// D19  → MISO
// D18  → SCK

// ESP32 - GPS NEO-6M
// GND  → GND
// 3V3  → VCC
// D16  → TX del GPS
// D17  → RX del GPS

// ---------- GPS ----------
TinyGPSPlus gps;
HardwareSerial GPSserial(1);

#define GPS_RX 16   // ESP32 D16 riceve dal TX del GPS
#define GPS_TX 17   // ESP32 D17 va al RX del GPS, opzionale

// ---------- LoRa ----------
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_SS     5
#define LORA_RST   14
#define LORA_DIO0   2


//#define LORA_BAND 868E6
 #define LORA_BAND 433E6

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Avvio SEGNALATORE: GPS + LoRa TX");

  // GPS NEO-6M:  9600 baud
  GPSserial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS avviato su D16/D17");

  // LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("ERRORE: LoRa non inizializzato");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("LoRa OK");
  Serial.println("Invio ogni 3 secondi...");
}

void loop() {
  // Legge continuamente i dati in arrivo dal GPS
  while (GPSserial.available() > 0) {
    gps.encode(GPSserial.read());
  }

  static unsigned long lastSend = 0;
  static int counter = 0;

  if (millis() - lastSend >= 3000) {
    lastSend = millis();

    String message;

    Serial.println("----------------------");

    if (gps.location.isValid()) {
      String lat = String(gps.location.lat(), 6);
      String lng = String(gps.location.lng(), 6);

      message = "GPS," + lat + "," + lng;

      Serial.println("GPS FIX OK");
      Serial.println("Invio coordinate:");
      Serial.println(message);

      if (gps.satellites.isValid()) {
        Serial.print("Satelliti: ");
        Serial.println(gps.satellites.value());
      }

    } else {
      message = "NO_FIX," + String(counter);

      Serial.println("GPS senza fix valido");
      Serial.print("Caratteri GPS letti: ");
      Serial.println(gps.charsProcessed());
      Serial.println("Invio messaggio di test:");
      Serial.println(message);

      if (gps.charsProcessed() < 10) {
        Serial.println("ATTENZIONE: non sembrano arrivare dati dal GPS.");
        Serial.println("Controlla: TX GPS -> D16, VCC, GND.");
      }
    }

    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();

    counter++;
  }
}