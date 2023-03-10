#include "TimeUtils.h"
#include "WeatherUtils.h"
#include <NTPClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "DHTesp.h"
#include "mem.h"
//#include "I2Cdev.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include "i0.h"
#include "i1.h"
#include "i2.h"
#include "i3.h"
#include "i4.h"
#include "i5.h"
#include "i6.h"
#include "i7.h"
#include "i8.h"
#include "i9.h"


#include "temperature.h"
#include "humidity.h"
#include "watch_top.h"
#include "watch_bottom.h"

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif




TFT_eSPI tft = TFT_eSPI();
uint16_t back = tft.color565(248, 248, 255);

const char* SSID = "330";				// 要连接的WiFi的名称
const char* PASSWORD = "abcdefg12345";		// 要连接的WiFi的密码
const char* CITY = "JinZhong";


bool ICACHE_FLASH_ATTR check_memleak_debug_enable (void)
{
  return MEMLEAK_DEBUG_ENABLE;
}


bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

TimeClient timeClient;
TimeData timeData, lastTime, lastWeather;

WeatherClient weatherClient;
WeatherData weatherData;

Adafruit_MPU6050 mpu;
int i = 1;


uint8_t cube_n = 0;
void cube() {
  tft.drawFastVLine(10 + cube_n, 13, 10, TFT_BLACK);
  cube_n++;
  if (cube_n > 60) {
    cube_n = 0;
    tft.fillRoundRect(10, 13, 61, 10, 0, back);
  }
}

String ShowWeeks(int x)
{
  switch (x) {
    case 1:
      tft.print("Mon"); break;
    case 2:
      tft.print("Tues"); break;
    case 3:
      tft.print("Wed"); break;
    case 4:
      tft.print("Thur"); break;
    case 5:
      tft.print("Fri"); break;
    case 6:
      tft.print("Sat"); break;
    case 7:
      tft.print("Sun"); break;
  }
}


//画太空人
void imgAnim() {
  int x = 65, y = 50, dt = 30; //瘦子版dt=10毫秒 胖子30较为合适

  TJpgDec.drawJpg(x, y, i0, sizeof(i0));
  delay(dt);
  TJpgDec.drawJpg(x, y, i1, sizeof(i1));
  delay(dt);
  TJpgDec.drawJpg(x, y, i2, sizeof(i2));
  delay(dt);
  TJpgDec.drawJpg(x, y, i3, sizeof(i3));
  delay(dt);
  TJpgDec.drawJpg(x, y, i4, sizeof(i4));
  delay(dt);
  TJpgDec.drawJpg(x, y, i5, sizeof(i5));
  delay(dt);
  TJpgDec.drawJpg(x, y, i6, sizeof(i6));
  delay(dt);
  TJpgDec.drawJpg(x, y, i7, sizeof(i7));
  delay(dt);
  TJpgDec.drawJpg(x, y, i8, sizeof(i8));
  delay(dt);
  TJpgDec.drawJpg(x, y, i9, sizeof(i9));
  delay(dt);

}
//改字体颜色
 /*void ChangeColor(int i)
{
  switch (i) {
    case 1:
      Serial.print(i);
      tft.setTextColor(TFT_NAVY, TFT_WHITE);
      i = 2;
      Serial.print("更换成功1");
      Serial.print("\n");
      Serial.print(i);
      break;
    case 2:
      tft.setTextColor(TFT_VIOLET, TFT_WHITE);
      i = 3;
      Serial.print("更换成功2");
      Serial.print(i);
      Serial.print("\n");
      Serial.print(i);
      break;
    case 3:
      tft.setTextColor(TFT_CYAN, TFT_WHITE);
      i = 4;
      Serial.print("更换成功3");
      Serial.print(i);
      Serial.print("\n");
      Serial.print(i);
      break;
    case 4:
      tft.setTextColor(TFT_GREENYELLOW, TFT_WHITE);
      i = 5;
      Serial.print("更换成功4");
      Serial.print(i);
      Serial.print("\n");
      Serial.print(i);
      break;
    case 5:
      tft.setTextColor(TFT_GOLD, TFT_WHITE);
      i = 6;
      Serial.print("更换成功5");
      Serial.print(i);
      Serial.print("\n");
      Serial.print(i);
      break;
    case 6:
      tft.setTextColor(TFT_PINK, TFT_WHITE);
      i = 7;
      Serial.print("更换成功6");
      Serial.print(i);
      Serial.print("\n");
      Serial.print(i);
      break;
    case 7:
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
      i = 1;
      Serial.print("更换成功7");
      Serial.print(i);
      Serial.print("\n");
      Serial.print(i);
      break;
  }
}*/

DHTesp dht;

