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

int FIRST_TIME_DISPLAY_STATUS = 0;
int FIRST_TIME_DISPLAY_MODE = 0;
int FIRST_TIME_DISPLAY_FANSPEED = 0;
int FIRST_TIME_DISPLAY_WIND_DIRECTION = 0;

int arrivedcount = 0;

void clear_screen_first_time(){
    BSP_LCD_ClearStringLine(6);
    BSP_LCD_ClearStringLine(7);
}

void update_power_status(int status, int firsttime){
    if(firsttime == 0)
    {
        BSP_LCD_ClearStringLine(7);
    }

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_ClearStringLine(3);
    if(status == 0){
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAtLine(3,(uint8_t*) "Status : OFF");
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    } else {
        BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGREEN);
        BSP_LCD_DisplayStringAtLine(3,(uint8_t*) "Status : ON");
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    }
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_ClearStringLine(6);

   
}

void update_temp(uint8_t*  number){
    clear_screen_first_time();
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAtLine(6, (uint8_t*) number);
    BSP_LCD_DrawCircle(33, 91, 2);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(29, 95, (uint8_t*) " C", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
}

void update_mode(int mode, int firsttime){
    
    if(firsttime == 0){
        clear_screen_first_time();
    }

    BSP_LCD_SetFont(&Font16);
    if(mode == 0){
        BSP_LCD_ClearStringLine(9);
        BSP_LCD_DrawHLine(0, 136, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(9, (uint8_t*) "MODE : Auto");
        BSP_LCD_DrawHLine(0, 163, BSP_LCD_GetXSize());
    } else if(mode == 1){
        BSP_LCD_ClearStringLine(9);
        BSP_LCD_DrawHLine(0, 136, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(9, (uint8_t*) "MODE : Cool");
        BSP_LCD_DrawHLine(0, 163, BSP_LCD_GetXSize());
    } else if(mode == 2){
        BSP_LCD_ClearStringLine(9);
        BSP_LCD_DrawHLine(0, 136, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(9, (uint8_t*) "MODE : Dry");
        BSP_LCD_DrawHLine(0, 163, BSP_LCD_GetXSize());
    }
}

void update_fanspeed(int mode, int firsttime){
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_LIGHTBLUE);
    if(mode == 0){
        BSP_LCD_ClearStringLine(11);
        BSP_LCD_DrawHLine(0, 170, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(11, (uint8_t*) "FANSPEED : Auto");
        BSP_LCD_DrawHLine(0, 195, BSP_LCD_GetXSize());
    } else if(mode == 1){
        BSP_LCD_ClearStringLine(11);
        BSP_LCD_DrawHLine(0, 170, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(11, (uint8_t*) "FANSPEED: Low");
        BSP_LCD_DrawHLine(0, 195, BSP_LCD_GetXSize());
    } else if(mode == 2){
        BSP_LCD_ClearStringLine(11);
        BSP_LCD_DrawHLine(0, 170, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(11, (uint8_t*) "FANSPEED : Medium");
        BSP_LCD_DrawHLine(0, 195, BSP_LCD_GetXSize());
    } else if(mode == 3){
        BSP_LCD_ClearStringLine(11);
        BSP_LCD_DrawHLine(0, 170, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(11, (uint8_t*) "FANSPEED : High");
        BSP_LCD_DrawHLine(0, 195, BSP_LCD_GetXSize());
    } else if(mode == 4){
        BSP_LCD_ClearStringLine(11);
        BSP_LCD_DrawHLine(0, 170, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(11, (uint8_t*) "FANSPEED : Turbo");
        BSP_LCD_DrawHLine(0, 195, BSP_LCD_GetXSize());
    }
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE); 
}

void update_wind_direction(int mode, int firsttime){

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    if(mode == 0){
        BSP_LCD_ClearStringLine(13);
        BSP_LCD_DrawHLine(0, 202, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(13, (uint8_t*) "WIND - DIR : Fix");
        BSP_LCD_DrawHLine(0, 225, BSP_LCD_GetXSize());
    } else if(mode == 1){
        BSP_LCD_ClearStringLine(13);
        BSP_LCD_DrawHLine(0, 202, BSP_LCD_GetXSize());
        BSP_LCD_DisplayStringAtLine(13, (uint8_t*) "WIND - DIR: SWING");
        BSP_LCD_DrawHLine(0, 225, BSP_LCD_GetXSize());
    }
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE); 
}

void touchscreen_display(){
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    /* Set Touchscreen Demo1 description */
    // BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize()); 
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK); 
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"A/C Simulator", CENTER_MODE);
    BSP_LCD_DrawHLine(0, 33, BSP_LCD_GetXSize());
    
    //STATUS
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAtLine(3,(uint8_t*) " ** Press a button ** ");
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    
    BSP_LCD_SetFont(&Font12);
    
}

void get_the_payload(char payload[]){

    // printf("PAYLOAD : %s\n", payload);
    
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

    char TWENTY[] = "20";
    char TWENTY_ONE[] = "21";
    char TWENTY_TWO[] = "22";
    char TWENTY_THREE[] = "23";
    char TWENTY_FOUR[] = "24";
    char TWENTY_FIVE[] = "25";
    char TWENTY_SIX[] = "26";
    char TWENTY_SEVEN[] = "27";
    char TWENTY_EIGHT[] = "28";
    char TWENTY_NINE[] = "29";
    char THIRTY[] = "30";

    char FAN_SPEED_AUTO[] = "00";
    char FAN_SPEED_LOW[] = "01";
    char FAN_SPEED_MEDIUM[] = "02";
    char FAN_SPEED_HIGH[] = "03";
    char FAN_SPEED_TURBO[] = "04";

    char WIN_DIRECTION_FIX[] = "fix";
    char WIN_DIRECTION_SWING[] = "swing";

    //STRSTR_VARABLE
    char *ON_STRSTR;
    char *OFF_STRSTR;

    char *AUTO_STRSTR;
    char *COOL_STRSTR;
    char *DRY_STRSTR;

    char *TWENTY_STRSTR;
    char *TWENTY_STRTSTR;
    char *TWENTY_ONE_STRSTR;
    char *TWENTY_TWO_STRSTR;
    char *TWENTY_THREE_STRSTR;
    char *TWENTY_FOUR_STRSTR;
    char *TWENTY_FIVE_STRSTR;
    char *TWENTY_SIX_STRSTR;
    char *TWENTY_SEVEN_STRSTR;
    char *TWENTY_EIGHT_STRSTR;
    char *TWENTY_NINE_STRSTR;
    char *THIRTY_STRSTR;

    char *FAN_SPEED_AUTO_STRSTR, *FAN_SPEED_LOW_STRSTR, *FAN_SPEED_MEDIUM_STRSTR, *FAN_SPEED_HIGH_STRSTR, *FAN_SPEED_TURBO_STRSTR;

    char *WIN_DIRECTION_FIX_STRSTR, *WIN_DIRECTION_SWING_STRSTR;

    //STRSTR
    ON_STRSTR = strstr(payload, ON);
    OFF_STRSTR = strstr(payload, OFF);
    AUTO_STRSTR = strstr(payload, AUTO);
    COOL_STRSTR = strstr(payload, COOL);
    DRY_STRSTR = strstr(payload, DRY);

    TWENTY_STRSTR = strstr(payload, TWENTY);
    TWENTY_ONE_STRSTR = strstr(payload, TWENTY_ONE);
    TWENTY_TWO_STRSTR = strstr(payload, TWENTY_TWO);
    TWENTY_THREE_STRSTR = strstr(payload, TWENTY_THREE);
    TWENTY_FOUR_STRSTR = strstr(payload, TWENTY_FOUR);
    TWENTY_FIVE_STRSTR = strstr(payload, TWENTY_FIVE);
    TWENTY_SIX_STRSTR = strstr(payload, TWENTY_SIX);
    TWENTY_SEVEN_STRSTR = strstr(payload, TWENTY_SEVEN);
    TWENTY_EIGHT_STRSTR = strstr(payload, TWENTY_EIGHT);
    TWENTY_NINE_STRSTR = strstr(payload, TWENTY_NINE);
    THIRTY_STRSTR = strstr(payload, THIRTY);

    FAN_SPEED_AUTO_STRSTR = strstr(payload, FAN_SPEED_AUTO);
    FAN_SPEED_LOW_STRSTR = strstr(payload, FAN_SPEED_LOW);
    FAN_SPEED_MEDIUM_STRSTR = strstr(payload, FAN_SPEED_MEDIUM);
    FAN_SPEED_HIGH_STRSTR = strstr(payload, FAN_SPEED_HIGH);
    FAN_SPEED_TURBO_STRSTR = strstr(payload, FAN_SPEED_TURBO);

    WIN_DIRECTION_FIX_STRSTR = strstr(payload, WIN_DIRECTION_FIX);
    WIN_DIRECTION_SWING_STRSTR = strstr(payload, WIN_DIRECTION_SWING);

    //CHECK_PRESSED_BUTTON
    if(ON_STRSTR){
        update_power_status(1, FIRST_TIME_DISPLAY_STATUS);
        FIRST_TIME_DISPLAY_STATUS += 1;
        printf("You press ON\n");
    }

    else if(OFF_STRSTR){
        update_power_status(0, FIRST_TIME_DISPLAY_STATUS);
        FIRST_TIME_DISPLAY_STATUS += 1;
        printf("You press OFF\n");
    }

    else if(AUTO_STRSTR){
        update_mode(0, FIRST_TIME_DISPLAY_MODE);
        FIRST_TIME_DISPLAY_MODE += 1;
        printf("You press AUTO\n");
    }

    else if(COOL_STRSTR){
        update_mode(1, FIRST_TIME_DISPLAY_MODE);
        FIRST_TIME_DISPLAY_MODE += 1;
        printf("You press COOL\n");
    }

    else if(DRY_STRSTR){
        update_mode(2, FIRST_TIME_DISPLAY_MODE);
        FIRST_TIME_DISPLAY_MODE += 1;
        printf("You press DRY\n");
    }

    else if(TWENTY_STRSTR){
        update_temp((uint8_t *) "20");
        printf("You press 20\n");
    }

    else if(TWENTY_ONE_STRSTR){
        update_temp((uint8_t *) "21");
        printf("You press 21\n");
    }

    else if(TWENTY_TWO_STRSTR){
        update_temp((uint8_t *) "22");
        printf("You press 22\n");
    }

    else if(TWENTY_THREE_STRSTR){
        update_temp((uint8_t *) "23");
        printf("You press 23\n");
    }

    else if(TWENTY_FOUR_STRSTR){
        update_temp((uint8_t *) "24");
        printf("You press 24\n");
    }

    else if(TWENTY_FIVE_STRSTR){
        update_temp((uint8_t *) "25");
        printf("You press 25\n");
    }

    else if(TWENTY_SIX_STRSTR){
        update_temp((uint8_t *) "26");
        printf("You press 26\n");
    }

    else if(TWENTY_SEVEN_STRSTR){
        update_temp((uint8_t *) "27");
        printf("You press 27\n");
    }

    else if(TWENTY_EIGHT_STRSTR){
        update_temp((uint8_t *) "28");
        printf("You press 28\n");
    }

    else if(TWENTY_NINE_STRSTR){
        update_temp((uint8_t *) "29");
        printf("You press 29\n");
    }

    else if(THIRTY_STRSTR){
        update_temp((uint8_t *) "30");
        printf("You press 30\n");
    }

    else if(FAN_SPEED_AUTO_STRSTR){
        update_fanspeed(0, FIRST_TIME_DISPLAY_FANSPEED);
        FIRST_TIME_DISPLAY_FANSPEED += 1;
        printf("You press FAN SPEED Auto\n");
    }

    else if(FAN_SPEED_LOW_STRSTR){
        update_fanspeed(1, FIRST_TIME_DISPLAY_FANSPEED);
        FIRST_TIME_DISPLAY_FANSPEED += 1;
        printf("You press FAN SPEED Low\n");
    }

    else if(FAN_SPEED_MEDIUM_STRSTR){
        update_fanspeed(2, FIRST_TIME_DISPLAY_FANSPEED);
        FIRST_TIME_DISPLAY_FANSPEED += 1;
        printf("You press FAN SPEED Medium\n");
    }

    else if(FAN_SPEED_HIGH_STRSTR){
        update_fanspeed(3, FIRST_TIME_DISPLAY_FANSPEED);
        FIRST_TIME_DISPLAY_FANSPEED += 1;
        printf("You press FAN SPEED High\n");
    }

    else if(FAN_SPEED_TURBO_STRSTR){
        update_fanspeed(4, FIRST_TIME_DISPLAY_FANSPEED);
        FIRST_TIME_DISPLAY_FANSPEED += 1;
        printf("You press FAN SPEED Turbo\n");
    } 
    
    else if(WIN_DIRECTION_FIX_STRSTR){
        update_wind_direction(0, FIRST_TIME_DISPLAY_WIND_DIRECTION);
        FIRST_TIME_DISPLAY_WIND_DIRECTION += 1;
        printf("You press WIND Direction Fix\n");
    }

    else if(WIN_DIRECTION_SWING_STRSTR){
        update_wind_direction(1, FIRST_TIME_DISPLAY_WIND_DIRECTION);
        FIRST_TIME_DISPLAY_WIND_DIRECTION += 1;
        printf("You press WIND Direction Swing\n");
    }

    else {
        printf("MAI JER\n");
    }
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

    int firsttime = 0;
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


    if ((rc = client.connect(data)) != 0)
    {
        printf("[Subscribe]rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("[Subscribe] Client Connected.\r\n");
    }

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

    int test_subscribe = 0;
    while(true){
        client.yield(10000);
        printf("Test subscribe :: %d\n", test_subscribe);
        test_subscribe += 1; 
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