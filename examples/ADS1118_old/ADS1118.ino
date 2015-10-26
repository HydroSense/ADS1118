//Colby Rome 10-10-2015

#include <SPI.h>
#define CS 5                    //CS_ADC on DIO5
unsigned int DEFAULT_CONFIG = 0x058B;
unsigned int TEST_CONFIG = 0b0001100000001011;

void setup() {
    pinMode(CS, OUTPUT);        //make CS an output
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));         //Clock Polarity 0, Clock Phase 1
    Serial.begin(9600);
    while(!Serial) ; // Wait for Serial to come up
    SPI.begin();
    Serial.print("DEFAULT_CONFIG = ");
    Serial.print(DEFAULT_CONFIG, BIN);
    Serial.println();
    Serial.println(DEFAULT_CONFIG, DEC);
    Serial.println(DEFAULT_CONFIG, HEX);
    Serial.print("Sizeof int = ");
    Serial.print(sizeof(int));
 
}

void loop() {
  // put your main code here, to run repeatedly:
//    Serial.println("Reading from ADS1118");
//    Serial.println(readConfig(DEFAULT_CONFIG),BIN);
    Serial.println("Reading the test config");
    Serial.println(readConfig(TEST_CONFIG), BIN);
    /*Serial.println(transfer_data(DEFAULT_CONFIG), BIN);
    delayMicroseconds(100);
    Serial.println(transfer_data(DEFAULT_CONFIG), BIN);
    */
    delay(1000);
}

int transfer_data(int data){
    digitalWrite(CS, LOW); // take CS low to select chip.
    delayMicroseconds(50); //Short delay
    int response = SPI.transfer(data); // capture the response
    digitalWrite(CS, HIGH);
    return response;
}

word readConfig(unsigned int config){
  /* The purpose of this function is to read the configuration.
   *  This is detailed on page 23; 32-bit data transmission cycle.
   *  CS is low and 4 bytes are sent. The last 2 bytes received are 
   *  the configuration registers.
   */
    // 1 integer = 2 bytes = 16 bits
    word rawValue; // This is the return config
    byte dummy, MSB, LSB;
    byte MSBConf = ((config >> 8) & 0xFF); // bit mask to get the most significant bit of config.
    byte LSBConf = (config & 0xFF);
    Serial.print("MSBCONF = ");
    Serial.print(MSBConf, BIN);
    Serial.print("LSBConf = ");
    Serial.print(LSBConf, BIN);
    digitalWrite(CS, LOW);
    delayMicroseconds(500);
    dummy = SPI.transfer(MSBConf);
    dummy = SPI.transfer(LSBConf);
    //delayMicroseconds(50); 
    // CS remains low
    MSB = SPI.transfer(MSBConf);
    LSB = SPI.transfer(LSBConf);
    delay(1);
    digitalWrite(CS, HIGH);

/*    Serial.println(MSB1, BIN);
    Serial.println(LSB1, BIN);
    Serial.println(MSB2, BIN);
    Serial.println(LSB2, BIN);
    */
    Serial.print("MSB received = ");
    printbyte(MSB);
    Serial.print("LSB received = ");
    printbyte(LSB);
    rawValue = (MSB<<8)|LSB;
    return rawValue;
}

void printbyte(byte b){
  byte mask;
  for(mask = 128; mask >0; mask>>=1){
    if(b & mask){
      Serial.print("1");
    }
     else Serial.print("0");
    }
  Serial.println();
}


