#include <SFE_BMP180.h>  // Basınç Sensör kütüphanesi
#include <deneyap.h>
#include "lsm6dsm.h"  // Deneyap Kart dahili IMU kütüphanesinin eklenmesi

#define buzzer D0


SFE_BMP180 pressure;  //sensör adı tanımlama
LSM6DSM IMU;          // IMU için class tanımlanması


double sabitbasinc;  // Basınç sabiti
int irtifa = 0, aci = 0;
int onceki_irtifa = 0;

void setup() {
  pinMode(buzzer, OUTPUT);

  Serial.begin(115200);
  if (pressure.begin())
    Serial.println("[Sensor] Basinc hazir!");
  else {
    Serial.println("HATA: Basinc sensoru.\n\n");
  }

  IMU.begin();
  sabitbasinc = getPressure();

  Serial.print("Baslangic Basinci: ");
  Serial.print(sabitbasinc);
}


void loop() {

  // KURTARMA ANA ALGORITMASI
  double a, P;

  Serial.print("Z ekseni: ");
  float zeksen = abs(getZAxis());
  Serial.println(zeksen);
  aci = (zeksen > .75);  // roket yatmış!!???

  P = getPressure();
  a = pressure.altitude(P, sabitbasinc);

  Serial.print("\n\nYükseklik: ");
  Serial.print(a, 1);
  irtifa = (a < onceki_irtifa);
  onceki_irtifa = a;
  Serial.println(irtifa && aci);
  if(irtifa && aci){
  digitalWrite(buzzer,HIGH);
delay(500);
digitalWrite(buzzer,LOW);
  }
  delay(500);

}

double getZAxis() {

  return IMU.readFloatAccelZ();
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