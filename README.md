#ADS1118 Arduino Library
The offical HydroSense library for the TI ADS1118 16-Bit ADC chip.

Created By: Colby Rome (cdr013@bucknell.edu)
Last Updated By: Ward Prescott (erp006@bucknell.edu) on Mar 9, 2016 12:08 AM

##ADS1118 Class
###Constructor
`Ads1118(int CS_pin)`
###Methods
- `.begin(void)`
- `adsRead(int port)`
- `readTemp(void)`
- `setGain(int GainSet)`
- `selfTest(void)`

###Parameters
There no public parameters of interest.

##Usage
Clone repository into your Arduino libraries folder (typically ~/Arduino/libraries/)

```

```

##Notes

##Further Work
1. Temperature readings
2. Low power mode (current it is set to constantly sample)
