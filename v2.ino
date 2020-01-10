#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>


//24*24小电视
const unsigned char bilibilitv_24u[] U8X8_PROGMEM = {0x00, 0x00, 0x02, 0x00, 0x00, 0x03, 0x30, 0x00, 0x01, 0xe0, 0x80, 0x01,
                                                     0x80, 0xc3, 0x00, 0x00, 0xef, 0x00, 0xff, 0xff, 0xff, 0x03, 0x00, 0xc0, 0xf9, 0xff, 0xdf, 0x09, 0x00, 0xd0, 0x09, 0x00, 0xd0, 0x89, 0xc1,
                                                     0xd1, 0xe9, 0x81, 0xd3, 0x69, 0x00, 0xd6, 0x09, 0x91, 0xd0, 0x09, 0xdb, 0xd0, 0x09, 0x7e, 0xd0, 0x0d, 0x00, 0xd0, 0x4d, 0x89, 0xdb, 0xfb,
                                                     0xff, 0xdf, 0x03, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x78, 0x00, 0x1e, 0x30, 0x00, 0x0c
                                                    };



//30*30小电视
const unsigned char bilibilitv_30u[] U8X8_PROGMEM = {0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x30, 0x00,
                                                     0xe0, 0x00, 0x38, 0x00, 0xc0, 0x07, 0x1c, 0x00, 0x00, 0x0f, 0x0c, 0x00, 0x00, 0x3e, 0x0e, 0x00, 0xff, 0xfd, 0xff, 0x1f, 0xff, 0xe3, 0xfb,
                                                     0x3f, 0x03, 0x00, 0x00, 0x30, 0xf3, 0xff, 0xff, 0x33, 0x3b, 0x00, 0x00, 0x37, 0x1b, 0x00, 0x00, 0x36, 0x1b, 0x00, 0x0c, 0x36, 0x1b, 0x0f,
                                                     0x1c, 0x36, 0x9b, 0x07, 0x78, 0x36, 0x9b, 0x01, 0x70, 0x36, 0x1b, 0x00, 0x40, 0x36, 0x1b, 0xcc, 0x0c, 0x36, 0x1b, 0xdc, 0x0e, 0x36, 0x1b,
                                                     0xf8, 0x07, 0x36, 0x1b, 0x30, 0x03, 0x36, 0x1b, 0x00, 0x00, 0x3e, 0xfb, 0xff, 0xff, 0x37, 0xf3, 0xff, 0xff, 0x33, 0xe7, 0xfe, 0xff, 0x38,
                                                     0xff, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x3f, 0xf0, 0x01, 0xc0, 0x03, 0xe0, 0x00, 0xc0, 0x03
                                                    };


//定义B站UID号码
char bilibiliUID[] = "427494870";
//粉丝计数
long fansCounts = 0;





U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 2, /* data=*/ 14, /* cs=*/ 15, /* dc=*/ 13, /* reset=*/ 12 );  // Enable U8G2_16BIT in u8g2.h
HTTPClient http;
WiFiClient client;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const unsigned long HTTP_TIMEOUT = 5000;
bool autoConfig()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin();                    // 默认连接保存的WIFI
  for (int i = 0; i < 20; i++)
  {
    int wstatus = WiFi.status();
    if (wstatus == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      //Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      //Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.printDiag(Serial);
      return true;

    }
    else
    {
      Serial.print("AutoConfig Waiting...");
      Serial.println(wstatus);
      delay(1000);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;

}
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      //Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      //Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);  // 设置自动连接
      break;
    }
    delay(1000); // 这个地方一定要加延时，否则极易崩溃重启
  }
}


void getFansCounts()
{
  if (WiFi.status() == WL_CONNECTED) {
    const char* HOST = "http://api.bilibili.com";
    String GetUrl1 = String(HOST) + "/x/relation/stat?vmid=";
    GetUrl1 += bilibiliUID;
    http.begin(GetUrl1);
    int httpCode = http.GET();
    Serial.printf("code = %d\r\n", httpCode);
    if (httpCode > 0) {
      String resBuff = http.getString();
      Serial.println(resBuff);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(resBuff);
      if (!root.success())
      {
        Serial.println("parseObject() failed");
        return;
      }
      long fans = root["data"]["follower"];
      Serial.println(fans);

      u8g2.clearDisplay();
      u8g2.setFont(u8g2_font_unifont_t_Ikcfonts_24u); // choose a suitable font  (设置字体)
      u8g2.setCursor(26, 24);
      u8g2.print("硬核拆解当前粉丝数");
      u8g2.setFont(u8g2_font_unifont_t_Ikcfonts_32u); // choose a suitable font  (设置字体)
      u8g2.drawXBMP( 50 , 29 , 24 , 24 , bilibilitv_24u );     //根据你的图片尺寸修改
      u8g2.setCursor(81, 53);
      u8g2.print(fans);
      u8g2.sendBuffer();
      fansCounts = fans;
      delay(1500);
    }
  }
}




