#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Keypad.h>
#include <Adafruit_NeoPixel.h> 
#define WIFI_SSID "Kat 1"
#define WIFI_PASSWORD "misafir10"
#define FIREBASE_API_KEY "AIzaSyADLy-YjOvSBQ1Afkfsx9xbO4NHT84TWwg"
#define FIREBASE_URL "https://vip-akilli-park-projesi-default-rtdb.europe-west1.firebasedatabase.app"
#define GIRIS_SERVO_PIN 5
#define CIKIS_SERVO_PIN 18
#define GIRIS_IR_PIN 34
#define CIKIS_IR_PIN 35
#define PARK1_IR_PIN 25
#define PARK2_IR_PIN 26
#define PARK3_IR_PIN 27
#define LED_PIN 13 
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo girisServo;
Servo cikisServo;
Adafruit_NeoPixel pixels(3, LED_PIN, NEO_GRB + NEO_KHZ800); 

const byte ROWS = 4; 
const byte COLS = 3; 
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {15, 2, 4, 16}; 
byte colPins[COLS] = {17, 19, 23}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String p1Durum = "Bos", p1Pin = "0000";
String p2Durum = "Bos", p2Pin = "0000";
String p3Durum = "Bos", p3Pin = "0000";

bool girisAracVar = false;
bool cikisAracVar = false;
int bosYerSayisi = 3;

unsigned long sonVeriSenkronizasyonu = 0;
const unsigned long senkronizasyonPeriyodu = 2000; 
int ekranYenilemeSayaci = 0; 

String blueprintHedef = ""; 
String hedefParkYeri = ""; 

void setup() {
  Serial.begin(115200); 
  Serial.println("\n--- Sistem Baslatiliyor ---");

  pinMode(GIRIS_IR_PIN, INPUT);
  pinMode(CIKIS_IR_PIN, INPUT);
  pinMode(PARK1_IR_PIN, INPUT);
  pinMode(PARK2_IR_PIN, INPUT);
  pinMode(PARK3_IR_PIN, INPUT);

  pixels.begin();
  pixels.setBrightness(10); 
  ledleriRenklendir();       

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("WiFi Baglaniyor.");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Baglandi!");
  lcd.clear();
  lcd.print("WiFi Baglandi!");
  delay(1000);

  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_URL;
  
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("[BULUT] Firebase Anonim Kayit Basarili.");
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  girisServo.attach(GIRIS_SERVO_PIN, 500, 2400);
  cikisServo.attach(CIKIS_SERVO_PIN, 500, 2400);
  girisServo.write(90);
  cikisServo.write(90);
  delay(600);
  girisServo.detach();
  cikisServo.detach();

  lcd.clear();
  lcd.print("Firebase Hazir!");
  delay(1000);
}

