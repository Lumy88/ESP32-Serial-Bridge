// ESP32 WiFi（TCP/MQTT） <-> UART Bridge
// Forked from AlphaLima/ESP32-Serial-Bridge

#include "config.h"
#include <WiFi.h>
#include <PubSubClient.h>
void callback(char* topic, byte* payload, unsigned int length);

HardwareSerial* COM = &Serial;

uint8_t buf1[bufferSize];
uint16_t i1=0;

uint8_t buf2[bufferSize];
uint16_t i2=0;

#ifdef PROTOCOL_TCP
  #include <WiFiClient.h>
  WiFiServer server_0(SERIAL0_TCP_PORT);
  WiFiServer *server=&server_0;
  WiFiClient TCPClient[MAX_NMEA_CLIENTS]; 
#endif

WiFiClient wfclient;
PubSubClient client(MQTT_server, 1883, callback, wfclient);

void callback(char* topic, byte* payload, unsigned int length) {
  
  byte *p = (byte *)malloc(length); 
  memcpy(p, payload, length); // Copy the payload to the new buffer

  /*
  char msg[length+1];
  for(int i=0;i<length;i++){
    msg[i]=(char)p[i];
  }
  msg[length]='\0';
  String msgText(msg);

  if ( msgText == "open relay1") digitalWrite(relay1, HIGH);
  else if( msgText == "close relay1") digitalWrite(relay1, LOW);
  else if ( msgText == "open relay2") digitalWrite(relay2, HIGH);
  else if ( msgText == "close relay2") digitalWrite(relay2, LOW);
  else {
    client.publish(pubTopic, p, length, retained);
    COM->write(p, length); // UART write buffer received via MQTT sub_topic
  }
  */
  client.publish(pubTopic, p, length, retained);
  COM->write(p, length); // UART write buffer received via MQTT sub_topic
  free(p);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_server, MQTT_user, MQTT_pass)) {
      // Once connected, publish an announcement...
      client.publish(pubTopic,"UART<-->WiFi Bridge Reconnected", false);
      // ... and resubscribe
      client.subscribe(subTopic, qos);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  delay(500);
  
  COM->begin(UART_BAUD0, SERIAL_PARAM0, SERIAL0_RXPIN, SERIAL0_TXPIN);
  if(debug) COM->println("\n\n WiFi Serial Bridge V2.00");
  
  //pinMode(relay1, OUTPUT);
  //pinMode(relay2, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  if (client.connect(ClientID, MQTT_user, MQTT_pass)){
    client.publish(pubTopic, "UART<-->WiFi Bridge Connected", false);
    client.subscribe(subTopic, qos);
  }

  #ifdef PROTOCOL_TCP
    COM->println("Starting TCP Server");  
    server->begin(); // start TCP server 
    server->setNoDelay(true);
  #endif

  esp_err_t esp_wifi_set_max_tx_power(50);  //lower WiFi Power
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
    
  #ifdef PROTOCOL_TCP
    if (server->hasClient()) {
      for(byte i = 0; i < MAX_NMEA_CLIENTS; i++){
      //find free/disconnected spot
        if (!TCPClient[i] || !TCPClient[i].connected()){
          if(TCPClient[i]) TCPClient[i].stop();
          TCPClient[i] = server->available();
          if(debug) COM->print("New client for COM"); 
          if(debug) COM->print(0); 
          if(debug) COM->println(i);
          continue;
        }
      }
        //no free/disconnected spot so reject
        WiFiClient TmpserverClient = server->available();
        TmpserverClient.stop();
    }
  #endif
 
  if(COM != NULL) {
    for(byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++) {               
      if(TCPClient[cln]) {
        while(TCPClient[cln].available()) {
          buf1[i1] = TCPClient[cln].read(); // read char from TCP port:8880
          if(i1<bufferSize-1) i1++;
        } 
        COM->write(buf1, i1); // now send to UART
        
        /*
        char msg[i1+1];
        for(int i=0;i<i1;i++){
          msg[i]=(char)buf1[i];
        }
        msg[i1]='\0';
        String msgT(msg);

        if ( msgT == "open relay1") digitalWrite(relay1, HIGH);
        else if( msgT == "close relay1") digitalWrite(relay1, LOW);
        else if ( msgT == "open relay2") digitalWrite(relay2, HIGH);
        else if ( msgT == "close relay2") digitalWrite(relay2, LOW);
        else {
         COM->write(buf1, i1); // now send to UART
        }
        */
        
        i1 = 0;
      }
    }
  
    if(COM->available()) {
      while(COM->available()) {     
        buf2[i2] = COM->read(); // read char from UART
        if(i2<bufferSize-1) i2++;
      }
      // now send to WiFi:
      for(byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++) {   
        if(TCPClient[cln])                     
          TCPClient[cln].write(buf2, i2); //send the buffer to TCP port:8880 
      }
      client.publish(pubTopic, buf2, i2, retained); //Publish the buffer received via serial
      i2 = 0;
    }
  }
}