void setup() {

  Serial.begin(9600);
  u8g2.begin();
  u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function

  //test
  // u8g2.clearBuffer();           // 清空显示设备内部缓冲区
  //  u8g2.drawXBMP( 0 , 0 , 24 , 24 , bilibilitv_24u );     //根据你的图片尺寸修改
  //  u8g2.sendBuffer();         // 显示缓冲区内容
  //  delay(111111000);





  //page1
  u8g2.setFont(u8g2_font_unifont_t_Ikcfonts_24u); // choose a suitable font  (设置字体)
  u8g2.drawXBMP( 16 , 9 , 24 , 24 , bilibilitv_24u );     //根据你的图片尺寸修改
  u8g2.setCursor(57, 31);
  u8g2.print("Emo Teardown");
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a); // choose a suitable font  (设置字体)
  u8g2.setCursor(0, 63);
  u8g2.print("Version 1.4-200105 +08Preview");
  u8g2.sendBuffer();
  delay(3500);

  //page2
  u8g2.clearDisplay();
  u8g2.setFont(u8g2_font_unifont_t_Ikcfonts_24u); // choose a suitable font  (设置字体)
  u8g2.drawXBMP( 54 , 9 , 24 , 24 , bilibilitv_24u );     //根据你的图片尺寸修改
  u8g2.setCursor(94, 31);
  u8g2.print("硬核拆解");
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a); // choose a suitable font  (设置字体)
  u8g2.setCursor(0, 63);
  u8g2.print("版本 1.4-200105 +08预览版");
  u8g2.sendBuffer();
  delay(3500);

  //page3
  u8g2.clearDisplay();
  u8g2.setFont(u8g2_font_unifont_t_Ikcfonts_24u); // choose a suitable font  (设置字体)
  u8g2.drawXBMP( 16 , 9 , 24 , 24 , bilibilitv_24u );     //根据你的图片尺寸修改
  u8g2.setCursor(57, 31);
  u8g2.print("B站十万粉丝纪念");
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a); // choose a suitable font  (设置字体)
  u8g2.setCursor(0, 63);
  u8g2.print("Powered by iCOMgx by Atai");
  u8g2.sendBuffer();
  delay(3500);


  u8g2.clearDisplay();
  u8g2.setCursor(0, 28);
  u8g2.print("Wifi Connecting...");
  delay(2000);
  u8g2.sendBuffer();
  if (!autoConfig())
  {
    u8g2.clearDisplay();
    Serial.println("Start module");
    u8g2.setCursor(0, 28);
    u8g2.print("Connecting Faild...");
    u8g2.setCursor(0, 42);
    u8g2.print("Start SmartConfig");
    u8g2.sendBuffer();
    smartConfig();
  }

  delay(1000);
  u8g2.clearDisplay();
  u8g2.setCursor(0, 14);
  u8g2.print("Wifi Connected");
  u8g2.setCursor(0, 28);
  u8g2.print("Connected SSID:");
  u8g2.setCursor(90, 28);
  u8g2.print(WiFi.SSID().c_str());
  u8g2.setCursor(0, 40);
  u8g2.print("Local IP:");
  u8g2.setCursor(52, 40);
  u8g2.print(WiFi.localIP());
  u8g2.setCursor(0, 52);
  u8g2.print("System Begin...");
  u8g2.sendBuffer();
  delay(2000);
  u8g2.clearDisplay();
  http.setTimeout(HTTP_TIMEOUT);
  getFansCounts();
  httpUpdater.setup(&httpServer);
  httpServer.begin();

  Serial.print("HTTPUpdateServer ready! Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/update in your browser");


}

void loop() {
  httpServer.handleClient(); //网页更新

  if (WiFi.status() == WL_CONNECTED) {
    const char* HOST = "http://api.bilibili.com";
    String GetUrl1 = String(HOST) + "/x/relation/stat?vmid=";
    GetUrl1 += bilibiliUID;
    http.begin(GetUrl1);
    int httpCode = http.GET();
    Serial.printf("code = %d\r\n", httpCode);
    if (httpCode > 0) {
      String resBuff = http.getString();
      Serial.println(resBuff);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(resBuff);
      if (!root.success())
      {
        Serial.println("parseObject() failed");
        return;
      }
      long fans = root["data"]["follower"];
      Serial.println(fans);
      if (fans != fansCounts)
      {
        u8g2.clearDisplay();
        u8g2.setFont(u8g2_font_unifont_t_Ikcfonts_24u); // choose a suitable font  (设置字体)
        u8g2.setCursor(26, 24);
        u8g2.print("硬核拆解当前粉丝数");
        u8g2.setFont(u8g2_font_unifont_t_Ikcfonts_32u); // choose a suitable font  (设置字体)
        u8g2.drawXBMP( 50 , 29 , 24 , 24 , bilibilitv_24u );     //根据你的图片尺寸修改
        u8g2.setCursor(81, 53);
        u8g2.print(fans);
        u8g2.sendBuffer();
        fansCounts = fans;
      }
      delay(1000);
    }
  }
}
