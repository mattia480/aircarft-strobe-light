/**
 * Arduino Strobe Light for Ultralight Aircraft
 * =====================
 *
 * @author      Current authors: Ing. Mattia Bernasconi <mattia@studiobernasconi.com>
 *              Original author: Ing. Mattia Bernasconi <mattia@studiobernasconi.com>
 * 
 * @versione    1.1.1
 *
 * @license     GNU General Public License v3.0
 *
 * @link        Homepage: http://www.mattiabernasconi.it
 * 
 * @date        2022-05-08
 * 
 * If "MULTI_MODE_ENABLED" variable is set to true, by powering on and off the device quickly (1.5 second), we can change the operating mode
 * Mode 1 (Standard) -> Wingtip flash "WINGTIP_STROBE_NUM" and Tail flash "TAIL_STROBE_NUM" every "TIME_BETWEEN_STROBE" millis
 * Mode 2            -> Wingtip flash "WINGTIP_STROBE_NUM" and Tail flash "TAIL_STROBE_NUM" every "TIME_BETWEEN_STROBE" millis but Tail Strobe remain at "MODE2_TAIL_BRIGHTNESS" all the time
 * Mode 3            -> Same at Mode 1 but disable temperature check (in case of needed while flyng, i.e. during hot day in summer if "TEMP_ERROR" is set too low)
 * 
 * 3 tones at startup indicate I2C error
 * 6 tones at startup indicate DHT error
 * 
 * N tones after startup indicate mode N selecetd (if "MULTI_MODE_ENABLED" variable is set to true)
 * 
 * Address 500 of I2C EEPROM is used to count DHT error
 * 
 * Serial enabled at baude rate 9600
 */

#define DEBUG // Comment to remove serial debug messages

// Include the Arduino's internal EEPROM Libray
#include <EEPROM.h>

// Include the I2C Wire Library
#include <Wire.h>

// Include the DHT Library
#include <DHT.h>;

// EEPROM I2C Address
#define EEPROM_I2C_ADDRESS 0x50

//DHT PIN
#define DHTPIN 7

//DHT TYPE 21 (AM2301)
#define DHTTYPE DHT21

// Set user editable variables
const bool MULTI_MODE_ENABLED = true; // Enable multi mode switching by power
const bool TEMP_CHECK_ENABLED = true; // Enable system temperature check
const float TEMP_ERROR = 60.0; // Temperature to put the system in protection mode
const int MODE2_TAIL_BRIGHTNESS = 22; // Brightness of Tail light (0-255)
const int WINGTIP_STROBE_NUM = 2; // Number of Wingtip flash
const int TAIL_STROBE_NUM = 1; // Number of Tail flash
const int TIME_BETWEEN_STROBE = 1600; // Time between flashes (in millis)

// Set system configuration variables
int strobe_wingtip = 10;
int strobe_tail = 9; 
int beacon = 6;
int buzzer = 8;

int multi_mode_selector = 0; // Multi mode selector
int MULTI_MODE_EEPROM_ADDR = 0; // Multi mode EEPROM address
int dht_error = 0; // DHT error
int TEMP_ERROR_EEPROM_ADDRESS = 500; //EEPROM Address used to count DHT error

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

// Function to write to I2C EEPROOM
void i2cWriteEEPROM(int address, byte val)
{
  // Begin transmission to I2C EEPROM
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);

  // Send memory address as two 8-bit bytes
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB

  // Send data to be stored
  Wire.write(val);

  // End the transmission
  Wire.endTransmission();

  // Add 5ms delay for EEPROM
  delay(5);
}

// Function to read from I2C EEPROOM
byte i2cReadEEPROM(int address)
{
  // Define byte for received data
  byte rcvData = 0xFF;

  // Begin transmission to I2C EEPROM
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);

  // Send memory address as two 8-bit bytes
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB

  // End the transmission
  Wire.endTransmission();

  // Request one byte of data at current memory address
  Wire.requestFrom(EEPROM_I2C_ADDRESS, 1);

  // Read the data and assign to variable
  rcvData =  Wire.read();

  // Return the data as function output
  return rcvData;
}

