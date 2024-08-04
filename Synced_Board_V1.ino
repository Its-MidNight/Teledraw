#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <esp_now.h>
#include <WiFi.h>

const uint8_t PROGMEM broadcastAddress[] = {0x1C,0x69,0x20,0x2C,0x18,0x98};//Replace the mac with the OTHER board's mac address e.g programming for board #1, input board #2's mac.Vice versa


TaskHandle_t Task0;
TaskHandle_t Task1;

#define T_CS 5
#define T_IRQ 16

#define BUTTON 22
/*Connections
  T_IRQ->16
  T_DO->D19
  T_DIN->D23
  T_CS->D5
  T_CLK->D18
  MISO->No connection
  LED->3V3
  SCK->D18
  MOSI->D23
  DC->D2
  RESET->D4
  CS->D15
*/

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(T_CS,T_IRQ);

int16_t calData[4]={5000/*X_MIN*/,5000/*Y_MIN*/,0/*X_MAX*/,0/*Y_MAX*/};
int lineCursors[4]; 
int receivedCursors[4]={-1,0,0,0};
bool syncing=0;
bool calibrated=0;
bool pressed=0;

void updateLine(){
  lineCursors[2]=lineCursors[0];
  lineCursors[3]=lineCursors[1];
  TS_Point p = ts.getPoint();
  lineCursors[0]=p.x;
  lineCursors[1]=p.y;
}

void translate(){
  int displayCursors[4];
  displayCursors[0]=map(lineCursors[0],calData[0],calData[2],0,tft.width());
  displayCursors[1]=map(lineCursors[1],calData[1],calData[3],0,tft.height());
  displayCursors[2]=map(lineCursors[2],calData[0],calData[2],0,tft.width());
  displayCursors[3]=map(lineCursors[3],calData[1],calData[3],0,tft.height());
  int sendCursors[2];
  for(int i=0;i<2;i++)sendCursors[i]=displayCursors[i];
  esp_now_send(broadcastAddress, (uint8_t *) &sendCursors, 8);
  tft.drawWideLine(displayCursors[0],displayCursors[1],displayCursors[2],displayCursors[3],2,TFT_WHITE,TFT_WHITE);
} 

void touchCalibrate(int16_t (*calibP)[4], int confirmPin){
  pinMode(confirmPin,INPUT_PULLUP);
  tft.drawCentreString("Calibrating",tft.width()/2,tft.height()/2,2);
  tft.drawCentreString("XMIN:",tft.width()/2-15,tft.height()/2-15,2);
  tft.drawCentreString("YMIN:",tft.width()/2-15,tft.height()/2-30,2);
  while(digitalRead(confirmPin)){
    vTaskDelay(1);
    if(ts.tirqTouched()||ts.touched()){
      TS_Point p=ts.getPoint();
      if(p.x<*(*calibP)&&p.x!=0) *(*calibP)=p.x;
      if(p.y<*(*calibP+1)&&p.y!=0) *(*calibP+1)=p.y;  
    }
    tft.setCursor(tft.width()/2+5,tft.height()/2-10);
    tft.printf("%d ",*(*calibP));
    tft.setCursor(tft.width()/2+5,tft.height()/2-25);
    tft.printf("%d ",*(*calibP+1));
  }
  tft.setCursor(tft.width()/2+5,tft.height()/2-10);
  tft.print("    ");
  tft.setCursor(tft.width()/2+5,tft.height()/2-25);
  tft.printf("    ");
  while(!digitalRead(confirmPin))vTaskDelay(1);;

  tft.drawCentreString("XMAX:",tft.width()/2-15,tft.height()/2-15,2);
  tft.drawCentreString("YMAX:",tft.width()/2-15,tft.height()/2-30,2);
  while(digitalRead(confirmPin)){
    vTaskDelay(1);
    if(ts.tirqTouched()||ts.touched()){
      TS_Point p=ts.getPoint();
      if(p.x>*(*calibP+2)/*&&p.x!=0*/) *(*calibP+2)=p.x;
      if(p.y>*(*calibP+3)/*&&p.y!=0*/) *(*calibP+3)=p.y;  
    }
    tft.setCursor(tft.width()/2+5,tft.height()/2-10);
    tft.printf("%d ",*(*calibP+2));
    tft.setCursor(tft.width()/2+5,tft.height()/2-25);
    tft.printf("%d ",*(*calibP+3));
  }
  calibrated=1;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    //Serial.println("Delivery Success :)");
  }
  else{
    //Serial.println("Delivery Fail :(");
  }
}

// 收到消息的回调函数
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  for(int i=0;i<2;i++)receivedCursors[i+2]=receivedCursors[i];
  memcpy(&receivedCursors, incomingData, 8);
  syncing=1;
}

void setup() {
  Serial.begin(115200);
  xTaskCreatePinnedToCore(
                    Task0code,   /* Task function. */
                    "Task0",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task0,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 
}

void Task0code(void * pvParameters){
  tft.init();
  ts.begin();
  tft.setRotation(0);
  ts.setRotation(2-tft.getRotation()%2);
  tft.fillScreen(TFT_BLACK);
  touchCalibrate(&calData,BUTTON);
  /*Serial.println(calData[0]);
  Serial.println(calData[1]);
  Serial.println(calData[2]);
  Serial.println(calData[3]);*/
  tft.fillScreen(TFT_BLACK);
  int relCount=2;
  for(;;){
    vTaskDelay(1);
    if(ts.tirqTouched()){
      relCount=0;
      while(ts.touched()){
        relCount=0;
        updateLine();
        if(lineCursors[2]!=0||lineCursors[3]!=0){
          translate();
          delay(20);//The best value
        }
      }
    }else{
      for(int i=0;i<4;i++)lineCursors[i]=0;
      int noTouch[2]={-1,0};
      if(relCount<2){
        esp_now_send(broadcastAddress, (uint8_t *) &noTouch, 8);
        relCount++;
      }
    }
    if(!digitalRead(BUTTON)&&!pressed){
      int clearScreen[2]={0,-1};
      esp_now_send(broadcastAddress, (uint8_t *) &clearScreen, 8);
      tft.fillScreen(TFT_BLACK);
      pressed=1;
    }else pressed=0;
  }
}

void Task1code(void * pvParameters){
  // 设置ESP32为STA模式
  WiFi.mode(WIFI_STA);

  //初始化 ESP-NOW
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }

//注册发送回调函数
  esp_now_register_send_cb(OnDataSent);
  
  // 注册通信频道
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
         
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    return;
  }
  //注册接收回调函数
  esp_now_register_recv_cb(OnDataRecv);

  for(;;){
    vTaskDelay(1);
    if(syncing&&calibrated){
      /*Serial.printf("RecX1:%d\n",receivedCursors[0]);
      Serial.printf("RecY1:%d\n",receivedCursors[1]);
      Serial.printf("RecX2:%d\n",receivedCursors[2]);
      Serial.printf("RecY2:%d\n",receivedCursors[3]);*/
      if(receivedCursors[0]>=0&&receivedCursors[1]>=0&&receivedCursors[2]>=0&&receivedCursors[3]>=0)tft.drawWideLine(receivedCursors[0],receivedCursors[1],receivedCursors[2],receivedCursors[3],2,TFT_WHITE,TFT_WHITE);
      else if(receivedCursors[1]==-1)tft.fillScreen(TFT_BLACK);
      syncing=0;
    }
  }
}

void loop() {
  
}
