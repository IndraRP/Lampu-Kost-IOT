#define BLYNK_TEMPLATE_ID "{your blynk id}"
#define BLYNK_TEMPLATE_NAME "Saklar Lampu Kost"
#define BLYNK_AUTH_TOKEN "{your auth}"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

char ssid[] = "Indrawr";
char pass[] = "{password wifi}";

#define SERVO_PIN D5
#define WIB_OFFSET 0  // Blynk SGP1 sudah UTC+8, WIB=UTC+7, jadi kurangi 1

int jamON = -1, menitON = -1;
int jamOFF = -1, menitOFF = -1;
bool sudahON = false;
bool sudahOFF = false;
bool fromBlynk = false;  // flag cegah infinite loop

BlynkTimer timer;
WidgetRTC rtc;

void setServoAngle(int angle) {
  int pulseWidth = map(angle, 0, 180, 500, 2400);
  for (int i = 0; i < 50; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulseWidth);
  }
}

void lampuON() {
  setServoAngle(90);
  fromBlynk = true;
  Blynk.virtualWrite(V0, 1);  // update tampilan switch
  fromBlynk = false;
  Blynk.logEvent("lampu_on", "Lampu dinyalakan!");
  Serial.println("Lampu ON");
}

void lampuOFF() {
  setServoAngle(0);
  fromBlynk = true;
  Blynk.virtualWrite(V0, 0);  // update tampilan switch
  fromBlynk = false;
  Blynk.logEvent("lampu_off", "Lampu dimatikan!");
  Serial.println("Lampu OFF");
}

BLYNK_WRITE(V0) {
  if (fromBlynk) return;  // cegah infinite loop
  int state = param.asInt();
  if (state == 1) setServoAngle(90);
  else setServoAngle(0);
}

BLYNK_WRITE(V1) { jamON    = param.asInt(); Serial.printf("Set Jam ON: %d\n", jamON); }
BLYNK_WRITE(V2) { menitON  = param.asInt(); Serial.printf("Set Menit ON: %d\n", menitON); }
BLYNK_WRITE(V3) { jamOFF   = param.asInt(); Serial.printf("Set Jam OFF: %d\n", jamOFF); }
BLYNK_WRITE(V4) { menitOFF = param.asInt(); Serial.printf("Set Menit OFF: %d\n", menitOFF); }

void cekJadwal() {
  // Blynk SGP1 = UTC+8, WIB = UTC+7, jadi kurangi 1
  int jamSekarang = hour();
  int menitSekarang = minute();

  Serial.printf("Waktu WIB: %02d:%02d | ON=%02d:%02d | OFF=%02d:%02d\n",
    jamSekarang, menitSekarang, jamON, menitON, jamOFF, menitOFF);

  if (jamON >= 0 && jamSekarang == jamON && menitSekarang == menitON) {
    if (!sudahON) { lampuON(); sudahON = true; }
  } else { sudahON = false; }

  if (jamOFF >= 0 && jamSekarang == jamOFF && menitSekarang == menitOFF) {
    if (!sudahOFF) { lampuOFF(); sudahOFF = true; }
  } else { sudahOFF = false; }
}

void setup() {
  Serial.begin(115200);
  pinMode(SERVO_PIN, OUTPUT);
  setServoAngle(0);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  rtc.begin();
  timer.setInterval(60000L, cekJadwal);
}

void loop() {
  Blynk.run();
  timer.run();
}
