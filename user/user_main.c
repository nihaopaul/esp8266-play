#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"



#include "c_types.h"
#include "uart.h"
#include "espconn.h"
#include "mem.h"







#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
#define TCPSERVERIP "10.0.10.82"
#define TCPSERVERPORT 9999


os_event_t    user_procTaskQueue[user_procTaskQueueLen];


static void user_procTask(os_event_t *events);
static void loop(os_event_t *events);
// void uart0_send_str(uint8 *buf);

static volatile os_timer_t some_timer;

//Main code function
static void ICACHE_FLASH_ATTR loop(os_event_t *events) {
  os_printf("Hello\n\r");
  os_delay_us(10000);
  system_os_post(user_procTaskPrio, 0, 0 );
}


static void ICACHE_FLASH_ATTR at_tcpclient_sent_cb(void *arg) {

     // The data sent, disconnected from the TCP-server
    struct espconn *pespconn = (struct espconn *)arg;
    espconn_disconnect(pespconn);
}
 
static void ICACHE_FLASH_ATTR at_tcpclient_discon_cb(void *arg) {
    struct espconn *pespconn = (struct espconn *)arg;
     // Disable, frees up memory
    os_free(pespconn->proto.tcp);
    os_free(pespconn);
}


static void ICACHE_FLASH_ATTR at_tcpclient_connect_cb(void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

     // Callback function that is called after sending data
    espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb);
     // Callback function that is called after disconnection
    espconn_regist_disconcb(pespconn, at_tcpclient_discon_cb);
    char payload[128];
     // Prepare the data string will send the MAC address of ESP8266 in AP mode and add the line ESP8266
    os_sprintf(payload, MACSTR ",%s\r\n", "Fuxx", "ESP8266");

     // Send data
    espconn_sent(pespconn, payload, strlen(payload));
}



static void ICACHE_FLASH_ATTR senddata() {


  char tcpserverip[15];
  struct espconn *pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
  if (pCon == NULL)
  {
      #ifdef PLATFORM_DEBUG
      uart0_sendStr("TCP connect failed\r\n");
      #endif
      return;
  }
  pCon->type = ESPCONN_TCP;
  pCon->state = ESPCONN_NONE;
   // Set address TCP-based server where the data will be send
  os_sprintf(tcpserverip, "%s", TCPSERVERIP);


  uint32_t ip = ipaddr_addr(tcpserverip);
  pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  pCon->proto.tcp->local_port = espconn_port();
   // Set the port TCP-based server, which will send data
  pCon->proto.tcp->remote_port = TCPSERVERPORT;
  os_memcpy(pCon->proto.tcp->remote_ip, &ip, 4);
   // Register the callback function to be called when the connection is established
  espconn_regist_connectcb (pCon, at_tcpclient_connect_cb);
   // You can register a callback function to be called when rekonekte, but we do not need it yet
   // espconn_regist_reconcb (pCon, at_tcpclient_recon_cb);
   // Display debugging information

   // Connect to the TCP-server
  espconn_connect(pCon);
}



void some_timerfunc(void *arg) {
  //Do blinky stuff
  if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2) {
    //Set GPIO2 to LOW
    gpio_output_set(0, BIT2, BIT2, 0);
  } else {
    //Set GPIO2 to HIGH
    gpio_output_set(BIT2, 0, BIT2, 0);

    senddata();
  }
}

static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
  os_delay_us(10);
}

//Init function 
void ICACHE_FLASH_ATTR user_init() {


  // uart_init(BIT_RATE_115200,BIT_RATE_115200); // start the UART
   



  char ssid[32] = SSID;
  char password[64] = SSID_PASSWORD;
  struct station_config stationConf;

  //Set station mode
  wifi_set_opmode( 0x1 );

  //Set ap settings
  os_memcpy(&stationConf.ssid, ssid, 32);
  os_memcpy(&stationConf.password, password, 64);
  wifi_station_set_config(&stationConf);


  // Initialize the GPIO subsystem.
  gpio_init();

  //Set GPIO2 to output mode
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

  //Set GPIO2 low - this is D4
  gpio_output_set(0, BIT2, BIT2, 0);

  //Disarm timer
  os_timer_disarm(&some_timer);

  //Setup timer
  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

  //Arm the timer
  //&some_timer is the pointer
  //1000 is the fire time in ms
  //0 for once and 1 for repeating
  os_timer_arm(&some_timer, 1000, 1);

  //Start os task
  system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

  //Start os task
  system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);




  system_os_post(user_procTaskPrio, 0, 0 );




}
