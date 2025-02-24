/*A basic 6 channel transmitter using the nRF24L01 module.*/
/* Like, share and subscribe, ELECTRONOOBS */
/* http://www.youtube/c/electronoobs */

/* First we include the libraries. Download it from
   my webpage if you donw have the NRF24 library */

#include <SPI.h>
#include <nRF24L01.h>             //Downlaod it here: https://www.electronoobs.com/eng_arduino_NRF24_lib.php
#include <RF24.h>
#include <LiquidCrystal_I2C.h>


float vol = 0;
int input = 0;

int vdividerPin = A7;

LiquidCrystal_I2C lcd(0x27, 16, 2);

/*Create a unique pipe out. The receiver has to
  wear the same unique code*/

const uint64_t pipeOut = 0xE8E8F0F0E1LL; //IMPORTANT: The same as in the receiver!!!

RF24 radio(9, 10); // select  CE and CSN  pins

// The sizeof this struct should not exceed 32 bytes
// This gives us up to 32 8 bits channels
struct MyData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
};

MyData data;

// Variables to handle LCD update timing
unsigned long lcdUpdateTime = 0;
const unsigned long lcdUpdateInterval = 100; // Update interval in milliseconds

void resetData()
{
  //This are the start values of each channal
  // Throttle is 0 in order to stop the motors
  //127 is the middle value of the 10ADC.

  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0;
}

void setup()
{
  //Start everything up
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  resetData();

  lcd.init();                          // initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.clear();
  lcd.print("Arduino");
  lcd.setCursor(0, 1);
  lcd.print("Transmitter");
  delay(1200);
  lcd.clear();
  lcd.print("For Arduino");
  lcd.setCursor(0, 1);
  lcd.print("Mini FPV Drone");  
  delay(2200);
  lcd.clear();
  pinMode(vdividerPin, INPUT);
  Serial.begin(9600);               // starting the Serial Monitor

  pinMode(5, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

/**************************************************/

// Returns a corrected value for a joystick position that takes into account
// the values of the outer extents and the middle of the joystick range.
int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);
  if ( val < middle )
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}

void loop()
{
  unsigned long currentMillis = millis();

  // Update LCD display at regular intervals
  if (currentMillis - lcdUpdateTime >= lcdUpdateInterval) {
    updateLCD();
    lcdUpdateTime = currentMillis;
  }
  // The calibration numbers used here should be measured
  // for your joysticks till they send the correct values.
  data.throttle = mapJoystickValues( analogRead(A3), 13, 524, 1015, true );
  data.yaw      = mapJoystickValues( analogRead(A1), 50, 505, 1020, true );
  data.pitch    = mapJoystickValues( analogRead(A2), 12, 544, 1021, true );
  data.roll     = mapJoystickValues( analogRead(A6), 34, 522, 1020, true );
  data.AUX1     = digitalRead(5);
  data.AUX2     = digitalRead(3);

  radio.write(&data, sizeof(MyData));
}

void updateLCD() {
  input = analogRead(vdividerPin);
  vol = (input * 10.0) / 1024.0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltage:");
  lcd.setCursor(0, 1);
  lcd.print(vol);
  lcd.print("V");
}