void loop() {

  if (millis() - sonVeriSenkronizasyonu >= senkronizasyonPeriyodu) {
    sonVeriSenkronizasyonu = millis();
    
    if (!Firebase.ready()) return; 

    if (Firebase.RTDB.getString(&fbdo, "otopark/P1/durum")) p1Durum = fbdo.stringData();
    if (Firebase.RTDB.getString(&fbdo, "otopark/P1/pin")) p1Pin = fbdo.stringData();
    if (Firebase.RTDB.getString(&fbdo, "otopark/P2/durum")) p2Durum = fbdo.stringData();
    if (Firebase.RTDB.getString(&fbdo, "otopark/P2/pin")) p2Pin = fbdo.stringData();
    if (Firebase.RTDB.getString(&fbdo, "otopark/P3/durum")) p3Durum = fbdo.stringData();
    if (Firebase.RTDB.getString(&fbdo, "otopark/P3/pin")) p3Pin = fbdo.stringData();

    if (kacakKontrolEt()) {
      return; 
    }

    sensorKontrolEt("P1", PARK1_IR_PIN, p1Durum);
    sensorKontrolEt("P2", PARK2_IR_PIN, p2Durum);
    sensorKontrolEt("P3", PARK3_IR_PIN, p3Durum);

    int anlikBos = 0;
    if (p1Durum == "Bos") anlikBos++;
    if (p2Durum == "Bos") anlikBos++;
    if (p3Durum == "Bos") anlikBos++;
    bosYerSayisi = anlikBos;

    ekraniGuncelle();
    ledleriRenklendir(); 
  }

  int girisDurum = digitalRead(GIRIS_IR_PIN);
  if (girisDurum == LOW && !girisAracVar) {
    delay(150); 
    if (digitalRead(GIRIS_IR_PIN) == LOW) {
      girisAracVar = true;
      if (bosYerSayisi > 0) {
        lcdMesajYaz("Arac Girdi!", "Geciniz...");
        hedefParkYeri = ""; 
        bariyerAc(girisServo, GIRIS_SERVO_PIN);
      } 
      else if (p1Durum == "Rez" || p2Durum == "Rez" || p3Durum == "Rez") {
        String girilenSifre = sifreAlEkrandan();
        
        if (p1Durum == "Rez" && girilenSifre == p1Pin) {
            lcdMesajYaz("PIN Dogru", "Hedefiniz: P1");
            hedefParkYeri = "P1"; 
            bariyerAc(girisServo, GIRIS_SERVO_PIN);
        } 
        else if (p2Durum == "Rez" && girilenSifre == p2Pin) {
            lcdMesajYaz("PIN Dogru", "Hedefiniz: P2");
            hedefParkYeri = "P2"; 
            bariyerAc(girisServo, GIRIS_SERVO_PIN);
        }
        else if (p3Durum == "Rez" && girilenSifre == p3Pin) {
            lcdMesajYaz("PIN Dogru", "Hedefiniz: P3");
            hedefParkYeri = "P3"; 
            bariyerAc(girisServo, GIRIS_SERVO_PIN);
        }
        else {
            lcdMesajYaz("Gecersiz PIN!", "Giris Engellendi");
            delay(2000);
        }
      } 
      else {
        lcdMesajYaz("Otopark DOLU!", "Yer Yoktur.");
        delay(2000);
      }
      ekraniGuncelle();
    }
  } 
  else if (girisDurum == HIGH && girisAracVar) {
    girisAracVar = false;
    delay(50);
  }

  int cikisDurum = digitalRead(CIKIS_IR_PIN);
  if (cikisDurum == LOW && !cikisAracVar) {
    cikisAracVar = true;
    lcdMesajYaz("Gule Gule!", "Yine Bekleriz.");
    bariyerAc(cikisServo, CIKIS_SERVO_PIN);
    ekraniGuncelle();
  } 
  else if (cikisDurum == HIGH && cikisAracVar) {
    cikisAracVar = false;
    delay(50);
  }
}

void sensorKontrolEt(String parkId, int pin, String mevcutDurum) {
  if (!Firebase.ready()) return;
  int fizikselDurum = digitalRead(pin);
  
  if (fizikselDurum == LOW && mevcutDurum != "Dolu") {
    if (Firebase.RTDB.setString(&fbdo, "otopark/" + parkId + "/durum", "Dolu")) {
      Firebase.RTDB.setString(&fbdo, "otopark/" + parkId + "/pin", "0000");
    }
  } 
  else if (fizikselDurum == HIGH && mevcutDurum == "Dolu") {
    if (Firebase.RTDB.setString(&fbdo, "otopark/" + parkId + "/durum", "Bos")) {
    }
  }
}

