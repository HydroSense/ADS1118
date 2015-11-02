/*
   Ads111.cpp - library for interacting with TI ADC
   Created by Colby Rome, October 20, 2015
*/
#include "Arduino.h"
#include "Ads1118.h"

#ifdef DEBUG
    #define DEBUG_PRINT(x)  Serial.println(x)
#else
    #define DEBUG_PRINT(x)
#endif

Ads1118::Ads1118(int CS_pin)
{
    pinMode(CS_pin, OUTPUT);
    _cs = CS_pin;
}

void Ads1118::begin()
{
    //Clock Polarity 0, Clock Phase 1 -> SPI mode 1. MSB first.
    //Page 25 on datasheet
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));
    SPI.begin();
    digitalWrite(_cs, HIGH); // Do not start transactions yet
    if(self_test()){
        //Error
        DEBUG_PRINT("An error has occurred during self test");
    }
}

byte Ads1118::self_test()
{
    //returns 0 (no error) or 1 (error)
    CURRENT_CONFIG = CONFIG_DEFAULT;
    if(CONFIG_DEFAULT != update_config(CONFIG_DEFAULT)){
        return 1;
    }
    return 0;
}

word Ads1118::update_config(word new_config)
{
    /* This function executes a 32-bit transaction, returning
       the configuration register 
     */
    word readConfig; //this is the return config
    byte MSB, LSB;
    byte MSBConf = ((new_config >> 8) & 0xFF); // bit mask to get MSB
    byte LSBConf = (new_config & 0xFF);
    digitalWrite(_cs, LOW);
    delayMicroseconds(500);
    // The following 2 transfers result in garbage data:
    SPI.transfer(MSBConf);
    SPI.transfer(LSBConf);

    MSB = SPI.transfer(MSBConf);
    LSB = SPI.transfer(LSBConf);
    digitalWrite(_cs, HIGH); // Done writing
    readConfig = (MSB << 8) | LSB;
    CURRENT_CONFIG = new_config; // Update global configuration variable
    return readConfig;
}

word Ads1118::adsReadRaw(word port)
{
    /* This method executes a 16 bit transaction if the current config
       is already correct, or 2 32-bit transactions otherwise, updating
       the config as necessary.
    */
    word read;
    byte dummy, MSB, LSB;
#ifdef DEBUG
    Serial.print("Current config is: ");
    Serial.println(CURRENT_CONFIG, HEX);
    Serial.print("Updating configuration to: ");
    Serial.println(port | (CURRENT_CONFIG & ~(PIN_BITMASK)), HEX);
    Serial.print("CURRENT_CONFIG & ~PIN_BITMASK = ");
    Serial.println((CURRENT_CONFIG & ~PIN_BITMASK), HEX);
#endif
    if((CURRENT_CONFIG & PIN_BITMASK) != port){
        //Updates the configuration to the proper pin
        update_config((port | (CURRENT_CONFIG & ~(PIN_BITMASK))));
        delayMicroseconds(1000); // Experiment with this
    }
#ifdef DEBUG
    Serial.print("Input mux: ");
    Serial.println(PIN_BITMASK & CURRENT_CONFIG, HEX);
    Serial.print("MOSI: ");
    Serial.println(CURRENT_CONFIG, HEX);
#endif
    digitalWrite(_cs, LOW);
    delayMicroseconds(500);
    MSB = SPI.transfer((CURRENT_CONFIG >> 8) & 0xFF);
    LSB = SPI.transfer(CURRENT_CONFIG & 0xFF);
    digitalWrite(_cs, HIGH);
    read = (MSB << 8) | LSB;
    return read;
}

double Ads1118::convToFloat(word read)
{
    /* This method returns a double (float) signifying the input signal
       Vin for the proper FS (scaling).
    */
    float gain[6] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};
    float myGain = gain[(((PGA_BITMASK & CURRENT_CONFIG) >> 8) / 2)];
#ifdef DEBUG
    Serial.print("Gain = ");
    Serial.println(gain[(((PGA_BITMASK & CURRENT_CONFIG) >> 8) / 2)], DEC);
#endif
    return (float)((int)read)*myGain/(0x8000);
}

double Ads1118::adsRead(word port)
{
    /* This method returns the floating point representation 
    return convToFloat(adsReadRaw(port)); // Reads from port; converts to float
}

double Ads1118::readTemp()
{
    /* This method loads the temperature configuration, and returns the
       floating point representation of the temperature in degrees Celcius.
    */
    if(update_config(CONFIG_TEMPERATURE) != CONFIG_TEMPERATURE){
        DEBUG_PRINT("Error: not updating to temp config");
    }
    word read;
    byte MSB, LSB;
    digitalWrite(_cs, LOW);
    delayMicroseconds(500);
    MSB = SPI.transfer((CURRENT_CONFIG >> 8) & 0xFF);
    LSB = SPI.transfer(CURRENT_CONFIG & 0xFF);
    digitalWrite(_cs, HIGH);
    delayMicroseconds(500);
    digitalWrite(_cs, LOW);
    delayMicroseconds(500);
    MSB = SPI.transfer((CURRENT_CONFIG >> 8) & 0xFF);
    LSB = SPI.transfer(CURRENT_CONFIG & 0xFF);
    digitalWrite(_cs, HIGH);
    read = (MSB << 6) | (LSB >> 2);
    Serial.print("Raw MSB =");
    Serial.println(MSB, HEX);
    Serial.print("Raw LSB =");
    Serial.println(LSB, BIN);
    Serial.print("Raw read =");
    Serial.println(read, BIN);
    if(0x8000 & read == 0){
        return (double)read*0.03125; // datasheet page 18
    }
    read = !(read - 1);
    return (double)read*-0.03125;
}
