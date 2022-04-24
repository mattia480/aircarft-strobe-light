/**
 * Arduino Strobe Light for Ultralight Aircraft
 * =====================
 *
 * @author      Current authors: Ing. Mattia Bernasconi <mattia@studiobernasconi.com>
 *              Original author: Ing. Mattia Bernasconi <mattia@studiobernasconi.com>
 * 
 * @versione    1.0.0
 *
 * @license     GNU General Public License v3.0
 *
 * @link        Homepage: http://www.mattiabernasconi.it
 * 
 * @date        2022-04-09
 */
#include <EEPROM.h>

// Set user editable variables
const bool MODE2_ENABLED = false; // Enable mode 2 switching by power
const int MODE2_TAIL_BRIGHTNESS = 22; // Brightness of Tail light (0-255)
const int WINGTIP_STROBE_NUM = 2; // Number of Wingtip flash
const int TAIL_STROBE_NUM = 1; // Number of Tail flash
const int TIME_BETWEEN_STROBE = 1500; // Time between flashes (in millis)

// Set system configuration variables
int strobe_wingtip = 10;
int strobe_tail = 9; 
int beacon = 6;
int mode2 = 0;

// Setup digital pin and peripherals
void setup() {
  const int MODE2_ADDR = 0;

  // Initialize digital pin
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(strobe_wingtip, OUTPUT);
  pinMode(strobe_tail, OUTPUT);
  pinMode(beacon, OUTPUT);

  // Switch off all output
  digitalWrite(strobe_wingtip, LOW);
  digitalWrite(strobe_tail, LOW);
  digitalWrite(beacon, LOW);

  // If mode2 enabled save and read state to/from EEPROM
  if (MODE2_ENABLED) {
    if (EEPROM.read(MODE2_ADDR) == 2) {
      mode2 = 1;
    }
  
    EEPROM.update(MODE2_ADDR, 2);
    delay(1000);
    EEPROM.update(MODE2_ADDR, 0);
  }
}

void loop() {
  // Arduino LED On
  digitalWrite(LED_BUILTIN, HIGH);

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
  if (mode2 == 1) {
    analogWrite(strobe_tail, MODE2_TAIL_BRIGHTNESS);
  } else {
    digitalWrite(strobe_tail, LOW);
  }

  // Arduino LED Off
  digitalWrite(LED_BUILTIN, LOW);

  // Delay after flash
  delay(TIME_BETWEEN_STROBE);
}
