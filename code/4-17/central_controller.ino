#include <HardwareSerial.h>

HardwareSerial ATmegaSerial(2);
#define ATMEGA_RX_PIN 38 // ESP32 RX <- ATmega TX (PD1)
#define ATMEGA_TX_PIN 33 // ESP32 TX -> ATmega RX (PD0)

void setup() {
    Serial.begin(115200);
    // Remember: Your uart.h defines 9600 baud
    ATmegaSerial.begin(9600, SERIAL_8N1, ATMEGA_RX_PIN, ATMEGA_TX_PIN);
    
    Serial.println("ESP32 Test Sender Ready.");
    Serial.println("Waiting 3 seconds before starting...");
    delay(3000);
}

void loop() {
    // 1. Send a simple 'X' every 2 seconds
    static unsigned long lastTime = 0;
    if (millis() - lastTime > 2000) {
        lastTime = millis();
        Serial.println("ESP32: Sending 'X' to ATmega...");
        ATmegaSerial.print('X'); 
    }

    // 2. Read anything coming BACK from the ATmega and print to PC
    while (ATmegaSerial.available()) {
        Serial.write(ATmegaSerial.read());
    }
}