/*
  Monitor  air quality
    C. Catlett Jul 2021
                          
 */

#include <Particle.h>
#include <MQTT.h>
#include "secrets.h" 

#include <Wire.h>
#include <SparkFun_SCD4x_Arduino_Library.h>
#include <Adafruit_BME280.h>

// include topics for mqtt
#include "topics.h"
// include misc variables
#include "vars.h"

// MQTT 
#define MQTT_KEEPALIVE 30 * 60              // 60s is std default
void timer_callback_send_mqqt_data();    
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
     char p[length + 1];
     memcpy(p, payload, length);
     p[length] = 0; 
     Particle.publish("mqtt recvd", p, 3600, PRIVATE);
 }
MQTT client(MY_SERVER, 1883, MQTT_KEEPALIVE, mqtt_callback);
int MQTT_CODE = 0;

Timer reportTimer(REPORT_period, reportPower);
bool  TimeToReport    = TRUE;

// Application watchdog - sometimes MQTT wedges things
int DOGTIME = 120000;           // wait 2 minutes before pulling the ripcord
retained bool REBORN  = FALSE;  // did app watchdog restart us?
ApplicationWatchdog *wd;
void watchdogHandler() {
  REBORN = TRUE;
  System.reset(RESET_NO_WAIT);
}
retained bool SELF_RESTART = FALSE;
int fails = 0;
int GIVE_UP = 3;

void setup() {
    Time.zone (-5);
    Particle.syncTime();

    Wire.begin();

  //.begin will start periodic measurements for us (see the later examples for details on how to override this)
      if (mySensor.begin() == false) {
        Particle.publish("dbug", "CO2 prob - check wiring.");
      }

  // bme
      if (bme.begin() == false) {
        Particle.publish("dbug", "CO2 prob - check wiring.");
      }

    Particle.publish("mqtt_startup", "Attempting to connect to HA", 3600, PRIVATE);
    client.connect(CLIENT_NAME, HA_USR, HA_PWD);
    // check MQTT 
    if (client.isConnected()) {
        Particle.publish("mqtt_startup", "Connected to HA", 3600, PRIVATE);
        //client.disconnect();
      } else {
        Particle.publish("mqtt_startup", "Fail connect HA - check secrets.h", 3600, PRIVATE);
    }
    //client.disconnect();
    
    reportTimer.start();
}

void loop() {


    if (TimeToReport) {

      TimeToReport = FALSE;

        // check sensors
      //Particle.publish("dbug", "checking sensors", 3600, PRIVATE);

      myCO2 = mySensor.getCO2();
      temperature = mySensor.getTemperature();
      temperatureF = (temperature * 9/5) + 32;
      humidity = mySensor.getHumidity();

      bmeTemp = bme.readTemperature();
      bmeTempF = (bmeTemp * 9/5) + 32;
      bmeRH = bme.readHumidity();
      bmeBP = (bme.readPressure() / 100.0F);
      bmeAlt = bme.readAltitude(SEALEVELPRESSURE_HPA);


     // if (++reportCounter%10=0) {
     //   Particle.publish("dbug", "reporting in", 3600, PRIVATE);
    //  }

      if (client.isConnected()) {
      //  Particle.publish("dbug", "still connected from last time", 3600, PRIVATE); delay(100);
      } else {  
        //Particle.publish("mqtt", "reconnecting", 3600, PRIVATE); delay(100);
        client.connect(CLIENT_NAME, HA_USR,HA_PWD);
       // delay(2000);
      }

      if (client.isConnected()){
        fails=0;
        tellHASS(TOPIC_A, String(myCO2));
        tellHASS(TOPIC_B, String(temperatureF));
        tellHASS(TOPIC_C, String(humidity));
        tellHASS(TOPIC_D, String(bmeTempF));
        tellHASS(TOPIC_E, String(bmeRH));
        tellHASS(TOPIC_F, String(bmeBP));
        tellHASS(TOPIC_G, String(bmeAlt));

      } else {
        Particle.publish("mqtt", "Failed to connect", 3600, PRIVATE);
        mqttFails++;
        fails++;
        if (fails > GIVE_UP) {
          SELF_RESTART=TRUE;
          Particle.publish("-STUCK-", "Too many fails- restarting", 3600, PRIVATE);
          System.reset(RESET_NO_WAIT);
        }
      }
      
    }
} 
/************************************/
/***         FUNCTIONS       ***/
/************************************/


// Reporting timer interrupt handler
void reportPower() {  TimeToReport = TRUE;  }

// Report to HASS via MQTT
void tellHASS (const char *ha_topic, String ha_payload) {  

  //delay(100); // bit of delay in between successive messages
  if (client.isConnected()){
    client.publish(ha_topic, ha_payload);
    mqttCt++;
//    Particle.publish("dbug", "still connected", 3600, PRIVATE);
  } else {
    mqttFails++;
    Particle.publish("mqtt", "Connection dropped", 3600, PRIVATE);
  }
}

