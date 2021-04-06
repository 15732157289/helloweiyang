#include "wifi.h"
#include <SoftwareSerial.h>
#include "mcu_api.h"              //引用Tuya




SoftwareSerial mySerial(14, 12); // RX, TX
#define _SS_MAX_RX_BUFF 300
char LED_Switch;
int time_cnt = 0, cnt = 0, init_flag = 0;

#define PC13 13
void setup() {
  
  pinMode(relay, OUTPUT);   // 继电器I/O初始化
  

  pinMode(PC13, INPUT);     // 重置Wi-Fi按键初始化
  pinMode(4, OUTPUT);       // Wi-Fi状态指示灯初始化

  mySerial.begin(9600);     // 软件串口初始化
  mySerial.println("myserial init successful!");
  Serial.begin(115200);     // PA3 RX   PA2 TX
  Serial.println("serial init successful!");

  wifi_protocol_init();
}

void loop() {
  if (init_flag == 0) {
    time_cnt++;
    if (time_cnt % 6000 == 0) {
      time_cnt = 0;
      cnt ++;
    }
    wifi_stat_led(&cnt);   // Wi-Fi状态处理
  }
  wifi_uart_service();
  myserialEvent();      // 串口接收处理
  key_scan();           // 重置配网按键检测
  LED_Control();            //水泵控制
}

// 串口接收处理
void myserialEvent() {
  if (mySerial.available()) {
    unsigned char value = (unsigned char)mySerial.read();
    uart_receive_input(value);
  }
}

void key_scan(void)
{
  static char ap_ez_change = 0;
  unsigned char buttonState  = HIGH;
  buttonState = digitalRead(PC13);
  if (buttonState == LOW) {
    delay(3000);
    buttonState = digitalRead(PC13);
      printf("----%d",buttonState);
    if (buttonState == LOW) {
      printf("123\r\n");
      init_flag = 0;
      switch (ap_ez_change) {
        case 0 :
          mcu_set_wifi_mode(SMART_CONFIG);
          break;
        case 1 :
          mcu_set_wifi_mode(AP_CONFIG);
          break;
        default:
          break;
      }
      ap_ez_change = !ap_ez_change;
    }

  }
}

void wifi_stat_led(int *cnt)
{
#define wifi_stat_led 4
  switch (mcu_get_wifi_work_state())
  {
    case SMART_CONFIG_STATE:  //0x00
      init_flag = 0;
      if (*cnt == 2) {
        *cnt = 0;
      }
      if (*cnt % 2 == 0)  //LED快闪
      {
        digitalWrite(wifi_stat_led, LOW);
      }
      else
      {
        digitalWrite(wifi_stat_led, HIGH);
      }
      break;
    case AP_STATE:  //0x01
      init_flag = 0;
      if (*cnt >= 30) {
        *cnt = 0;
      }
      if (*cnt  == 0)      // LED 慢闪
      {
        digitalWrite(wifi_stat_led, LOW);
      }
      else if (*cnt == 15)
      {
        digitalWrite(wifi_stat_led, HIGH);
      }
      break;

    case WIFI_NOT_CONNECTED:  // 0x02
      digitalWrite(wifi_stat_led, HIGH); // LED 熄灭
      break;
    case WIFI_CONNECTED:  // 0x03
      break;
    case WIFI_CONN_CLOUD:  // 0x04
      if ( 0 == init_flag )
      {
        digitalWrite(wifi_stat_led, LOW);// LED 常亮
        init_flag = 1;                  // Wi-Fi 连接上后该灯可控
        *cnt = 0;
      }

      break;

    default:
      digitalWrite(wifi_stat_led, HIGH);
      break;
  }
}

//UV杀菌控制
void LED_Control(){
  //当UV杀菌开关打开时开启UVC灯
  if (LED_Switch  == 1){
    digitalWrite(relay, HIGH);
  }
  else{
    digitalWrite(relay, LOW);
  }
}