void setup()
{
  // uncommet this to do calibration, code will be blocked here

  Serial.begin(115200);
  tft.init();
  tft.fillScreen(0x0000);

  /*mySCoop.start();*/

  WiFi.mode(WIFI_STA);     //设置ESP8266工作模式
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }//连接WIFI

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  // Autodetect is not working reliable, don't use the following line
  // use this instead:
  dht.setup(12, DHTesp::DHT22); // Connect DHT sensor to D6

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  TJpgDec.drawJpg(0, 0, watchtop, sizeof(watchtop));
  TJpgDec.drawJpg(0, 140, watchbottom, sizeof(watchbottom));

  //绘制一个视口
  tft.setViewport(0, 20, 128 , 140);
  //tft.fillScreen(0x0000);
  tft.fillRoundRect(0, 0, 128, 120, 5, 0xFFFF); //实心圆角矩形

  //绘制线框
  tft.drawFastHLine(0, 20, 128, TFT_BLACK);
  tft.drawFastVLine(75, 0, 20, TFT_BLACK);
  tft.drawFastHLine(0, 70, 128, TFT_BLACK);
  tft.drawFastVLine(64, 70, 50, TFT_BLACK);


  TJpgDec.drawJpg(1, 71, temperature, sizeof(temperature)); //温度图标
  TJpgDec.drawJpg(1, 96, humidity, sizeof(humidity)); //湿度图标

  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor(78, 6 );
  tft.setTextSize(1);
  tft.print(CITY);//写位置

  //mpu6050 setup
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;

  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }


  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }
}

  void loop()
  {
    imgAnim();
    
    timeClient.UpdateTime(timeData);
    weatherClient.UpdateWeather(CITY, weatherData);
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    imgAnim();

    
    
     if (a.acceleration.x > 4)
    {
      switch(i){
      case 1:
    Serial.print(i);
    tft.setTextColor(TFT_NAVY,TFT_WHITE);
    i=2;
    Serial.print("更换成功1");
    Serial.print("\n");
    break;
  case 2:
       tft.setTextColor(TFT_VIOLET,TFT_WHITE);
       i=3;
       Serial.print("更换成功2");
       Serial.print(i);
       Serial.print("\n");
      break;
  case 3:
       tft.setTextColor(TFT_CYAN,TFT_WHITE);
       i=4;
       Serial.print("更换成功3");
       Serial.print(i);
       Serial.print("\n");
      break;
  case 4:
       tft.setTextColor(TFT_GREENYELLOW,TFT_WHITE);
       i=5;
       Serial.print("更换成功4");
       Serial.print(i);
       Serial.print("\n");
      break;
  case 5:
       tft.setTextColor(TFT_GOLD,TFT_WHITE);
       i=6;
       Serial.print("更换成功5");
       Serial.print(i);
       Serial.print("\n");
      break;   
   case 6:
       tft.setTextColor(TFT_PINK,TFT_WHITE);
       i=7;
       Serial.print("更换成功6");
       Serial.print(i);
       Serial.print("\n");
      break; 
   case 7:
       tft.setTextColor(TFT_BLACK,TFT_WHITE);
       i=1;
       Serial.print("更换成功7");
       Serial.print(i);
       Serial.print("\n");
      break; 
      }
    }
    /*Serial.print("Now the time is ");
      Serial.print(timeData.hours);
      Serial.print(":");
      Serial.println(timeData.minutes);
      Serial.println(timeData.day);

      Serial.print("City: ");
      Serial.print(weatherData.city);
      Serial.print("   Weather: ");
      Serial.print(weatherData.weather);
      Serial.print("\n");
      Serial.print("   Temp: ");
      Serial.println(weatherData.temp+"°C");
      Serial.println(temperature);*/
      Serial.println(a.acceleration.x);
      tft.setCursor(78, 6 );
    tft.setTextSize(1);
    tft.print(CITY);//写位置
    if (timeData.minutes != lastTime.minutes || a.acceleration.x > 4)//每分钟更新时间
    { tft.setCursor(20, 29);
      tft.setTextSize(3);
      tft.print(timeData.hours);
      tft.print(":");
      if (timeData.minutes < 10)
      {
        tft.print("0");
        tft.print(timeData.minutes);
      }
      else {
        tft.print(timeData.minutes);
      }
      lastTime = timeData;//写时间
    }
    imgAnim();
    /* tft.setCursor(20,60);
      tft.setTextSize(1);
      ShowWeeks(timeData.day);//写星期*/
    if (timeData.minutes != lastWeather.minutes||a.acceleration.x > 4)//每分钟更新天气
    { tft.setCursor(5, 6 );
      tft.setTextSize(1);
      tft.print(weatherData.weather);//写天气
      imgAnim();
      tft.setCursor(sizeof(weatherData.weather) + 20, 6 );
      tft.setTextSize(1);
      tft.println(weatherData.temp);//写温度
      lastWeather = timeData;
    }
    imgAnim();
    // tft.pushImage(103+tft.textWidth(weatherData.temp, 1)+3, 106, 24, 30,tempicon[0]);
    tft.setCursor(25, 105);
    tft.setTextSize(1);
    tft.print(humidity);//室内湿度
    imgAnim();
    tft.setCursor(25, 75);
    tft.setTextSize(1);
    tft.print(temperature);//室温
    imgAnim();
 

    delay(1);
  }
