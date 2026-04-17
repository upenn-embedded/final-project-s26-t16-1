#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include <ctype.h>

typedef struct struct_message {
    uint16_t adcValue;
} struct_message;

struct_message myData;

HardwareSerial ATmegaSerial(1);
#define ATMEGA_RX_PIN 14
#define ATMEGA_TX_PIN 33

uint8_t broadcastAddress[] = {0xF0, 0x24, 0xF9, 0x97, 0xD7, 0x88};

char lineBuf[64];
size_t linePos = 0;

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

bool extractFirstUInt16(const char *s, uint16_t &outVal) {
    while (*s && !isdigit((unsigned char)*s)) {
        s++;
    }

    if (!*s) {
        return false;
    }

    char *endptr;
    long parsed = strtol(s, &endptr, 10);

    if (endptr == s || parsed < 0 || parsed > 65535) {
        return false;
    }

    outVal = (uint16_t)parsed;
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("Starting Feather UART + ESP-NOW bridge...");

    ATmegaSerial.begin(9600, SERIAL_8N1, ATMEGA_RX_PIN, ATMEGA_TX_PIN);

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    Serial.println("Peripheral Controller Initialized.");
}

void loop() {
    while (ATmegaSerial.available()) {
        char c = ATmegaSerial.read();

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            lineBuf[linePos] = '\0';

            Serial.print("RAW LINE: [");
            Serial.print(lineBuf);
            Serial.println("]");

            uint16_t parsedValue;
            if (linePos > 0 && extractFirstUInt16(lineBuf, parsedValue)) {
                myData.adcValue = parsedValue;

                Serial.print("Sending ADC: ");
                Serial.println(myData.adcValue);

                esp_err_t result = esp_now_send(
                    broadcastAddress,
                    (uint8_t *)&myData,
                    sizeof(myData)
                );

                if (result != ESP_OK) {
                    Serial.print("esp_now_send error: ");
                    Serial.println(result);
                }
            } else {
                Serial.println("Parse error: no valid integer found.");
            }

            linePos = 0;
        } else {
            if (linePos < sizeof(lineBuf) - 1) {
                lineBuf[linePos++] = c;
            } else {
                Serial.println("UART line too long, clearing buffer.");
                linePos = 0;
            }
        }
    }
}


