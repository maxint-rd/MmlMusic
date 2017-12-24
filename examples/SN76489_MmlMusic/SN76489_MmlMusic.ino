/*
 * mxUnifiedSN76489 MML Music
 * mxUnifiedSN76489 library example for Arduino IDE
 * 
 * Tbis example demonstrates playing MML music on the
 * SN76489 sound generator.
 * Using the mxUnifiedIO libraries, The SN76489 can be connected
 * to the MCU (e.g. Arduino Uno or ESP8266) via a shift
 * register or an I2C I/O expander.
 * 
 * For more information see: 
 *     https://github.com/maxint-rd/mxUnifiedSN76489
 *     https://github.com/maxint-rd/mxUnifiedIO
 *     https://github.com/maxint-rd/MmlMusic
 *
 * Made by MMOLE (@maxint-rd) 2017
 * 
 * For more MML music go to http://www.archeagemmllibrary.com/
 */

// Include header files for the used libraries
#include <mxUnifiedPCF8574.h>     // Use the PCF8574/8575 I/O-expander
//#include <mxUnified74HC595.h>   // use the 74HC595 shift-register
#include <mxUnifiedSN76489.h>
#include <MmlMusic.h>

#if defined(MXUNIFIED_ATTINY)
  // ATtiny85 has no regular hardware serial. You can use TinyDebugSerial (TX=3) or software serial
  // For more info see  http://www.ernstc.dk/arduino/tinycom.html
  //#include <SoftwareSerial.h>
  //SoftwareSerial mySerial(4, 3); // RX, TX

  #include <TinyDebugSerial.h>
  TinyDebugSerial mySerial = TinyDebugSerial();   // Debug TX on PB3, no RX, only 9600, 38400 or 115200
  #define Serial mySerial
#endif

// When using one or more shift registers, these connections and definitions can be used.
// Hardware SPI pins for shift registers:
//   ATmega328: SS=10, MOSI=11, SCLK=13
//   ATtiny85:  SS=0, MOSI=1, SCLK=2
//   ESP8266:   SS=15, MOSI=13, SCLK=14
//   Suggested pins ESP-01:   SS=3 (RX), MOSI=2, SCLK=0

// create the 74HC595 output object
//mxUnified74HC595 unio = mxUnified74HC595();                  // use hardware SPI pins, no cascading
//mxUnified74HC595 unio = mxUnified74HC595(3, 2, 0);      // alternative software SPI pins for ESP-01: SS, MOSI, SCLK, no cascading (slower, but pin-freedom)
//mxUnified74HC595 unio = mxUnified74HC595(10, 11, 13, 2);   // alternative software SPI pins: SS, MOSI, SCLK, two cascaded shift-registers
//mxUnified74HC595 unio = mxUnified74HC595(0, 1, 2);      // alternative software SPI pins for Attiny85: SS, MOSI, SCLK, no cascading

// When using a PCF8574/8575 I/O-expander, these connections and definitions can be used.
// Hardware I2C on ESP8266/D1-mini: SCL=GPIO5/D1, SDA=GPIO4/D2

// Use the PCF8574/8575 I/O-expander
mxUnifiedPCF8575 unio = mxUnifiedPCF8575(0x20);     // use the PCF875 I2C output expander on address 0x20

// define the pins used and initialize the mxUnifiedSN76489 object
mxUnifiedSN76489 sound(&unio);     // use expanded pisn 0-7 for data, expanded pin 8 for NotWE
//mxUnifiedSN76489 sound(&unio,4);   // use expanded pins 0-7 for data, MCU pin 4 for NotWE

// define the MML Music object
MmlMusic music;    // Note: don't use empty parenthesis () to use the (void) constructor; this will confuse the compiler.

// To play the tones on the SN76489 a callback functions are used.
// These are also used to display debug information on the serial console
// Resources are limited on the ATtiny85, so you may need to comment things out to preserve memory.
bool MyToneCallback(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume)
{   // the tone callback is called on three occasions
  // 1. play a tone of certain frequency for specified length or indefinitely (len=0)
  // 2. wait for a certain period while tone or silence is played (freq=0, len=msec)
  // 3. stop playing (freq=0; len=0)
  
  if(frequency==0 && length==0)
  {
    //Serial.println(F("shhht"));
  }
  else
  {
    if(frequency>0)
    {
      Serial.print(nTrack);
      Serial.print(F("t:"));
      Serial.print(frequency);
      Serial.print(F(" "));
    }
    else
    {
      //Serial.print(F("wait"));
    }

/*
    if(length>0)
    {
      Serial.print(F(" len:"));
      Serial.print(length);
    }
*/  
  }

  if(frequency==0 && length==0)
  {   // stop specific track
    sound.noTone(nTrack);
  }
  else if(frequency>0)
  { // tone
    uint8_t nVol=15-(nVolume>>3);   // map volume: quiet=0 - loud=128 to quiet=15 - loud=0 
    sound.tone((float)frequency, nVol, nTrack);
  }
  return(true);    // return true to indicate callback made the tone and wants original playTone NOT to continue
}

 // Arduino core has no definition for FPSTR
