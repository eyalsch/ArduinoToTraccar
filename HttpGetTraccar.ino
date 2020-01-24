// Send GPS location to Traccar server

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h> //I2C LCD Library

static const int GPSBaud = 9600;
static const int CellularBaud = 9600;
static const int SerialBaud = 9600;

static const int GPS_RXPin = 4, GPS_TXPin = 3; // GPS pins
static const int CELLULAR_RXPin = 6, CELLULAR_TXPin = 5; // GPS pins

static const int RecButton = 7; // Record Button Pin

static const int Duration = 5000; // Milliseconds between samples

static const String TraccarServer = "swimmer.duckdns.org";
static const String APN = "uinternet";
static const String id = "eyal";


uint32_t lasttime = 0;

LiquidCrystal_I2C lcd(0x27, 20, 4);
TinyGPSPlus gps; // The TinyGPS++ object
SoftwareSerial GPSSerial(GPS_RXPin, GPS_TXPin); // The serial connection to the GPS device
SoftwareSerial CellularSerial(CELLULAR_RXPin, CELLULAR_TXPin); // The serial connection to the Cellular module

void setup()
{
  Serial.begin(SerialBaud);
  while (!Serial);
  GPSSerial.begin(GPSBaud);
  CellularSerial.begin(CellularBaud);
  CellularSerial.println("AT+CIPSHUT");
  CellularSerial.println("AT+CIPMUX=0");
  CellularSerial.println("AT+CGATT=1");
  CellularSerial.println("AT+CSTT=\"uinternet\",\"\",\"\"");
  CellularSerial.println("AT+CIICR");
  CellularSerial.println("AT+CIFSR");

  GPSSerial.listen();
  lcd.begin();
  pinMode(RecButton, INPUT_PULLUP);

  lasttime = millis();
}

void loop()
{
  while (GPSSerial.available() > 0)
    if (gps.encode(GPSSerial.read())) displayOnLCD();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
  // Button pushed and location valid and Time between samples bigger the Duration THEN send point to TRACCAR Server
  if ((digitalRead(RecButton) == 0) && gps.location.isValid() && (millis() - lasttime > Duration)) {
    // send point
    SendTraccar(gps.location.lat(), gps.location.lng(), gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second(), gps.hdop.value(), gps.course.deg(), gps.altitude.meters(), gps.speed.kmph());

    lasttime = millis();

    Serial.println(F("Sent point to Server"));
    Serial.print(gps.location.lat(), 8);
    Serial.print(F(":"));
    Serial.println(gps.location.lng(), 8);
    Serial.print(gps.date.value());
    Serial.print("-");
    Serial.println(gps.time.value());
  }
}

void SendTraccar(double lat, double lng, int year, int month, int day, int hour, int minute, int second, int hdop, double course, double altitude, double kmph)
{
  CellularSerial.print("AT+CIPSTART=\"TCP\",\"");
  CellularSerial.print(TraccarServer);
  CellularSerial.println("\", 5055");
  CellularSerial.println("AT+CIPSEND");
  CellularSerial.print("GET /?id=");
  CellularSerial.print(id);
  CellularSerial.print("&lat=");
  CellularSerial.print(lat);
  CellularSerial.print("&lon=");
  CellularSerial.print(lng);
  CellularSerial.print("&timestamp=");
  CellularSerial.print(year);
  CellularSerial.print("-");
  CellularSerial.print(month);
  CellularSerial.print("-");
  CellularSerial.print(day);
  CellularSerial.print("%20");
  CellularSerial.print(hour);
  CellularSerial.print(":");
  CellularSerial.print(minute);
  CellularSerial.print(":");
  CellularSerial.print(second);
  CellularSerial.print("&hdop=");
  CellularSerial.print(hdop);
  CellularSerial.print("&course=");
  CellularSerial.print(course);
  CellularSerial.print("&altitude=");
  CellularSerial.print(altitude);
  CellularSerial.print("&speed=");
  CellularSerial.print(kmph);
  CellularSerial.println(" HTTP/1.1");
  CellularSerial.print("Host: ");
  CellularSerial.print(TraccarServer);
  CellularSerial.println("\r\n");
  CellularSerial.write(0x1A);
  delay(1000);
}

void displayOnLCD()
{
  if (gps.location.isValid())
  {
    lcd.setCursor(0, 0);
    lcd.print(gps.location.lat(), 6);
    lcd.print(F(":"));
    lcd.print(gps.location.lng(), 6);

    lcd.setCursor(0, 1);
    lcd.print(gps.date.year() - 2000);
    lcd.print("/");
    lcd.print(gps.date.month() < 10 ? "0" : "");
    lcd.print(gps.date.month());
    lcd.print("/");
    lcd.print(gps.date.day() < 10 ? "0" : "");
    lcd.print(gps.date.day());
    lcd.print("  ");
    lcd.print(gps.time.hour() < 10 ? "0" : "");
    lcd.print(gps.time.hour());
    lcd.print(":");
    lcd.print(gps.time.minute() < 10 ? "0" : "");
    lcd.print(gps.time.minute());
    lcd.print(":");
    lcd.print(gps.time.second() < 10 ? "0" : "");
    lcd.print(gps.time.second());

    lcd.setCursor(0, 2);
    lcd.print("Hdop / kmph / Sat.");
    lcd.setCursor(0, 3);
    lcd.print(gps.hdop.hdop(), 2);
    lcd.print(F(" / "));
    lcd.print(gps.speed.kmph(), 2);
    lcd.print(F(" / "));
    lcd.print(gps.satellites.value());
  }
  else {
    lcd.setCursor(0, 3);
    lcd.print(F("INVALID"));
  }

}
