//#include arduino.h // not needed in arduino IDE
//****************** Setup DHT22 **********************
#include <DHT.h>
#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
// init DHT sensor
DHT dht(DHTPIN, DHTTYPE);
//*****************************************************
void setup()
{
  Serial.begin(9600);
  Serial.println("DHTxx test!");
    sensors();
    set_time();
}

void sensors()
{
    init_DHT22();
    init_DS1307();
    init_W5100();
    // connected?
    // connected = true
    setup_MySQL();
    set_time();

    // inserted?
    // inserted = true
    setup_SQLite();
}

void init_DHT22()
{
    // initialize air humidity and air temperature sensor
  dht.begin();
}

void init_DS18B20()
{
    // initialize water temperature sensor
}

void init_DS1307()
{
    // initialize RTC
}

void init_W5100()
{
    // initialize ethernet shield

    // return connected, inserted
}

void setup_MySQL()
{
    // initialize MySQL connection to server
}

void write_MySQL()
{
    // sent data to database
}

void setup_SQLite()
{
    // initialize SQLite on SD
}

void write_SQLite()
{
    //write data to database
}

void read_DHT22()
{
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}

void read_DS18B20()
{
    // return water temperature
}

void read_DS1307()
{
    // return RTC data
}

void write_DS1307()
{
    // write RTC
}

void set_time()
{
    init_DS1307();
//commented out because will not compile...
//    write_DS1307(NTP());
}

void NTP()
{
    // return time from nl.pool.ntp.org
}

void read_sensors()
{
    read_DHT22();
    read_DS18B20();
    read_DS1307();

    // connected?
    write_MySQL();

    // inserted?
    write_SQLite();

    // delay one minute
    delay(1000);
}

void loop()
{
    read_sensors();
}
