// config: ////////////////////////////////////////////////////////////

#define PROTOCOL_TCP
#define MAX_NMEA_CLIENTS 4
//#define relay1 19
//#define relay2 18
const char* ssid     = "";
const char* password = "";
const char* MQTT_server= "";
const char* MQTT_user= "";
const char* MQTT_pass= "";
const char* ClientID= "Esp32";
const char* pubTopic= "Esp32/result";
const char* subTopic= "Esp32/cmd";
//when qos == 2, it does not work with my MQTT-GPIO control 
unsigned char qos= 1; //subscribe qos
bool retained= false;
bool debug = false;

/*************************  COM Port 0 *******************************/
#define UART_BAUD0 9600            // Baudrate UART0
#define SERIAL_PARAM0 SERIAL_8N1    // Data/Parity/Stop UART0
#define SERIAL0_RXPIN 21            // receive Pin UART0
#define SERIAL0_TXPIN 1             // transmit Pin UART0
#define SERIAL0_TCP_PORT 8880       // Wifi Port UART0

#define bufferSize 1024

//////////////////////////////////////////////////////////////////////////

