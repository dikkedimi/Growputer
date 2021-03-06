#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

#define DHTPIN 3
#define DHTTYPE DHT22

#define ONE_WIRE_BUS 5
#define TEMPERATURE_PRECISION 9

#define NTP_PACKET_SIZE 48
#define TIMEZONE 2
#define LOCALPORT 1337

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float airHumids[10];
float airTemps[10];
float waterTemps[10];

unsigned int index = 0;
bool inited = false;
float deltaTime = 0;

tmElements_t tm;

byte mac[6] = { 0xBE, 0xEF, 0xED, 0xC0, 0xFF, 0xEE };
byte ip[4] = { 192, 168, 178, 254 };
byte dnsServer[4] = { 192, 168, 178, 1 };
byte gateway[4] = { 192, 168, 178, 1 };
byte subnet[4] = { 255, 255, 255, 0 };

IPAddress timeServer(5, 101, 105, 6);
EthernetUDP udp;
byte packetBuffer[NTP_PACKET_SIZE];

void setup()
{
  Serial.begin(9600);

  while (!Serial) delay(1000);

  Serial.flush();

  Serial.println("Growputer v0.1 initializing . . .");
  Serial.println();

  initHardware();

  Serial.println("Init: Done!");
  Serial.println();

  /*readDate();
  Serial.print(" ");
  readTime();*/

  Serial.println();

  inited = true;
}

void initHardware()
{
  init_DHT22();
  init_DS18B20();

  if (init_DS1307() && init_W5100()) NTP();
}

void init_DHT22()
{
  Serial.println("Init: DHT22");

  dht.begin();
}

void init_DS18B20()
{
  Serial.println("Init: DS18B20");

  sensors.begin();
}

bool init_DS1307()
{
  Serial.println("Init: DS1307");

  RTC.set(0);
  
  if (RTC.read(tm)) return true;
  else return false;
}

bool init_W5100()
{
  Serial.println("Init: W5100");
  
  Ethernet.begin(mac, ip, dnsServer, gateway, subnet);
  
  /*if (Ethernet.begin(mac) == 0)
  {
    Serial.println("      Failed to configure ethernet using DHCP!");

    return false;
  }*/
  
  Serial.print("      IP address from DHCP: "); // static right now
  Serial.println(Ethernet.localIP());

  return true;
}

void read_DHT22()
{
  airHumids[index] = dht.readHumidity();
  airTemps[index] = dht.readTemperature();

  while (airHumids[index] || airTemps[index])
  {
    Serial.println("delay DHT22");
    
    delay(2000);
    break;
    
    airHumids[index] = dht.readHumidity();
    airTemps[index] = dht.readTemperature();
  }
}

void read_DS18B20()
{
  sensors.requestTemperatures();

  waterTemps[index] = sensors.getTempCByIndex(0);
}

void readTime()
{
  while (!RTC.read(tm)) delay(1000);

  Serial.print("");
  print2digits(tm.Hour);
  Serial.print(":");
  print2digits(tm.Minute);
  Serial.print(":");
  print2digits(tm.Second);
}

void readDate()
{
  while (!RTC.read(tm)) delay(1000);

  Serial.print("");
  print2digits(tm.Day);
  Serial.print("-");
  print2digits(tm.Month);
  Serial.print("-");
  print2digits(tmYearToCalendar(tm.Year));
}

void print2digits(int number)
{
  if (number >= 0 && number < 10) Serial.print("0");

  Serial.print(number);
}

void NTP()
{
  udp.begin(LOCALPORT);
  
  setSyncProvider(getNtpTime);
  
  RTC.set(getNtpTime());
}

time_t getNtpTime()
{
  while (udp.parsePacket() > 0);
  
  Serial.println("      Transmiting NTP Request . . .");
  
  sendNTPpacket(timeServer);
  
  uint32_t beginWait = millis();
  
  while (millis() - beginWait < 1500)
  {
    int size = udp.parsePacket();
    
    if (size >= NTP_PACKET_SIZE)
    {
      Serial.println("      Received NTP Response!");
      
      udp.read(packetBuffer, NTP_PACKET_SIZE);
      
      unsigned long secsSince1900;
      
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      
      return secsSince1900 - 2208988800UL + TIMEZONE * SECS_PER_HOUR;
    }
  }
  
  Serial.println("No NTP Response :-(");
  
  return 0;
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void read_sensors()
{
  float currentTime = millis();

  read_DHT22();
  read_DS18B20();

  deltaTime += (millis() - currentTime);

  index++;

  if (index == 10)
  {
    index = 1;

    airHumids[0]  = (airHumids[0]  + airHumids[1]  + airHumids[2]  + airHumids[3]  + airHumids[4]  + airHumids[5]  + airHumids[6]  + airHumids[7]  + airHumids[8]  + airHumids[9])  / 10;
    airTemps[0]   = (airTemps[0]   + airTemps[1]   + airTemps[2]   + airTemps[3]   + airTemps[4]   + airTemps[5]   + airTemps[6]   + airTemps[7]   + airTemps[8]   + airTemps[9])   / 10;
    waterTemps[0] = (waterTemps[0] + waterTemps[1] + waterTemps[2] + waterTemps[3] + waterTemps[4] + waterTemps[5] + waterTemps[6] + waterTemps[7] + waterTemps[8] + waterTemps[9]) / 10;

    Serial.print("Air humidity: ");
    Serial.print(airHumids[0]);
    Serial.println("%");

    Serial.print("Air temperature: ");
    Serial.println(airTemps[0]);

    Serial.print("Water temperature: ");
    Serial.println(waterTemps[0]);

    Serial.println();

    Serial.print("Procrastinated for ");
    Serial.print(deltaTime / 1000);
    Serial.println(" seconds!");

    Serial.println();

    readDate();
    readTime();

    Serial.println();

    deltaTime = 0;
  }
}

void loop()
{
  if (inited)
  {
    readDate();
    Serial.print(" ");
    readTime();
    Serial.println();

    read_sensors();
  }
}
