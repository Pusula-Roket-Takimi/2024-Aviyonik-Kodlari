/*
 *  DENEYAPKART PUSULA ROKET VERICI KODU
*/
#include "WiFi.h"
#include "esp_now.h"

// Sensörler
#include <SFE_BMP180.h>  // Basınç Sensör kütüphanesi
#include <deneyap.h>
#include "lsm6dsm.h"  // Deneyap Kart dahili IMU kütüphanesinin eklenmesi


SFE_BMP180 pressure;  //sensör adı tanımlama
LSM6DSM IMU;          // IMU için class tanımlanması

double sabitbasinc;  // Basınç sabiti
int irtifa = 0, aci = 0;
int onceki_irtifa = 0;

//////// sensör son


//// iletişim
uint8_t broadcastAddress[] = { 0x7c, 0x9e, 0xbd, 0xd9, 0x19, 0xb8 };  // ALICI kartın MAC adresi

typedef struct struct_message {
  float lat, log, alt;
  int irtifa;
  float gyroX, gyroY, gyroZ;
} struct_message;

struct_message VeriPaketi;

esp_now_peer_info_t peerInfo;

// debug
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Veri gönderme başarılı" : "Veri gönderme başarısız");
}


// iletişim son

void setup() {
  Serial.begin(115200);

  if (pressure.begin()) {
    sabitbasinc = getPressure();
    Serial.println("[Sensor] Basinc hazir!");
  } else
    Serial.println("HATA: Basinc sensoru.\n\n");


  IMU.begin();
  Serial.print("Baslangic Basinci: ");
  Serial.print(sabitbasinc);



  //////////////////////////////////////////////7
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Eşleşme başarısız");
    return;
  }
}

void loop() {


  // KURTARMA ANA ALGORITMASI

  Serial.print("Z ekseni: ");
  float zeksen = abs(IMU.readFloatAccelZ());
  Serial.println(zeksen);
  aci = (zeksen > .75);  // roket yatmış!!???
/*

  double a, P;

  P = getPressure();
  a = pressure.altitude(P, sabitbasinc);

  Serial.print("\n\nYükseklik: ");
  Serial.print(a, 1);
  irtifa = (a < onceki_irtifa);
  onceki_irtifa = a;
  Serial.println(irtifa && aci);
  bool patlama=(irtifa && aci) ;*/


/*
  if (irtifa && aci) {
    patlama=true;
 //  digitalWrite(buzzer, HIGH);
  }
*/



//VeriPaketi.irtifa=a;

VeriPaketi.gyroZ=IMU.readFloatAccelZ();
VeriPaketi.gyroY=IMU.readFloatAccelY();
VeriPaketi.gyroX=IMU.readFloatAccelX();
Serial.println(VeriPaketi.gyroX);

//  float lat, log;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&VeriPaketi, sizeof(VeriPaketi));

  delay(750);  /// sil
}




double getPressure() {
  char status;
  double T, P, p0, a;

  status = pressure.startTemperature();  //Bir sıcaklık ölçümü başlatılır
  if (status != 0)                       //
  {
    delay(status);                        // Ölçüm tamamlanması için bekle
    status = pressure.getTemperature(T);  // T değerini ölç
    Serial.println(status);
    if (status != 0)  //Sıfırdan faklı ise
    {
      status = pressure.startPressure(3);  //Basınç ölçümünü başlat
      if (status != 0) {

        delay(status);  // Ölçümünü tamamlanmasını bekle

        status = pressure.getPressure(P, T);  //Tamamlanan basınç ölçümü Al :
        if (status != 0)                      //sıfıra eşit değilse
        {
          return (P);  // Ölçüm birimi P saklanır
        }
      }
    }
  }
}