// Function to check external temp and block the system if temp is over TEMP_ERROR user's value
void tempCheck() {
  // First temperature and humidity read
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  // Check if any reads failed and increment error counter
  if (isnan(hum) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    dht_error++;
  } else {
    dht_error = 0;
  }

  // Write error into external EEPROM
  if(dht_error == 1) {
    int eeprom_temp_error = i2cReadEEPROM(TEMP_ERROR_EEPROM_ADDRESS);
    eeprom_temp_error++;
    i2cWriteEEPROM(TEMP_ERROR_EEPROM_ADDRESS, eeprom_temp_error);
  }

  // If the DHT sensor is in error for more than 15 times (30sec) or there is an overtemperature stop the system
  if (dht_error >= 15 || temp >= TEMP_ERROR) {
    unsigned long error_time = (millis() / 1000);
    do {
      temp = dht.readTemperature();
      Serial.println(F("The temperature is too high or DHT sensor is unavailable!"));
      Serial.print("Current reading temperature: "); Serial.print(temp);
      Serial.println(" Celsius");
      
      // Turn on buzzer
      digitalWrite(buzzer, HIGH);
      delay(500);
      // Turn off buzzer
      digitalWrite(buzzer, LOW);

      // Wait 1 second before read again
      delay(1000);

      // If more than 3 minutes have passed turn on the buzzer to notify the problem
      if ((millis() / 1000) - error_time > 180) {
        // Turn on buzzer for at least 5 minutes
        digitalWrite(buzzer, HIGH);
        delay(300000);
      }
    } while (temp >= TEMP_ERROR);

    // Turn off buzzer
    digitalWrite(buzzer, LOW);
  }
  
  // Print temp and humidity values to serial monitor if debug enabled
  #ifdef DEBUG
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print("%, Temperature: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  #endif
}

// Setup digital pin and peripherals
void setup() {
  Serial.begin(9600);
  Serial.println("System is starting up. Please wait...");

  // Connect to I2C BUS as Master
  Wire.begin();

  // Initizalize DHT sensor
  dht.begin();

  // Initialize digital pin
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(strobe_wingtip, OUTPUT);
  pinMode(strobe_tail, OUTPUT);
  pinMode(beacon, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Switch off all output
  digitalWrite(strobe_wingtip, LOW);
  digitalWrite(strobe_tail, LOW);
  digitalWrite(beacon, LOW);
  digitalWrite(buzzer, LOW);

  // Check I2C EEPROM Connection
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  byte i2c_eeprom_error = Wire.endTransmission();
  if (i2c_eeprom_error == 0) {
    Serial.println("I2C EEPROM device OK!");
  } else {
    Serial.println("I2C EEPROM device not found!");
    // Tone 3 time to declare I2C EEPROM Error 
    for (int i = 0; i < 3; i++) {
      digitalWrite(buzzer, HIGH);
      delay(150);
      digitalWrite(buzzer, LOW);
      delay(150);
    }
  }
  
  // If multi mode enabled save and read state to/from EEPROM
  if (MULTI_MODE_ENABLED) {
    // External EEPROM is available
    if (i2c_eeprom_error == 0) {
      // Read EEPROM
      multi_mode_selector = i2cReadEEPROM(MULTI_MODE_EEPROM_ADDR);
      // Increment mode readed
      multi_mode_selector++;

      // Write actual mode to EEPROM
      i2cWriteEEPROM(MULTI_MODE_EEPROM_ADDR, multi_mode_selector);

      // After 1 second we can safely put to mode 0 (standard)
      delay(1100);
      i2cWriteEEPROM(MULTI_MODE_EEPROM_ADDR, 0);
    } else {
      // Read EEPROM
      multi_mode_selector = EEPROM.read(MULTI_MODE_EEPROM_ADDR);
      // Increment mode readed
      multi_mode_selector++;

      // Write actual mode to EEPROM
      EEPROM.update(MULTI_MODE_EEPROM_ADDR, multi_mode_selector);

      // After 1 second we can safely put to mode 0 (standard)
      delay(1100);
      EEPROM.update(MULTI_MODE_EEPROM_ADDR, 0);
    }
  } else {
    delay(500); // Fix DHT startup on Nano Every
  }

  // Check DHT Sensor
  if (dht.read(true) != 0) {
    Serial.println("DHT device OK!");
    Serial.print("DHT Error counter read from EEPROM: ");
    Serial.println(i2cReadEEPROM(TEMP_ERROR_EEPROM_ADDRESS));
  } else {
    Serial.println("DHT device not found!");
    // Tone 6 time to declare DHT device error 
    for (int i = 0; i < 6; i++) {
      digitalWrite(buzzer, HIGH);
      delay(150);
      digitalWrite(buzzer, LOW);
      delay(150);
    }
  }

  // Buzzer sound of mode enabled
  for (int i = 0; i < multi_mode_selector; i++) {
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(150);
  }

  Serial.println("System startup complete!");
  Serial.print("Mode ");
  Serial.print(multi_mode_selector);
  Serial.println(" selected");
}

void loop() {
  // Arduino LED On
  digitalWrite(LED_BUILTIN, HIGH);

  // Check temperature before flash (if enabled)
  if (TEMP_CHECK_ENABLED && multi_mode_selector != 3)
    tempCheck();
  
  // Wingtip flash
  for (int i = 0; i < WINGTIP_STROBE_NUM; i++) {
    digitalWrite(strobe_wingtip, HIGH);
    delay(70);
    digitalWrite(strobe_wingtip, LOW);
    if (i+1 < WINGTIP_STROBE_NUM)
      delay(70);
  }

  // Tail flash
  for (int i = 0; i < TAIL_STROBE_NUM; i++) {
    digitalWrite(strobe_tail, HIGH);
    delay(70);
    digitalWrite(strobe_tail, LOW);
    if (i+1 < TAIL_STROBE_NUM)
      delay(70);
  }
  if (multi_mode_selector == 2) {
    analogWrite(strobe_tail, MODE2_TAIL_BRIGHTNESS);
  } else {
    digitalWrite(strobe_tail, LOW);
  }

  // Arduino LED Off
  digitalWrite(LED_BUILTIN, LOW);

  // Delay after flash
  delay(TIME_BETWEEN_STROBE);
}
