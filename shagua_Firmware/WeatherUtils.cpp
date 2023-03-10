#include "WeatherUtils.h"

WeatherClient::WeatherClient()
{
  client.setTimeout(HTTP_TIMEOUT);
}

void WeatherClient::UpdateWeather(const char* city, WeatherData& data)
{
  while (!client.connected())
  {
    if (!client.connect(host, 80))
    {
      Serial.println("connection....");
      delay(500);
    }
  }
  if (sendRequest(host, city, APIKEY) && skipResponseHeaders())
  {
    clrEsp8266ResponseBuffer();
    readReponseContent(response, sizeof(response));

    if (parseUserData(response, &data))
    {
      //printUserData(&data);
    }
  }
}

/**
* @��������ָ��
*/
bool WeatherClient::sendRequest(const char* host, const char* cityid, const char* apiKey)
{
  // We now create a URI for the request
  //��֪����
  String GetUrl = "/v3/weather/now.json?key=";
  GetUrl += apiKey;
  GetUrl += "&location=";
  GetUrl += cityid;
  GetUrl += "&language=";
  GetUrl += language;
  // This will send the request to the server
  client.print(String("GET ") + GetUrl + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");
  /*Serial.println("create a request:");
  Serial.println(String("GET ") + GetUrl + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n");*/
  delay(1000);
  return true;
}

/**
* @Desc ���� HTTP ͷ��ʹ��������Ӧ���ĵĿ�ͷ
*/
bool WeatherClient::skipResponseHeaders()
{
  // HTTP headers end with an empty line
  bool ok = client.find("\r\n\r\n");
  if (!ok)
  {
    Serial.println("No response or invalid response!");
  }
  return ok;
}

/**
* @Desc ��HTTP��������Ӧ�ж�ȡ����
*/
void WeatherClient::readReponseContent(char* content, size_t maxSize)
{
  size_t length = client.readBytes(content, maxSize);
  delay(100);
  //Serial.println("Get the data from Internet!");
  content[length] = 0;
  //Serial.println(content);
  //Serial.println("Read data Over!");
  client.flush();//��������Ҫ����  ��Ȼ�ᷢ��ÿ��һ��client.find��ʧ��
}

/**
 * @Desc ��������
 * ���ݸ�ʽ���£�
 * {
 *    "results": [
 *        {
 *            "location": {
 *                "id": "WX4FBXXFKE4F",
 *                "name": "����",
 *                "country": "CN",
 *                "path": "����,����,�й�",
 *                "timezone": "Asia/Shanghai",
 *                "timezone_offset": "+08:00"
 *            },
 *            "now": {
 *                "text": "����",
 *                "code": "4",
 *                "temperature": "23"
 *            },
 *            "last_update": "2017-09-13T09:51:00+08:00"
 *        }
 *    ]
 *}
 */
bool WeatherClient::parseUserData(char* content, struct WeatherData* userData)
{
  //    -- ����������Ҫ����������������JSON��������Ѵ�С
  //   �����ʹ��StaticJsonBufferʱ����Ҫ
  //    const size_t BUFFER_SIZE = 1024;
  //   �ڶ�ջ�Ϸ���һ����ʱ�ڴ��
  //    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  //    -- �����ջ���ڴ��̫��ʹ�� DynamicJsonBuffer jsonBuffer ����
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(content);

  if (!root.success())
  {
    Serial.println("JSON parsing failed!");
    return false;
  }

  //�������Ǹ���Ȥ���ַ���
  strcpy(userData->city, root["results"][0]["location"]["name"]);
  strcpy(userData->weather, root["results"][0]["now"]["text"]);
  userData->temp = atoi(root["results"][0]["now"]["temperature"]);
  strcpy(userData->udate, root["results"][0]["last_update"]);
  //  -- �ⲻ��ǿ�Ƹ��ƣ������ʹ��ָ�룬��Ϊ������ָ�����ݡ��������ڣ���������Ҫȷ��
  //   �����ȡ�ַ���ʱ�������ڴ���
  return true;
}

// ��ӡ��JSON����ȡ������
void WeatherClient::printUserData(const struct WeatherData* userData)
{
  Serial.println("Print parsed data :");
  Serial.print("City : ");
  Serial.print(userData->city);
  Serial.print(", \t");
  Serial.print("Weather : ");
  Serial.print(userData->weather);
  Serial.print(",\t");
  Serial.print("Temp : ");
  Serial.print(userData->temp);
  Serial.print(" C");
  Serial.print(",\t");
  Serial.print("Last Updata : ");
  Serial.print(userData->udate);
  Serial.println("\r\n");
}

// �ر���HTTP����������
void WeatherClient::stopConnect()
{
  Serial.println("Disconnect");
  client.stop();
}

void WeatherClient::clrEsp8266ResponseBuffer(void)
{
  memset(response, 0, MAX_CONTENT_SIZE);      //���
}
