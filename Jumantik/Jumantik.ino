#include <OneWire.h>
#include <DallasTemperature.h>
//variabel ph air (kalibrasi analog menjadi ph)
float Po = 0;
float PH_step;
int nilai_analog_PH;
double TeganganPh;

//variabel nilai batas
//ph
float asamLo = 1;
float netralLo = 6.5;
float basahLo =7.1;
float asamHi = 6.4;
float netralHi = 7;
float basahHi = 7.5;
//suhu
float dinginLo = 10;
float normalLo = 26;
float panasLo = 31;
float dinginHi = 25;
float normalHi = 30;
float panasHi = 50;
float celsiusLo;
// untuk kalibrasi voltage
float PH4 = 4.189;
float PH7 = 3.739;
float PH9 = 3.199; // Mengubah PH10 menjadi PH9

float potensi1;
//suhu air
// Inisialisasi pin data untuk sensor DS18B20
const int oneWireBusPin = D3;  // Sesuaikan dengan pin yang digunakan pada Arduino Anda
// Inisialisasi objek OneWire dan DallasTemperature
OneWire oneWire(oneWireBusPin);
DallasTemperature sensors(&oneWire);
float celsius;

//firebase
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
// tambahkan wifi dan password(manual)
// #define WIFI_SSID ""
// #define WIFI_PASSWORD ""
#define API_KEY "AIzaSyBy-2erUEW2AwGkRHkgOfVM1v6qeywplc4"
#define DATABASE_URL "https://jumantik-8f0e6-default-rtdb.firebaseio.com/" 

#include <WiFiManager.h>
char ssid[64];  // Menyimpan SSID yang diperoleh dari WiFi Manager
char pass[64];  // Menyimpan password yang diperoleh dari WiFi Manager

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
//unsigned long sendDataPrevMillis = 0;
//int count = 0;
bool signupOK = false;
bool ledwifi;
void setup(){
  WiFi.mode(WIFI_STA);
  sensors.begin();
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  // Inisialisasi WiFi Manager
  bool res;
  WiFiManager wifiManager;
  res = wifiManager.autoConnect("WEMOSPH", "password");
  // Menyimpan SSID dan password yang diperoleh dari WiFi Manager
  strncpy(ssid, WiFi.SSID().c_str(), sizeof(ssid));
  strncpy(pass, WiFi.psk().c_str(), sizeof(pass));
  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    ledwifi = true;
    Serial.println("Connected...yeey :)");
    
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  output();
  PHAir();
  kirimDataFirebase();
  if(ledwifi == true){
    digitalWrite(LED_BUILTIN, HIGH);
  }else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  delay(3000);
}
void output() {
  // 0 = tidak
  // 1 = ya
  if ((celsius >=dinginLo) && (celsius <=dinginHi) && (Po <=asamHi)) {
    potensi1 = 0;
  } else if ((celsius >=dinginLo) && (celsius <=dinginHi) && (Po <=netralHi)) {
    potensi1 = 0;
  } else if ((celsius >=dinginLo) && (celsius <=dinginHi) && (Po <=basahHi)) {
    potensi1 = 0;
  } else if ((celsius >=normalLo) && (celsius <=normalHi) && (Po <=asamHi)) {
    potensi1 = 0;
  } else if ((celsius >=normalLo) && (celsius <=normalHi) && (Po <=netralHi)) {
    potensi1 = 1;
  } else if ((celsius >=normalLo) && (celsius <=normalHi) && (Po <=basahHi)) {
    potensi1 = 1;
  } else if ((celsius >=panasLo) && (celsius <=panasHi) && (Po <=asamHi)) {
    potensi1 = 0;
  } else if ((celsius >=panasLo) && (celsius <=panasHi) && (Po <=netralHi)) {
    potensi1 = 0;
  } else if ((celsius >=panasLo) && (celsius <=panasHi) && (Po <=basahHi)) {
    potensi1 = 0;
  }
}
void kirimDataFirebase(){
  sensors.requestTemperatures();  // Minta sensor untuk membaca suhu
  String potensi;
  // Baca suhu dalam Celsius dan Fahrenheit
  celsius = sensors.getTempCByIndex(0);
  float fahrenheit = sensors.toFahrenheit(celsius);

  if (Firebase.ready() && signupOK ) {
    // Write an Float number on the database path test/float  
    if (Firebase.RTDB.setFloat(&fbdo, "Data/temperature", celsius)){
    // Serial.println("PASSED");
       Serial.print("Temperature: ");
       Serial.println(celsius);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Data/PH", Po)){
    //  Serial.println("PASSED");
       Serial.print("Ph masuk: ");
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Data/PotensiJen", potensi1)){
    //  Serial.println("PASSED");
       Serial.println("potensi:");
       Serial.print(potensi1);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    
  }
  Serial.println("______________________________");
}
void PHAir()
{
  nilai_analog_PH = analogRead(A0);
  Serial.print("Nilai ADC PH: ");
  Serial.println(nilai_analog_PH);

  TeganganPh = 4.189 / 1024.0 * nilai_analog_PH;
  Serial.print("Tegangan PH: ");
  Serial.println(TeganganPh, 3);

  // Menghitung langkah perubahan pH
  PH_step = ((PH4 - PH7) + (PH7 - PH9)) / 6; // Mengubah titik kalibrasi PH10 menjadi PH9

  if (TeganganPh > PH7) {
    Po = 6.86 + ((PH7 - TeganganPh) / PH_step);
  } else if (TeganganPh > PH9) {
    Po = 9.18 + ((PH9 - TeganganPh) / PH_step); // Mengubah perhitungan untuk PH9
  } else {
    Po = 4.01 + ((PH4 - TeganganPh) / PH_step);
  }

  Serial.println("Nilai PH cairan = ");
  Serial.println(Po, 2);
  Serial.println(); // Garis kosong sebagai pemisah antara setiap output
}