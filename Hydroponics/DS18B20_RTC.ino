#include <virtuabotixRTC.h>
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// RTC Configuration
virtuabotixRTC myRTC(6, 7, 8);

// DS18B20 Sensor Configuration
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// SD Card Configuration
File myFile;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize RTC
  myRTC.setDS1302Time(00, 12, 15, 1, 25, 11, 2024);

  // Initialize DS18B20 Sensor
  sensors.begin();
  Serial.println("DS18B20 Temperature Sensor Initialized");

  // Initialize SD Card
  Serial.println("Initializing SD Card...");
  if (!SD.begin(4)) {
    Serial.println("SD Card Initialization Failed!");
    while (true); // Halt if SD card initialization fails
  }
  Serial.println("SD Card Initialized Successfully.");

  // Create or open the data log file
  Serial.println("Creating File...");
  myFile = SD.open("datalog.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("Temperature Logger Initialized");
    myFile.println("Date, Time, Temperature (°C)");
    myFile.close();
    Serial.println("File Created Successfully.");
  } else {
    Serial.println("File Creation Failed!");
    while (true); // Halt if file creation fails
  }
}

void loop() {
  static unsigned long printTime = millis();

  // Read and log temperature every second
  if (millis() - printTime > 1000) {
    printTime = millis();

    // Request temperature readings
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    if (temperatureC != DEVICE_DISCONNECTED_C) {
      // Update RTC time
      myRTC.updateTime();

      // Prepare data to log
      String logData = String(myRTC.dayofmonth) + "/" + String(myRTC.month) + "/" + String(myRTC.year) + ", " +
                       String(myRTC.hours) + ":" + String(myRTC.minutes) + ":" + String(myRTC.seconds) + ", " +
                       String(temperatureC) + " °C";

      // Log data to SD card
      myFile = SD.open("datalog.txt", FILE_WRITE);
      if (myFile) {
        myFile.println(logData);
        myFile.close();
        Serial.println("Data Logged: " + logData);
      } else {
        Serial.println("Error Writing to File!");
      }

      // Display data on Serial Monitor
      Serial.println("Current Date / Time: " + logData);
    } else {
      Serial.println("Error: Temperature Sensor Not Found!");
    }
  }
}
