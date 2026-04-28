#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>

typedef struct struct_message {
   uint16_t adcValue;
} struct_message;

struct_message incomingData;

volatile bool newData = false;
volatile uint16_t currentAdcValue = 0;

HardwareSerial ATmegaSerial(2);
#define ATMEGA_RX_PIN 14 // ESP32 RX to ATmega TX (PD1)
#define ATMEGA_TX_PIN 32 // ESP32 TX to ATmega RX (PD0)

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
   if (len != sizeof(struct_message)) return; 
   
   memcpy(&incomingData, data, sizeof(incomingData));
   currentAdcValue = incomingData.adcValue;
   newData = true;
}

void setup() {
   Serial.begin(115200);
   ATmegaSerial.begin(9600, SERIAL_8N1, ATMEGA_RX_PIN, ATMEGA_TX_PIN);

   WiFi.mode(WIFI_STA);
   if (esp_now_init() != ESP_OK) return;
   esp_now_register_recv_cb(OnDataRecv);
   
   Serial.println("Central Bridge Ready. Waiting for data...");
}

void loop() {
   if (newData) {
       newData = false; 
       uint16_t adc = currentAdcValue; 
       
       Serial.print("Sending to ATmega: ");
       Serial.println(adc);

       // Send the value and flush the buffer
       ATmegaSerial.print(adc);
       ATmegaSerial.print("\r\n");
       ATmegaSerial.flush(); 
   }

   // Read debug messages from ATmega 
   while (ATmegaSerial.available()) {
       Serial.write(ATmegaSerial.read());
   }
}

