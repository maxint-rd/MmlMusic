// MML Music Library - MmlMusic base class
// Play multitrack MML music using different sound devices
//
// Derived from Retro Music Engine
// Author: Chris Taylor (taylorza)
// see https://developer.mbed.org/users/taylorza/code/MusicEngine/
// Ported from mBed to ESP8266/Arduino by MMOLE (mm_rd@maxint.nl)

// MmlMusic provides a means to play Music Macro Language sequences.
// http://www.github.com/maxint-rd/MmlMusic
// Learn more about Music Macro Language (MML) on wikipedia
// http://en.wikipedia.org/wiki/Music_Macro_Language
// For music see http://www.archeagemmllibrary.com/


#include "MmlMusic.h"


//#include <Arduino.h>
#if defined (ARDUINO_ARCH_ESP8266)
    #include <cctype>
#endif

const float MmlMusicTrack::WHOLE_NOTE_DURATION = 1.0f;
const float MmlMusicTrack::QUARTER_NOTE_DURATION = MmlMusicTrack::WHOLE_NOTE_DURATION / 4.0f;
const float MmlMusicTrack::QUARTER_NOTES_PER_MINUTE = 60.0f / MmlMusicTrack::QUARTER_NOTE_DURATION;

const float MmlMusicTrack::DEFAULT_TIMING = 7.0f / 8.0f;
const float MmlMusicTrack::LEGATO_TIMING = 1.0f;
const float MmlMusicTrack::STACCATO_TIMING = 3.0f / 4.0f;

const uint8_t MmlMusicTrack::NOTE_REST = 0;
const uint8_t MmlMusicTrack::NOTE_C = 1;
const uint8_t MmlMusicTrack::NOTE_CS = 2;
const uint8_t MmlMusicTrack::NOTE_D = 3;
const uint8_t MmlMusicTrack::NOTE_DS = 4;
const uint8_t MmlMusicTrack::NOTE_E = 5;
const uint8_t MmlMusicTrack::NOTE_F = 6;
const uint8_t MmlMusicTrack::NOTE_FS = 7;
const uint8_t MmlMusicTrack::NOTE_G = 8;
const uint8_t MmlMusicTrack::NOTE_GS = 9;
const uint8_t MmlMusicTrack::NOTE_A = 10;
const uint8_t MmlMusicTrack::NOTE_AS = 11;
const uint8_t MmlMusicTrack::NOTE_B = 12;

MmlMusic::MmlMusic(void)
//    : _isPlaying(false)
    : _nNumTracks(0)
#if(OPT_MMLMUSIC_PLAYCALLBACK)
    ,	_playCallback(NULL)
#endif
    , _completionCallback(NULL)
    , _toneCallback(NULL)
    , _fUseFlash(false)
{	// nothing?
	_fUseFlash=false;
}


bool MmlMusic::callToneCallback(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume)
{
  if (_toneCallback)
 	{
		return(_toneCallback(frequency, length, nTrack, nVolume));
		//return(true);
	}
	return(false);
}
void MmlMusic::playTone(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume)		// default: nTrack=0
{	// This virtual public method may be implemented by the child class and can be
	// called by the toneCallBack function.
  // The playTone method is called only when there is no toneCallBack function set,
  // on the following three occasions:
  // 1. play a tone of certain frequency for specified length or indefinitely (len=0)
  // 2. wait for a certain period while tone or silence is played (freq=0, len=msec)
  // 3. stop playing (freq=0; len=0)	// The code below can be used as a template
  if(frequency==0 && length==0)
	{ // stop playing the tone
	}
  else if(frequency>0)
  {	// start playing the tone for the specified lenght, then execute next command
	}
	else
	//if(length > 0)
  {	// continue playing or waiting until lenght has passed, then continue with the next command
  	continuePlaying();		// should only be called when length is passed
	}
}

void MmlMusic::playToneWithCB(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume)		// default: nTrack=0
{
	if(callToneCallback(frequency, length, nTrack, nVolume))
		return;
	playTone(frequency, length, nTrack, nVolume);
}

/**/


