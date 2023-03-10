#pragma once

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
 

const unsigned long HTTP_TIMEOUT = 5000;               // max respone time from server
const size_t MAX_CONTENT_SIZE = 10000;                   // max size of the HTTP response

 
struct WeatherData
{
  char city[16];//��������
  char weather[32];//�������ܣ�����...��
  int  temp;//�¶�
  char udate[32];//����ʱ��
};

class WeatherClient
{
public:
  WeatherClient();

  void UpdateWeather(const char* city, WeatherData& data);

private:
  const char* host = "api.seniverse.com";
  const char* APIKEY = "8trpor4bfqsfnqhm";        //API KEY
  const char* language = "en";//zh-Hans ��������  ����ʾ����
  
  char response[MAX_CONTENT_SIZE];
  WiFiClient client;

  bool sendRequest(const char* host, const char* cityid, const char* apiKey);

  bool skipResponseHeaders();

  void readReponseContent(char* content, size_t maxSize);

  bool parseUserData(char* content, struct WeatherData* userData);

  void printUserData(const struct WeatherData* userData);

  void clrEsp8266ResponseBuffer(void);

  void stopConnect();
};