// on ESP in wstring.h: #define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#if !defined(FPSTR)
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#endif

// Callback functions can also be set to indicate start or end of playing the music sequence
void MyPlayCallback(const char* mml, bool fUseFlash)
{ // Called prior to start of plsy. Note mml may point to PROGMEM string, depending on call to play or play_P
  // on ESP in wstring.h: #define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
  Serial.print(F("Play "));
  //Serial.println(fUseFlash ? FPSTR(mml) : mml); // cant use conditional expression with distinct pointer types
  if(fUseFlash)
    Serial.println(FPSTR(mml)); // Arduino core has no definition for FPSTR
  else
    Serial.println(mml);
}

void MyCompletionCallback(void)
{ // Called prior when done playing
  Serial.println(" Done!");
}

void delayR(uint16_t ms)
{ // without interrupts, normal delay will not continue Playing
  unsigned long ulEnd=millis()+ms;
  while(millis()<ulEnd)
  {
    if(music.isPlaying())
      music.continuePlaying();
    delay(1);   // on ESP we need to yield to feed the watchdog
  }
}

/**/
// Star Wars – The Force theme (two tracks)
const char szPlay[] PROGMEM="v127t100l4o4 r g>c2d.d16+f16d+2<g.l8g>c4.dd+<d+>l12d+cgf2.<g4>c4.l8dd+.<g16>g.d+16>c2<c4l12d+dcg4&gd+c<g4g8.g16>c2"
                              ",v127t100l8o3 r4cg>cd+fdc<gcg>cd+dc<bgcg>cd+ggc<gf>cfa>c4<d<gcg>cd+gd+<a+4g+>cd+g+4.<g+4g>cd+gc4<b4c2 r";
/**/
/*
// Star Wars Theme (three tracks)
// short version for ATtiny85: const char szPlay2[] PROGMEM="T107o6c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>f16e16d16c16<g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16.>c2<g4f16.e16f16.d4<g4g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16."
const char szPlay[] PROGMEM="T107o6c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>f16e16d16c16<g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16.>c2<g4f16.e16f16.d4<g4g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16.>c2<g4>d16.c16<b16.>c4<c16.c16c16.c4"
",T107o4b8r8>c8r4c8r8c8r8c8r8c8c8r8c8r8<b8r8>c8r4c8r8c8r8c8r8c8c8r8<g16.g16g16.e2>c2<a4>e4f4c4<a4>e4f4c4d16.c+16d16.<b4d4d16.d16d16.e2>c2<a16.r8r32>e2c4<a4>e4f4c4g16.r8r32<g4c16.c16c16.c4"
",T107o3c8r8 e8r4f8r8f8r8e8r8g8f8r8f8r8c8r8e8r4f8r8f8r8e8r8g8f8r8<g16.g16g16.>c4<b4a4g4f4e4d4c4f4e4d4c4>a+16.a16a+16.g4c4c16.c16c16.c4<b4a4g4a4g4f4e4f4e4d4c4g16.g16g16.>c4<c16.c16c16.c4";
/**/

void setup()
{
  Serial.begin(115200);
  Serial.println(F("mxUnifiedSN76489 MML Music"));
  
  // initialize the sound chip
  //unio.setBitOrder(LSBFIRST);   // reverse connections on 74HC595 (could also be done by changing wires)
  unio.begin();     // start using the mxUnified74HC595 I2C shift register
  sound.begin();

  // set callback functions
  music.setPlayCallback(MyPlayCallback);
  music.setToneCallback(MyToneCallback);
  music.setCompletionCallback(MyCompletionCallback);

  // Start playing some music (if impatient use the short tune of the lower line).
  music.play_P(szPlay);
  //music.play_P("T180 L8 CDEC. r CDEC. r EFG. r EFG. r GAGFEC. r GAGFEC. r L4 C<A>C. r C<A>C.");
}


// the loop function runs over and over again forever
void loop()
{
  if(music.isPlaying())
    music.continuePlaying();
  else
  {
    music.play_P(PSTR("T240 O4 L64 AB>C, T240 O4 L32 CBA V0 A \0\0")); // give a short beep
    delayR(1000);   // without interrupts, delay will make playing to not continue on time
  }
}