void MmlMusic::tone(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume)	// defaults: length=0,	nTrack=0
{
		playToneWithCB(frequency, length, nTrack, nVolume);
}

void MmlMusic::noTone(uint8_t nTrack)		// default nTrack=0
{
		playToneWithCB(0, 0, nTrack);
}

void MmlMusic::waitTone(unsigned long length, uint8_t nTrack)	// defaults: length=0,	nTrack=0
{		// schedule wait time. length is specified in msec
		tracks[nTrack]._ulNextTimeMS=millis()+length;
		playToneWithCB(0, length, nTrack);
}


void MmlMusic::playMML(const char* mml)
{
//Serial.print(F("--DEBUG MmlMusic play:"));
//Serial.print(mml);
//Serial.println(F("--"));
    //    __disable_irq();
		//noInterrupts();
#if(OPT_MMLMUSIC_PLAYCALLBACK)
    if (_playCallback)
        _playCallback(mml, _fUseFlash);
#endif
//    _mml = mml;

		initTracks(mml);
    //    __enable_irq();
		//interrupts();
    MmlMusic::continuePlaying();
}

void MmlMusic::play(const char* mml)
{
	_fUseFlash=false;
	playMML(mml);
}

void MmlMusic::play_P(const char* mml)
{
	_fUseFlash=true;
	playMML(mml);
}

void MmlMusic::stop()
{
    //    __disable_irq();
		//noInterrupts();
//    _isPlaying = false;
    for(uint8_t nTrack=0; nTrack<_nNumTracks; nTrack++)
    	tracks[nTrack]._isPlaying=false;
		//interrupts();
    //    __enable_irq();
}

bool MmlMusic::isPlaying()
{
  for(uint8_t nTrack=0; nTrack<_nNumTracks; nTrack++)
  	if(tracks[nTrack]._isPlaying)	return(true);
	return(false);
}


void MmlMusic::continuePlaying()
{	// play the next event for the active tracks
//	tracks[0]._isPlaying=false;
	for(uint8_t nTrack=0; nTrack<_nNumTracks; nTrack++)
	{
		if(tracks[nTrack]._isPlaying && millis()>= tracks[nTrack]._ulNextTimeMS)
			tracks[nTrack].executeCommandTrack(this);
	}
}

