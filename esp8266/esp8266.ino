#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPSRedirect.h>

#define PIN 4
#define NUMLED 1

#define SSID "ここにSSID"
#define PASSWORD "ここにパスワード"

#define HTTPS_PORT 443
#define HOST "script.google.com"
#define URL "/macros/s/（GASのAPIのURLをコピー）/exec"
#define FINGERPRINT "08:9E:B2:B8:77:37:3E:85:26:09:CA:29:13:D2:B0:57:26:DE:C4:6D"

#define INTERVAL_SEC 5

HTTPSRedirect* client = nullptr;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMLED, PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  // Serialの初期化
  Serial.begin(115200);
  Serial.println("");

  // マイコン内蔵RGB LEDの初期化
  pixels.begin();

  // Wi-Fi接続開始
  connectWifi();
}

void loop() {
  // Google Apps Scriptを叩く
  String result = doRedirectGet();

  // サフィックスを念のため確認
  if (!result.startsWith("pokio")) {
    delay(INTERVAL_SEC * 1000);
    return;
  }

  letMeBlink(result);
}

// Wi-Fi接続
void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    blinkWhite(3, 64, 64, 64);
  }

  Serial.println("Wi-Fi connected");
}

// Wi-Fi切断
void disconnectWifi() {
  WiFi.disconnect();
  Serial.println("Wi-Fi disconnected");
}

// 取得した文字列をパースして、指定された色で光らせる
void letMeBlink(String result) {
  result = result.substring(result.indexOf(",") + 1);
  int r =  result.substring(0, result.indexOf(",")).toInt();

  result = result.substring(result.indexOf(",") + 1);
  int g =  result.substring(0, result.indexOf(",")).toInt();

  result = result.substring(result.indexOf(",") + 1);
  int b =  result.substring(0, result.indexOf(",")).toInt();

  Serial.println(String("blinking : ") + r + "," + g + "," + b);
  blinkWhite(INTERVAL_SEC, r, g, b);
}

// 点滅
void blinkWhite(int sec, int r, int g, int b) {
  int count = 0;

  while (count < sec) {
    for (int i = 0; i < 256; i += 5) {
      setColor((r * i / 255), (g * i / 255), (b * i / 255));
    }

    for (int i = 255; i >= 0; i -= 5) {
      setColor((r * i / 255), (g * i / 255), (b * i / 255));
    }

    count++;
  }
}

// LEDを指定した色で光らせる
void setColor(int r, int g, int b) {
  for (int i = 0 ; i < NUMLED; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
    pixels.show();
  }
  delay(10);
}

// Redirectを回避してHTTP-GETする
String doRedirectGet() {
  String body = "";
  client = new HTTPSRedirect(HTTPS_PORT);

  if (!client->connect(HOST, HTTPS_PORT)) {
    Serial.println("connection failed");
    return body;
  }

  if (!client->verify(FINGERPRINT, HOST)) {
    Serial.println("certificate doesn't match");
  }

  client->GET(URL, HOST);
  body = client->getResponseBody();
  delete client;
  client = nullptr;
  return body;
}
