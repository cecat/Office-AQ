
// Variables

// reporting and checking intervals
#define REPORT_period        120001  // 2 min reporting interval

// BME SENSOR  
#define SEALEVELPRESSURE_HPA (1013.25)


// mqtt debugging
int   mqttCt        = 0;
int   mqttFails     = 0;        
int   mqttDis       = 0;        // unexpected disconnects