void MmlMusicTrack::executeCommandTrack(MmlMusic *pMusic)
{	// play the next command of the specified track
    //    _scheduler.detach();
    if (_pause < 0)
        _pause = 0;
    if (_pause != 0) {
				pMusic->noTone(_nTrack);
				pMusic->waitTone(_pause*1000, _nTrack);
        _pause = 0;
    } else {
        int freqIndex = -1;
        do {
            skipWhiteSpace();
            switch (getChar()) {
            case 'a':
                freqIndex = NOTE_A;
                break;
            case 'b':
                freqIndex = NOTE_B;
                break;
            case 'c':
                freqIndex = NOTE_C;
                break;
            case 'd':
                freqIndex = NOTE_D;
                break;
            case 'e':
                freqIndex = NOTE_E;
                break;
            case 'f':
                freqIndex = NOTE_F;
                break;
            case 'g':
                freqIndex = NOTE_G;
                break;

            case 'p':
            case 'r':
                freqIndex = NOTE_REST;
                break;

            case 'l':
                if (isdigit(peekChar()))
                    _duration = (float)WHOLE_NOTE_DURATION / getNumber(1, 64);
                break;
            case 'o':
                if (isdigit(peekChar()))
                    _octave = getNumber(0, 7);
                break;
            case 't':
                if (isdigit(peekChar()))
                    _tempo = getNumber(32, 255);
                break;
            case 'v':
                if (isdigit(peekChar()))
                    _volume = getNumber(0, 127);		// was: 128 
                break;
            case 'm':
                switch (getChar()) {
                case 'n':
                    _durationRatio = DEFAULT_TIMING;
                    break;
                case 'l':
                    _durationRatio = LEGATO_TIMING;
                    break;
                case 's':
                    _durationRatio = STACCATO_TIMING;
                    break;
                }
                break;

            case 'n':
                if (isdigit(peekChar()))
                    freqIndex = getNumber(0, 96);
                break;
            case '<':
                --_octave;
                if (_octave < 0)
                    _octave = 0;
                break;
            case '>':
                ++_octave;
                if (_octave > 7)
                    _octave = 7;
                break;

            case '\0':
            case ',':		// track ends where new track starts
                _isPlaying = false;
                break;
            }	// switch (getChar())
            if (!_isPlaying) {
                //_pwm.period_ms(1);
                //_pwm.write(0.0);
								pMusic->noTone(_nTrack);
                if (pMusic->_completionCallback)
                    pMusic->_completionCallback();
                return;
            }

            float durationRatio = _durationRatio;
            float duration = _duration;

            if (freqIndex != -1) {
                bool fCminus = false; // MMOLE: ugly fix for nitwits that use C- in their music
                switch (getChar()) {
                case '+':
                case '#':
                    ++freqIndex;
                    break;
                case '-':
                    --freqIndex;
                    if (freqIndex == 0)
                        fCminus = true;
                    break;
                case '.':
                    durationRatio = 3.0f / 2.0f;
                    while (peekChar() == '.') {
                        durationRatio *= 3.0f / 2.0f;
                        getChar();
                    }
                    break;
                default:
                    rewind();
                    break;
                }

                if (isdigit(peekChar())) {
                    duration = WHOLE_NOTE_DURATION / getNumber(1, 64);
                }

#if defined (ARDUINO_ARCH_ESP8266)
// ESP8266 cores 2.5.1 and 2.5.2 have an alignment issue causing incorrect reading of floats in PROGMEM
// By default the 2.5.1 and 2.5.2 cores use pgm_read_float_unaligned() but then the PROGMEM floats are not properly read.
// See reported issue https://github.com/esp8266/Arduino/issues/6590 and fix in newer version https://github.com/esp8266/Arduino/pull/6593
// See also https://github.com/esp8266/Arduino/issues/5628 and https://github.com/esp8266/Arduino/pull/5692
#if defined(pgm_read_float_aligned)
	#define pgm_read_float(addr)            pgm_read_float_aligned(addr)
#endif
#endif
// TODO: making the note array uint16_t may make the library faster and smaller (better for ATtiny's)

                if (freqIndex != NOTE_REST || (fCminus && _octave > 0)) {
                    float ftFreq = pgm_read_float(&FREQ_TABLE[freqIndex + (_octave * 12)]);
                    if (fCminus)
                        ftFreq = pgm_read_float(&FREQ_TABLE[NOTE_B + ((_octave - 1) * 12)]);
                        // MMOLE: ugly fix: play C- as B in lower octave
										//pMusic->tone((unsigned int)ftFreq);		// start playing the new tone
										pMusic->tone((unsigned int)ftFreq, 0, _nTrack, _volume);		// start playing the new tone
                }
                else if(freqIndex == NOTE_REST)
										pMusic->tone(0, 0, _nTrack, _volume);		// stop playing the current tone
                duration *= (QUARTER_NOTES_PER_MINUTE / _tempo);
                _pause=duration * durationRatio;
								pMusic->waitTone(_pause*1000, _nTrack);		// schedule to wait until tone is done
                _pause = duration * (1 - durationRatio);
            } // if (freqIndex != -1) 
        } while (freqIndex == -1);
    }
}

int MmlMusicTrack::getNumber(int min, int max)
{
    char ch;
    int value = 0;
		// fixed issue #1: reading beyond end after number due to missing rewind
    do
    {
    		ch = getChar();
        if (!isdigit(ch)) {
            rewind();	// no digit, also rewind for \0
            break;
        }
        int digit = (int)ch - 48;
        value = (value * 10) + digit;
    } while (true);
    value = value < min ? min : value > max ? max : value;
    return value;
}

void MmlMusicTrack::skipWhiteSpace()
{
    while (isspace(peekChar()))
        getChar();
}

