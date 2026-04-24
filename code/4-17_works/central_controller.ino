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
#define ATMEGA_RX_PIN 14 // ESP32 RX <- ATmega TX (PD1)
#define ATMEGA_TX_PIN 32 // ESP32 TX -> ATmega RX (PD0)

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

       // 1. Send the number
       ATmegaSerial.print(adc);
       // 2. Send exactly the \r\n sequence your uart_scanf demands
       ATmegaSerial.print("\r\n");
       // 3. Force the buffer to transmit immediately so the \n isn't delayed
       ATmegaSerial.flush(); 
   }

   // Read debug messages from ATmega and print to PC
   while (ATmegaSerial.available()) {
       Serial.write(ATmegaSerial.read());
   }
}
