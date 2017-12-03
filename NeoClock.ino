//NeoPixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//RTC
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

//Gamma correction table
const uint8_t gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

//Monthly tables for sunrise og sunset.
const uint8_t sunrise[] = {521, 465, 395, 374, 306, 271, 296, 349, 410, 470, 476, 525};
const uint8_t sunset[] = {976, 1042, 1102, 1226, 1286, 1328, 1316, 1257, 1177, 1099, 971, 943};
//Transition time for changing brightness (in minutes). Total time from base to max is tTime*2
const int tTime = 60;
//Minimum and maximum brightness %. 
const int minBrightness = 10;
const int maxBrightness = 100;

//NeoPixel pin
#define PIN            2
//Amount of neopixels
#define NUMPIXELS      3

#define LDRPIN         A7

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void setup() {
  Wire.begin();
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(100);
  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year
  // setDS3231time(40,24,16,7,3,12,17);
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

uint32_t mapToColor(int pos){
 int red;
 int green;
 int blue; 
 
 if (pos <= 60){
   red = 255;
   green = map(pos,0,60,0,255);
   blue = 0;
 }else if (pos <= 120){
   red = map(pos,60,120,255,0);
   green = 255;
   blue = 0;
 }else if (pos <= 180){
   red = 0;
   green = 255;
   blue = map(pos,120,180,0,255);
 }else if (pos <= 240){
   red = 0;
   green = map(pos,180,240,255,0);
   blue = 255;
 }else if (pos <= 300){
   red = map(pos,240,300,0,255);
   green = 0;
   blue = 255;
 }else if (pos <= 360){
   red = 255;
   green = 0;
   blue = map(pos,300,360,255,0);
 }
 return pixels.Color(gamma[red], gamma[green], gamma[blue]);
}

int calcBrightness(int month, int hour, int minute){
  int lmonth = month-1;
  int sr = sunrise[lmonth];
  int ss = sunset[lmonth];
  int brightness = minBrightness;
  int cTime = hour*60+minute;
  if (cTime >= sr-tTime && cTime < sr+tTime){
    brightness = map(cTime,sr-tTime,sr+tTime,minBrightness,maxBrightness);
  }
  else if (cTime >= sr+tTime && cTime < ss-tTime){
    brightness = maxBrightness;
  } 
  else if (cTime >= ss-tTime && cTime < ss+tTime){
    brightness = map(cTime,sr-tTime,sr+tTime,maxBrightness,minBrightness);
  }

  return brightness;
}

int getLDRBrightness(){
  int lSensor = analogRead(LDRPIN);
  int brightness = map(lSensor, 400, 1023, minBrightness, maxBrightness);
  return brightness;
}

void loop() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  
  uint32_t secondColor = mapToColor(map(second, 0, 60, 0, 360));
  uint32_t minuteColor = mapToColor(map(minute, 0, 60, 0, 360));
  uint32_t hourColor = mapToColor(map(hour % 12, 0, 24, 0, 360));
  
  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  pixels.setPixelColor(0, secondColor);
  pixels.setPixelColor(1, minuteColor);
  pixels.setPixelColor(2, hourColor);
  
  pixels.show(); // This sends the updated pixel color to the hardware.
  //If new minute, calculate new brightness and set it.
  /*if (second == 0){
    pixels.setBrightness(calcBrightness(month, hour, minute));
  }*/
  pixels.setBrightness(getLDRBrightness());

  delay(1000); // Delay for a second
  

}