char MmlMusicTrack::getChar()
{
	if(_fUseFlash)
    return tolower((char)pgm_read_byte(_mml+(_mmlIndex++)));
  else
    return tolower(_mml[_mmlIndex++]);
}

char MmlMusicTrack::peekChar()
{
	if(_fUseFlash)
    return tolower((char)pgm_read_byte(_mml+_mmlIndex));
  else
    return tolower(_mml[_mmlIndex]);
}

void MmlMusicTrack::rewind()
{
    --_mmlIndex;
}

void MmlMusicTrack::init(uint8_t nTrack, const char *mml, bool fUseFlash)
{
	_fUseFlash=fUseFlash;
	_nTrack=nTrack;
  _mml=mml;
  _isPlaying=true;				// set track active
  _mmlIndex = 0;
  _octave = 4;
  _duration = QUARTER_NOTE_DURATION;
  _durationRatio = DEFAULT_TIMING;
  _tempo = 120;
  _volume = 127;		// TODO: 128 is max according original code, but 127 is easier to map to 0xF
  _pause = 0;

	_ulNextTimeMS=millis();
}

// strchr_P seems not implemented for ESP8266
#if defined (ARDUINO_ARCH_ESP8266)
const char *strchr_P(const char *pstr, char c)
{
	char cFound;
	char *p=(char *)pstr;
	while(cFound=(char)pgm_read_byte(p))
	{
		if(cFound==c)
			 return(p);
		p++;
	}
	return(NULL);
}
#endif

	
void MmlMusic::initTracks(const char *mml)
{	// check for multiple tracks and initialize them
	_nNumTracks=1;
	tracks[0].init(0, mml, _fUseFlash);
  this->noTone(0);
  char *pTrack=(char *) mml;
  while(char *pFound = (_fUseFlash? ((char *)strchr_P(pTrack, ',')): strchr(pTrack, ',')))
  {
		pTrack=pFound+1;		// set pointer to just after track separating comma
		tracks[_nNumTracks].init(_nNumTracks, pTrack, _fUseFlash);
	  this->noTone(_nNumTracks);
		if(++_nNumTracks >= MMLMUSIC_MAX_TRACKS)
			break;
  }
}


// clang-format off
const float MmlMusicTrack::FREQ_TABLE[] PROGMEM = {
    0,
    //1       2         3         4         5         6         7         8         9         10        11        12
    //C       C#        D         D#        E         F         F#        G         G#        A         A#        B
    16.35f,   17.32f,   18.35f,   19.45f,   20.60f,   21.83f,   23.12f,   24.50f,   25.96f,   27.50f,   29.14f,   30.87f,   // Octave 0
    32.70f,   34.65f,   36.71f,   38.89f,   41.20f,   43.65f,   46.25f,   49.00f,   51.91f,   55.00f,   58.27f,   61.74f,   // Octave 1
    65.41f,   69.30f,   73.42f,   77.78f,   82.41f,   87.31f,   92.50f,   98.00f,   103.83f,  110.00f,  116.54f,  123.47f,  // Octave 2
    130.81f,  138.59f,  146.83f,  155.56f,  164.81f,  174.62f,  185.00f,  196.00f,  207.65f,  220.00f,  233.08f,  246.94f,  // Octave 3
    261.63f,  277.18f,  293.67f,  311.13f,  329.63f,  349.23f,  370.00f,  392.00f,  415.31f,  440.00f,  466.17f,  493.89f,  // Octave 4
    523.25f,  554.37f,  587.33f,  622.26f,  659.26f,  698.46f,  739.99f,  783.99f,  830.61f,  880.00f,  932.33f,  987.77f,  // Octave 5
    1046.51f, 1108.74f, 1174.67f, 1244.51f, 1318.52f, 1396.92f, 1479.99f, 1567.99f, 1661.23f, 1760.01f, 1864.66f, 1975.54f, // Octave 6
    2093.02f, 2217.47f, 2349.33f, 2489.03f, 2637.03f, 2793.84f, 2959.97f, 3135.98f, 3322.45f, 3520.02f, 3729.33f, 3951.09f, // Octave 7
};
// clang-format on
