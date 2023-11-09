#include <Arduino.h>
#include <SensirionI2CSgp41.h>
#include <Wire.h>
#include"TFT_eSPI.h"

TFT_eSPI tft;
SensirionI2CSgp41 sgp41;

// time in seconds needed for NOx conditioning
uint16_t conditioning_s = 10;

void setup() {

    //initial screen
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_WHITE); //Red background

    //draw a green rectangle
    tft.fillRect(0, 0, 400, 80, TFT_GREEN);
    tft.drawRect(0, 0, 400, 80, TFT_BLACK);

    //draw a line
    tft.drawLine(0,160,400,160,TFT_BLACK);

    tft.setTextColor(TFT_BLACK);          //sets the text colour to black
    tft.setTextSize(3);                   //sets the size of text
    tft.drawString("SGP41 VALUE", 50, 30); 

    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    Wire.begin();

    uint16_t error;
    char errorMessage[256];

    sgp41.begin(Wire);

    uint8_t serialNumberSize = 3;
    uint16_t serialNumber[serialNumberSize];

    error = sgp41.getSerialNumber(serialNumber);

    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("SerialNumber:");
        Serial.print("0x");
        for (size_t i = 0; i < serialNumberSize; i++) {
            uint16_t value = serialNumber[i];
            Serial.print(value < 4096 ? "0" : "");
            Serial.print(value < 256 ? "0" : "");
            Serial.print(value < 16 ? "0" : "");
            Serial.print(value, HEX);
        }
        Serial.println();
    }

    uint16_t testResult;
    error = sgp41.executeSelfTest(testResult);
    if (error) {
        Serial.print("Error trying to execute executeSelfTest(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else if (testResult != 0xD400) {
        Serial.print("executeSelfTest failed with error: ");
        Serial.println(testResult);
    }
}

void loop() {
    uint16_t error;
    char errorMessage[256];
    uint16_t defaultRh = 0x8000;
    uint16_t defaultT = 0x6666;
    uint16_t srawVoc = 0;
    uint16_t srawNox = 0;

    delay(1000);

    if (conditioning_s > 0) {
        // During NOx conditioning (10s) SRAW NOx will remain 0
        error = sgp41.executeConditioning(defaultRh, defaultT, srawVoc);
        conditioning_s--;
    } else {
        // Read Measurement
        error = sgp41.measureRawSignals(defaultRh, defaultT, srawVoc, srawNox);
    }

    if (error) {
        Serial.print("Error trying to execute measureRawSignals(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("SRAW_VOC:");
        Serial.print(srawVoc);
        Serial.print("\t");
        Serial.print("SRAW_NOx:");
        Serial.println(srawNox);

        tft.setTextSize(2);
        tft.drawString("SRAW_VOC=", 50, 110);
        tft.drawString(String(srawVoc), 170, 110); 
        tft.drawString("SRAW_NOx=", 50, 190); 
        tft.drawString(String(srawNox), 170, 190); 
    }
}
