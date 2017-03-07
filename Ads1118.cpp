/*
   Ads111.cpp - library for interacting with TI ADC
   Created by Colby Rome, October 20, 2015
   Edited by Ward Prescott, March 1, 2016
*/

#include "Arduino.h"
#include "Ads1118.h"

//#define DEBUG

#ifdef DEBUG
    #define DEBUG_PRINT(x)  Serial.println(x)
#else
    #define DEBUG_PRINT(x)
#endif

Ads1118::Ads1118(int CS_pin)
{
    /* Constructor for Ads1118 class. */
    _cs = CS_pin;
}

void Ads1118::begin()
{
    /* Initializes SPI parameters.
       Clock Polarity 0, Clock Phase 1 -> SPI mode 1. MSB first.
       Page 25 on datasheet
     */
    pinMode(_cs, OUTPUT);
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));
    SPI.begin();
    digitalWrite(_cs, HIGH);  // Do not begin transactions yet
    if(self_test()){
        //Error
        DEBUG_PRINT("An error has occurred during self test");
    }
}

byte Ads1118::self_test()
{
    /* This self test returns 1 if there is an error communicating with
       the ADS1118 chip. This may be due to a wiring problem or other.
     */
    CURRENT_CONFIG = CONFIG_DEFAULT;
    if(CONFIG_DEFAULT != update_config(CONFIG_DEFAULT)){
        return 1;
    }
    return 0;
}

word Ads1118::update_config(uint16_t new_config)
{
    /* This function executes a 32-bit transaction.

       Input: a word new_config (see datasheet)
       Output: a word specifying the configuration register. Should be
       identical to new_config if config updated correctly.
     */
    word readConfig; //this is the return config

    digitalWrite(_cs, LOW);
    delayMicroseconds(1);
    // The following transfer results in garbage data:
    SPI.transfer16(new_config);
    // We capture the following transfer to read back the configuration
    readConfig = SPI.transfer16(new_config);
    digitalWrite(_cs, HIGH); // Done writing


    CURRENT_CONFIG = new_config; // Update global configuration variable
    Serial.print("Current_config is:");
    Serial.println(CURRENT_CONFIG, HEX);
    return readConfig;
}

word Ads1118::adsReadRaw(word port)
{
    /* This method executes a 16 bit transaction if the current config
       is already correct, or 2 32-bit transactions otherwise, updating
       the config as necessary.

       Input: A port specifying the pin selection (see header file)
       Output: Raw requested data from sensor
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

    //Only update the configuration *if necessary*
    if((CURRENT_CONFIG & PIN_BITMASK) != port){
        //Updates the configuration to the proper pin:
        update_config((port | (CURRENT_CONFIG & ~(PIN_BITMASK))));
    }

#ifdef DEBUG
    Serial.print("Input mux: 0x");
    Serial.println(PIN_BITMASK & CURRENT_CONFIG, HEX);
    Serial.print("MOSI: ");
    Serial.println(CURRENT_CONFIG, HEX);
#endif

    digitalWrite(_cs, LOW);
    delayMicroseconds(1);
    read = SPI.transfer16(CURRENT_CONFIG);
    digitalWrite(_cs, HIGH);
    return read;
}

double Ads1118::convToFloat(word read)
{
    /* This method returns a double (float) signifying the input signal
       Vin for the proper FS (scaling).

       Input: a binary word from the sensor
       Output: the floating point representation of the sensor value.
    */

    // See page 26 of datasheet detailing the 6 gain modes.
    float gain[6] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};

    // Find the correct gain index using bitshift and bitmask
    float myGain = gain[(((PGA_BITMASK & CURRENT_CONFIG) >> 8) / 2)];

#ifdef DEBUG
    Serial.print("Gain = ");
    Serial.println(myGain);
#endif

    return (float)((int)read)*((float)myGain)/ ((float)32768);
}


bool Ads1118::setGain(uint16_t GainSet){
    //Temp variable
    uint16_t newConfig = CURRENT_CONFIG;

    //Clear the corresponding bits in current config
    newConfig &= ~(0b111 << 9);
    
    //Set the corresponding pins in current config
    newConfig |= (GainSet << 9);
    
    //Send update to the ADC
    if (update_config(newConfig) == newConfig){
        return true;
    }
    else {
        return false;
    }
}

double Ads1118::adsRead(word port)
{
    /* This method returns the floating point representation of the desired
       sensor operation.

       Input: a port selection (see header file)
       Output: the floating point representation of desired operation.
     */
    return convToFloat(adsReadRaw(port)); // Reads from port; converts to float
}

double Ads1118::readTemp()
{
    /* This function is still being debug and may or may not work.
     */

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
    /*
    MSB = SPI.transfer((CURRENT_CONFIG >> 8) & 0xFF);
    LSB = SPI.transfer(CURRENT_CONFIG & 0xFF);
    digitalWrite(_cs, HIGH);
    delayMicroseconds(500);
    digitalWrite(_cs, LOW);
    delayMicroseconds(500);
    */
#ifdef DEBUG
    Serial.print("Sending: ");
    Serial.println((CURRENT_CONFIG >> 8), HEX);
#endif
    MSB = SPI.transfer((CURRENT_CONFIG >> 8) & 0xFF);
#ifdef DEBUG
    Serial.print("Sending: ");
    Serial.println((CURRENT_CONFIG & 0xFF), HEX);
#endif
    LSB = SPI.transfer(CURRENT_CONFIG & 0xFF);
    delayMicroseconds(50);
    digitalWrite(_cs, HIGH);
    read = (MSB << 6) | (LSB >> 2);
#ifdef DEBUG
    Serial.print("Raw MSB =");
    Serial.println(MSB, BIN);
    Serial.print("Raw LSB =");
    Serial.println(LSB, BIN);
    Serial.print("Raw read =");
    Serial.println(read, BIN);
#endif
    if((0x8000 & read) == 0){
        return (double)read*0.03125; // datasheet page 18
    }
    read = ~(read - 1);
    return (double)read*-0.03125;
}