bool kacakKontrolEt() {
  if (hedefParkYeri == "") return false; 

  int p1Fiziksel = digitalRead(PARK1_IR_PIN);
  int p2Fiziksel = digitalRead(PARK2_IR_PIN);
  int p3Fiziksel = digitalRead(PARK3_IR_PIN);

  if (hedefParkYeri == "P1" && p1Fiziksel == LOW) { hedefParkYeri = ""; return false; }
  if (hedefParkYeri == "P2" && p2Fiziksel == LOW) { hedefParkYeri = ""; return false; }
  if (hedefParkYeri == "P3" && p3Fiziksel == LOW) { hedefParkYeri = ""; return false; }

  String hataliParkYeri = "";
  int hataliLedIndex = -1;

  if (hedefParkYeri == "P1") {
    if (p2Fiziksel == LOW && p2Durum != "Dolu") { hataliParkYeri = "P2"; hataliLedIndex = 2; }
    else if (p3Fiziksel == LOW && p3Durum != "Dolu") { hataliParkYeri = "P3"; hataliLedIndex = 1; }
  }
  else if (hedefParkYeri == "P2") {
    if (p1Fiziksel == LOW && p1Durum != "Dolu") { hataliParkYeri = "P1"; hataliLedIndex = 0; }
    else if (p3Fiziksel == LOW && p3Durum != "Dolu") { hataliParkYeri = "P3"; hataliLedIndex = 1; }
  }
  else if (hedefParkYeri == "P3") {
    if (p1Fiziksel == LOW && p1Durum != "Dolu") { hataliParkYeri = "P1"; hataliLedIndex = 0; }
    else if (p2Fiziksel == LOW && p2Durum != "Dolu") { hataliParkYeri = "P2"; hataliLedIndex = 2; }
  }

  if (hataliParkYeri != "") {
    lcdMesajYaz("KURAL IHLALI!", "YANLIS PARK YERI");
    Serial.println("[PRO AKTİF] " + hataliParkYeri + " alanında ihlal yakalandı! Veriler kurtarılıyor...");

    String calinanDurum = "";
    String calinanPin = "0000";
    if (hataliParkYeri == "P1") { calinanDurum = p1Durum; calinanPin = p1Pin; }
    else if (hataliParkYeri == "P2") { calinanDurum = p2Durum; calinanPin = p2Pin; }
    else if (hataliParkYeri == "P3") { calinanDurum = p3Durum; calinanPin = p3Pin; }

    FirebaseJson updateData;
    updateData.set("otopark/" + hataliParkYeri + "/durum", "Dolu");
    updateData.set("otopark/" + hataliParkYeri + "/pin", "0000");

    if (calinanDurum == "Rez") {
      updateData.set("otopark/" + hedefParkYeri + "/durum", "Rez");
      updateData.set("otopark/" + hedefParkYeri + "/pin", calinanPin); 
    } 
    else if (calinanDurum == "Bos") {
      updateData.set("otopark/" + hedefParkYeri + "/durum", "Bos");
      updateData.set("otopark/" + hedefParkYeri + "/pin", "0000");
    }

    Firebase.RTDB.updateNode(&fbdo, "/", &updateData);

    for (int i = 0; i < 15; i++) {
      pixels.setPixelColor(hataliLedIndex, pixels.Color(255, 0, 0));
      pixels.show();
      delay(150);
      pixels.setPixelColor(hataliLedIndex, pixels.Color(0, 0, 0));
      pixels.show();
      delay(150);
    }

    hedefParkYeri = ""; 
    lcd.init();
    lcd.backlight();
    return true; 
  }
  return false;
}

/
void ledleriRenklendir() {
  renkSec(0, p1Durum);
  renkSec(2, p2Durum); 
  renkSec(1, p3Durum); 
  pixels.show();
}

void renkSec(int ledIndex, String durum) {
  if (durum == "Bos") {
    pixels.setPixelColor(ledIndex, pixels.Color(0, 255, 0));   
  } else if (durum == "Dolu") {
    pixels.setPixelColor(ledIndex, pixels.Color(255, 0, 0));  
  } else if (durum == "Rez") {
    pixels.setPixelColor(ledIndex, pixels.Color(0, 0, 255));  
  } else {
    pixels.setPixelColor(ledIndex, pixels.Color(0, 0, 0));     
  }
}

String sifreAlEkrandan() {
  String sifre = "";
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  lcd.print("PIN Giriniz:");
  lcd.setCursor(0, 1);
  unsigned long baslangicZamani = millis();
  while (sifre.length() < 4 && (millis() - baslangicZamani < 15000)) {
    char tus = keypad.getKey();
    if (tus != NO_KEY && tus >= '0' && tus <= '9') {
      sifre += tus;
      lcd.print("*"); 
    }
  }
  return sifre;
}

void bariyerAc(Servo &motor, int pin) {
  motor.attach(pin, 500, 2400);
  delay(30);
  motor.write(0);  
  delay(3000);     
  motor.write(90); 
  delay(600);
  motor.detach();  
  lcd.init();
  lcd.backlight();
}

void ekraniGuncelle() {
  ekranYenilemeSayaci++;
  if (ekranYenilemeSayaci >= 5) {
    lcd.init();
    lcd.backlight();
    ekranYenilemeSayaci = 0;
  } else {
    lcd.clear();
  }
  
  lcd.setCursor(0, 0);
  lcd.print("P1:" + shortDurum(p1Durum) + " P2:" + shortDurum(p2Durum));
  lcd.setCursor(0, 1);
  lcd.print("P3:" + shortDurum(p3Durum));
}

String shortDurum(String durum) {
  if (durum == "Bos") return "Bos";
  if (durum == "Dolu") return "Dolu"; 
  if (durum == "Rez") return "Rez";
  return "---";
}

void lcdMesajYaz(String m1, String m2) {
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(m1);
  lcd.setCursor(0, 1);
  lcd.print(m2);
}