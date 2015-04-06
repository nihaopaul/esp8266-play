#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];


static void user_procTask(os_event_t *events);
static void loop(os_event_t *events);

static volatile os_timer_t some_timer;

//Main code function
static void ICACHE_FLASH_ATTR loop(os_event_t *events) {
  os_printf("Hello\n\r");
  os_delay_us(10000);
  system_os_post(user_procTaskPrio, 0, 0 );
}

void some_timerfunc(void *arg) {
  //Do blinky stuff
  if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2) {
    //Set GPIO2 to LOW
    gpio_output_set(0, BIT2, BIT2, 0);
  } else {
    //Set GPIO2 to HIGH
    gpio_output_set(BIT2, 0, BIT2, 0);
  }
}

static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
  os_delay_us(10);
}

//Init function 
void ICACHE_FLASH_ATTR user_init() {
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


