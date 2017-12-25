/*
 * MML Music Tone example
 * MmlMusic library example for Arduino IDE
 * 
 * This example demonstrates playing MML music on a piezo speaker
 * using the Arduino tone() function.
 * A callback function is defined to call the tone()/noTone() functions.
 * 
 * For more information see: 
 *     https://github.com/maxint-rd/MmlMusic
 *     https://github.com/maxint-rd/MmlMusicPWM
 *
 * Made by MMOLE (@maxint-rd) 2017
 * 
 * For more MML music go to http://www.archeagemmllibrary.com/
 */

// Include header files for the used libraries
#include <MmlMusic.h>

#if defined(__AVR_ATtiny85__)
  // ATtiny85 has no regular hardware serial. You can use TinyDebugSerial (TX=3) or software serial
  // For more info see  http://www.ernstc.dk/arduino/tinycom.html
  //#include <SoftwareSerial.h>
  //SoftwareSerial mySerial(4, 3); // RX, TX

  #include <TinyDebugSerial.h>
  TinyDebugSerial mySerial = TinyDebugSerial();   // Debug TX on PB3, no RX, only 9600, 38400 or 115200
  #define Serial mySerial
  // To use the Arduino tone() function on ATtiny85 the board core needs to support it.
  // Note that the Arduino tone() function is not supported in the standard Arduino "core" for ATtiny85 by D.A. Mellis.
  // Alternative cores: http://www.leonardomiliani.com/en/ , https://github.com/SpenceKonde/ATTinyCore
  #define BUZ_PIN 1
#elif defined(ARDUINO_ARCH_ESP8266)
  #define BUZ_PIN 14
#else
  #define BUZ_PIN 4  // pin 4 recommended on Pro Mini since it has perfect distance from GND
#endif

// define the MML Music object
MmlMusic music;    // Note: don't use empty parenthesis () to use the (void) constructor; this will confuse the compiler.

// To play the tones using tone() a callback functions are  used.
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
  }

  if(frequency==0 && length==0)
  {   // stop specific track
    noTone(BUZ_PIN);
  }
  else if(frequency>0)
  { // tone
    tone(BUZ_PIN, frequency);
  }
  return(true);    // return true to indicate callback made the tone and wants original playTone NOT to continue
}

// Arduino core has no definition for FPSTR
// on ESP in wstring.h: #define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#if !defined(FPSTR)
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#endif
// Unfortunately the definition above won't compile on Arduino IDE 1.6.7. It will compile in IDE 1.8.2
// You can upgrade or remove this code and the use of MyPlayCallback below.

// Callback functions can also be set to indicate start or end of playing the music sequence
void MyPlayCallback(const char* mml, bool fUseFlash)
{ // Called prior to start of plsy. Note mml may point to PROGMEM string, depending on call to play or play_P
  Serial.print(F("Play "));
  //Serial.println(fUseFlash ? FPSTR(mml) : mml); // can't use conditional expression with distinct pointer types
  if(fUseFlash)
    Serial.println(FPSTR(mml));
  else
    Serial.println(mml);
}

void MyCompletionCallback(void)
{ // Called prior when done playing
  Serial.println(" Done!");
}

void delayR(uint16_t ms)
{ // Without interrupts, normal delay will not continue playing.
  // Therefor we regularly need to call continuePlaying().
  unsigned long ulEnd=millis()+ms;
  while(millis()<ulEnd)
  {
    if(music.isPlaying())
      music.continuePlaying();
    delay(1);   // on ESP we need to yield to feed the watchdog
  }
}

/*
// Star Wars – The Force theme (originally two tracks, only one played)
const char szPlay[] PROGMEM="v127t100l4o4 r g>c2d.d16+f16d+2<g.l8g>c4.dd+<d+>l12d+cgf2.<g4>c4.l8dd+.<g16>g.d+16>c2<c4l12d+dcg4&gd+c<g4g8.g16>c2";
/**/

/**/
// Star Wars Theme (originally three tracks, only one played. Short version is for ATtiny85.
//const char szPlay[] PROGMEM="T107o6c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>f16e16d16c16<g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16.>c2<g4f16.e16f16.d4<g4g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16.";
const char szPlay[] PROGMEM="T107o5c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>c16<b16a16g16>f16e16d16c16c16<b16a16g16>c16<b16a16g16>f16e16d16c16<g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16.>c2<g4f16.e16f16.d4<g4g16.g16g16.>c2g2f16.e16d16.>c2<g4f16.e16d16.>c2<g4>d16.c16<b16.>c4<c16.c16c16.c4";
/**/

void setup()
{
  Serial.begin(115200);
  Serial.println(F("\n -- mxUnifiedSN76489 MML Music Tone example --"));

  // set callback functions
  music.setPlayCallback(MyPlayCallback);
  music.setToneCallback(MyToneCallback);
  music.setCompletionCallback(MyCompletionCallback);

  // Start playing some music (if impatient use the short tune of the lower line).
  music.play_P(szPlay);
  //music.play_P(PSTR("T180 L8 CDEC. r CDEC. r EFG. r EFG. r GAGFEC. r GAGFEC. r L4 C<A>C. r C<A>C."));
  //music.play("T180 L8 CDEC. r CDEC. r EFG. r EFG. r GAGFEC. r GAGFEC. r L4 C<A>C. r C<A>C.");
}

// the loop function runs over and over again forever
void loop()
{
  if(music.isPlaying())
    music.continuePlaying(); // Without timer interupts we regularly need to call continuePlaying()
                             // To simplify this use the MmlMusicPWM child library
  else
  {
    music.play_P(PSTR("T240 O4 L64 AB>C")); // give a short blurp
    delayR(1000);   // use alternative delay to continue playing as needed
  }
}
