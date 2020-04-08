#include "mbed.h"
#include "TCPSocket.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "RemoteIR/ReceiverIR.h"
#include "RemoteIR/TransmitterIR.h"
#include <cstring>
#include <ctype.h>
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
Thread thread;
WiFiInterface *wifi;
TS_StateTypeDef  TS_State = {0};
Serial UART(SERIAL_TX, SERIAL_RX);

int arrivedcount = 0;


void get_the_payload(char payload[]){
    
    //CONVERT TO LOWER CASE
    for(int i=0; payload[i]; i++){
        payload[i] = tolower(payload[i]);
    }

    //WORD
    char ON[] = "on";
    char OFF[] = "off";
    char AUTO[] = "auto";
    char COOL[] = "cool";
    char DRY[] = "dry";


    //STRSTR_VARABLE
    char *ON_STRSTR;
    char *OFF_STRSTR;
    char *AUTO_STRSTR;
    char *COOL_STRSTR;
    char *DRY_STRSTR;
    
    //STRSTR
    ON_STRSTR = strstr(payload, ON);
    OFF_STRSTR = strstr(payload, OFF);
    AUTO_STRSTR = strstr(payload, AUTO);
    COOL_STRSTR = strstr(payload, COOL);
    DRY_STRSTR = strstr(payload, DRY);

    //CHECK_PRESSED_BUTTON
    if(ON_STRSTR){
        printf("You press ON\n");
        
    }

    else if(OFF_STRSTR){
        printf("You press OFF\n");
    }

    else if(AUTO_STRSTR){
        printf("You press AUTO\n");
    }

    else if(COOL_STRSTR){
        printf("You press COOL\n");
    }

    else if(DRY_STRSTR){
        printf("You press DRY\n");
    }

    else {
        printf("MAI JER\n");
    }
}

void touchscreen_display(){
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    /* Set Touchscreen Demo1 description */
    // BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 100); 
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK); 
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"A/C Simulator", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 50, (uint8_t*) "26 C", LEFT_MODE);
    BSP_LCD_DrawCircle(29, 50, 2);
}

// callback for subscribe topic
void subscribeCallback(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    //printf("Message received: qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    // printf("%.*s\n", message.payloadlen, (char*)message.payload);
    // printf("String len : %d\n", strlen((char*)message.payload));
    get_the_payload((char*)message.payload);
    ++arrivedcount;
}

int main()
{
    //DISPLAY_SETTING
    UART.baud(115200);
    uint16_t x1, y1;
    BSP_LCD_Init();

    //DISPLAY_SHOW
    thread.start(touchscreen_display);

    int count = 0;
    char* topic = "/Final";

    printf("WiFi MQTT example\n");

    #ifdef MBED_MAJOR_VERSION
        printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    #endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) 
    {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) 
    {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());

    MQTTNetwork mqttNetwork(wifi);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "192.168.43.248";
    int port = 1883;
    printf("[Subscribe] Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
    {
        printf("[Subscribe] rc from TCP connect is %d\r\n", rc);
    }
        
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-subscribe";
    data.username.cstring = "";
    data.password.cstring = "";

    // printf("RC_1 : %d\n", rc);
    if ((rc = client.connect(data)) != 0)
    {
        printf("[Subscribe]rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("[Subscribe] Client Connected.\r\n");
    }

    
    // printf("RC_2 : %d\n", rc);   
    if ((rc = client.subscribe(topic, MQTT::QOS0, subscribeCallback)) != 0)
    {
        printf("[Subscribe] rc from MQTT subscribe is %d\r\n", rc);
    }
    else
    {
        printf("[Subscribe] Client subscribed.\r\n");
    }
        

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;

    while(1){
        client.yield(20000); 
    }
    
    printf("[Subscribe] Finishing with %d messages received\n", arrivedcount);
    
    mqttNetwork.disconnect();

    if ((rc = client.disconnect()) != 0)
    {
        printf("[Subscribe] rc from disconnect was %d\r\n", rc);
        printf("[Subscribe] Client Disconnected.\r\n");
    }

    wifi->disconnect();

    printf("\n[Subscribe] Done\n");
}