
// Variables

// reporting and checking intervals
#define REPORT_period        120001  // super short reporting interval while we figure out an MQTT issue

// CO2 sensor

SCD4x mySensor;
float myCO2 = 0;
float temperature = 0;
float temperatureF = 0;
float humidity = 0;

// BME280 sensor

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
float bmeTemp = 0;
float bmeTempF = 0;
float bmeBP = 0;
float bmeAlt = 0;
float bmeRH = 0;

// mqtt debugging
int   mqttCt        = 0;
int   mqttFails     = 0;        
int   mqttDis       = 0;        // unexpected disconnects

// misc
int reportCounter = 0;