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
 * @date        2024-10-06
 * 
 * Serial enabled at baude rate 9600
 */

#define DEBUG // Comment to remove serial debug messages

// Set user editable variables
const int NAVIGATION_BRIGHTNESS = 115; // Brightness of Navigation lights (0-255) - 115≈11V with 13.8V on Vin
const int WINGTIP_STROBE_NUM = 2; // Number of Wingtip flash
const int TAIL_STROBE_NUM = 1; // Number of Tail flash
const int TIME_BETWEEN_STROBE = 1800; // Time between flashes (in millis)

// Set system configuration variables
int navigation = 10;
int strobe_wingtip = 9;
int strobe_tail = 6; 

// Setup digital pin and peripherals
void setup() {
  Serial.begin(9600);
  Serial.println("System is starting up. Please wait...");

  // Initialize digital pin
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(navigation, OUTPUT);
  pinMode(strobe_wingtip, OUTPUT);
  pinMode(strobe_tail, OUTPUT);

  // Switch off all output
  digitalWrite(navigation, LOW);
  digitalWrite(strobe_wingtip, LOW);
  digitalWrite(strobe_tail, LOW);

  Serial.println("System startup complete!");

  // Switch on Navigation lights
  analogWrite(navigation, NAVIGATION_BRIGHTNESS);
}

void loop() {
  // Arduino LED On
  digitalWrite(LED_BUILTIN, HIGH);

  // Wingtip flash
  for (int i = 0; i < WINGTIP_STROBE_NUM; i++) {
    digitalWrite(strobe_wingtip, HIGH);
    delay(100);
    digitalWrite(strobe_wingtip, LOW);
    if (i+1 < WINGTIP_STROBE_NUM)
      delay(100);
  }

  // Tail flash
  for (int i = 0; i < TAIL_STROBE_NUM; i++) {
    digitalWrite(strobe_tail, HIGH);
    delay(100);
    digitalWrite(strobe_tail, LOW);
    if (i+1 < TAIL_STROBE_NUM)
      delay(100);
  }

  // Arduino LED Off
  digitalWrite(LED_BUILTIN, LOW);

  // Delay after flash
  delay(TIME_BETWEEN_STROBE);
}
