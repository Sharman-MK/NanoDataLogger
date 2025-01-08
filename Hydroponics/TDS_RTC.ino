#include <virtuabotixRTC.h>
#include <SPI.h>
#include <SD.h>

// RTC Configuration
virtuabotixRTC myRTC(6, 7, 8);

// TDS Sensor Configuration
#define TdsSensorPin A0   // TDS sensor pin
#define VREF 5.0          // Reference voltage for Arduino (5V)
#define SCOUNT 30         // Sample count

int analogBuffer[SCOUNT];    // Store analog values
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;

// SD Card Configuration
File myFile;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize RTC
  myRTC.setDS1302Time(00, 12, 15, 1, 25, 11, 2024);

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
    myFile.println("TDS Logger Initialized");
    myFile.println("Date, Time, TDS (ppm)");
    myFile.close();
    Serial.println("File Created Successfully.");
  } else {
    Serial.println("File Creation Failed!");
    while (true); // Halt if file creation fails
  }
}

void loop() {
  static unsigned long analogSampleTime = millis();
  static unsigned long printTime = millis();

  // Collect analog samples every 40ms
  if (millis() - analogSampleTime > 40) {
    analogSampleTime = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }

  // Calculate and log TDS value every 800ms
  if (millis() - printTime > 800) {
    printTime = millis();

    // Calculate average voltage
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      averageVoltage += analogBuffer[copyIndex];
    }
    averageVoltage = averageVoltage / SCOUNT;
    averageVoltage = (averageVoltage * VREF) / 1024.0; // Convert ADC value to voltage

    // Calculate TDS value using a factor
    tdsValue = (133.42 * averageVoltage * averageVoltage * averageVoltage 
                - 255.86 * averageVoltage * averageVoltage 
                + 857.39 * averageVoltage) * 0.5;

    // Update RTC time
    myRTC.updateTime();

    // Prepare data to log
    String logData = String(myRTC.dayofmonth) + "/" + String(myRTC.month) + "/" + String(myRTC.year) + ", " +
                     String(myRTC.hours) + ":" + String(myRTC.minutes) + ":" + String(myRTC.seconds) + ", " +
                     String(tdsValue) + " ppm";

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

    // Reset averageVoltage for next reading
    averageVoltage = 0;
  }
  delay(5000);
}
