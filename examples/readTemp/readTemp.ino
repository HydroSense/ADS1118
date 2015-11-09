// Colby Rome 11-8-2015

#include "Ads1118.h"
#include <SPI.h>
#define CS 5

Ads1118 ads1118(CS); // instantiate an instance of class Ads1118

void setup(){
    ads1118.begin();
    while(!Serial); // wait for serial to come up
}

void loop(){
    Serial.println("Reading temperature");
    Serial.println(ads1118.readTemp(), DEC);
    Serial.println();

    delay(1000);
}
