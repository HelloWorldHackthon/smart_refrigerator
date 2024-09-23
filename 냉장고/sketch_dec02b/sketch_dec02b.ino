//#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

// 와이파이 주소 / 비밀번호
const char* ssid     = "HACKATHON 5";
const char* password = "HACK-567";

// 데이터 전송할 서버 주소
const char* serverName = "http://leejiseok.iptime.org/input.php";


float getVPP();
const int sensorIn = 34;
int mVperAmp = 66; // use 185 for 5A Module and 100 for 20A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

int pin1=27;
int pin2=26;
int pin3=14;
int repin=4;

void setup() {
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  Serial.begin(115200);
  
  // 와이파이를 연결한다
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  // 와이파이 접속 연결 시도 
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  // 연결 정보를 표시 
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Voltage = getVPP();

  VRMS = (Voltage/2.0) *0.707;//전압측정
  AmpsRMS = (VRMS * 1000)/mVperAmp;//전류측정((AmpsRMS*220.0)/100.0)

  int a= digitalRead(repin);  // 문이 열였는지 닫혔는지를 확인하다 
  Serial.println(String(a));
  Serial.println(String(AmpsRMS));

  if(a==0){
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,HIGH);
  }
  else{
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,LOW);
  }

  //Check WiFi connection status
  if(AmpsRMS<4){
    AmpsRMS=0;
  }
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
    
    // 서버와 연결을 한다 
    http.begin(client, serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // post로 전송할 데이터를 key=data형식으로 입력
    String httpRequestData = "open=" + String(a) + "&power=" + String(AmpsRMS)+"";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    // http데이터를 전송한다
    int httpResponseCode = http.POST(httpRequestData);
     
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  delay(3000);  
}

/*
 * 전력을 측정하고 노이즈를 제거한다 
*/
float getVPP()
{
  float result;

  int readValue;
  //데이터를 읽어올 범위를 지정한다
  int maxValue = 0;
  int minValue = 4096;
  
   uint32_t start_time = millis();

   while((millis()-start_time) < 1000) // 1초동안 값을 모아서 AC 전류의 최고점, 최저점을 찾아 평균치를 구함
   {
       readValue = analogRead(sensorIn);// 데이터를 읽어온다 
       if (readValue > maxValue) 
       {
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           minValue = readValue;
       }
   }
   
   // 사용 전력을 측정함
   result = ((maxValue - minValue) * 5.0)/1024.0;        // 5V 분해능, 아날로그핀의 분해능 1024
      
   return result;
 }
