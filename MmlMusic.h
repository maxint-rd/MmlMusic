#ifndef __MMLMUSIC_H__
#define __MMLMUSIC_H__
#include <Arduino.h>

#define OPT_MMLMUSIC_PLAYCALLBACK 1			// set to 1 to use PLAYCALLBACK on start of play

class MmlMusic; 

class MmlMusicTrack
{
public:
    MmlMusicTrack(void) :_isPlaying(false) {}; // Constructor

    void init(uint8_t nTrack, const char *mml, bool fUseFlash);
		void executeCommandTrack(MmlMusic *pMusic);

    int getNumber(int min, int max);
    void skipWhiteSpace();
    char getChar();
    char peekChar();
    void rewind();

    bool _isPlaying;
    const char* _mml;
    int _mmlIndex;
    int _octave;						// 0-7
    float _duration;				// 1-64
    float _durationRatio;
    float _pause;
    int _tempo;							// 32-255
    uint8_t _volume;				// 0-127 (was 128 in original code, 1-8-15 according https://wiki.mabinogiworld.com/view/MML)
    uint8_t _nTrack;				// 0-MAX_TRACKS
    unsigned long _ulNextTimeMS;
		bool _fUseFlash;

public:
    static const float WHOLE_NOTE_DURATION;
    static const float QUARTER_NOTE_DURATION;
    static const float QUARTER_NOTES_PER_MINUTE;

    static const float DEFAULT_TIMING;
    static const float LEGATO_TIMING;
    static const float STACCATO_TIMING;

    static const uint8_t NOTE_REST;
    static const uint8_t NOTE_C;
    static const uint8_t NOTE_CS;
    static const uint8_t NOTE_D;
    static const uint8_t NOTE_DS;
    static const uint8_t NOTE_E;
    static const uint8_t NOTE_F;
    static const uint8_t NOTE_FS;
    static const uint8_t NOTE_G;
    static const uint8_t NOTE_GS;
    static const uint8_t NOTE_A;
    static const uint8_t NOTE_AS;
    static const uint8_t NOTE_B;
    
    static const float FREQ_TABLE[];
};

#define MMLMUSIC_MAX_TRACKS 4

class MmlMusic {
public:
    /** Creates an instance of the MmlMusic
      * @param pin pin used to generate the note frequencies
    */
    MmlMusic(void); // Constructor

    /** Support for Arduino tone() replacements, needed when reusing Timer2 on ATmega
     */
    void tone(unsigned int frequency, unsigned long length=0, uint8_t nTrack=0, uint8_t nVolume=127);
	  void noTone(uint8_t nTrack=0);
	  void waitTone(unsigned long length=0, uint8_t nTrack=0);


    /** Starts playing a new MML sequence. If one is already playing it is stopped and the new
      * sequences started.
      * @param mml string of MML commands to be played
     */
    void play(const char* mml);
    void play_P(const char* mml);

    /** Stop a currently playing sequence */
    void stop();

    /** Query the engine to determine if a MML sequence is currently being played. */
    bool isPlaying();

#if(OPT_MMLMUSIC_PLAYCALLBACK)
    /** Setup a callback function that will be executed when the music sequence starts. */
    void setPlayCallback(void (*function)(const char* mml, bool fUseFlash))
    {
        _playCallback = function;
    }
#endif
    /** Setup a callback function that will be executed when a tone is played. */
    void setToneCallback(bool (*function)(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume))
    {
        _toneCallback = function;
    }
    /** Setup a callback function that will be executed when the music sequence ends. */
    void setCompletionCallback(void (*function)(void))
    {
        _completionCallback = function;
    }
    void continuePlaying();		// public to allow access from callback

protected:
		virtual void playTone(unsigned int frequency, unsigned long length, uint8_t nTrack=0, uint8_t nVolume=127);		// use =0 for pure virtual method which must be implemented by child

private:
    void playMML(const char* mml);
    void initTracks(const char *mml);
		bool callToneCallback(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume);
    void playToneWithCB(unsigned int frequency, unsigned long length, uint8_t nTrack=0, uint8_t nVolume=127);		// default: nTrack=0
		
protected:
		bool _fUseFlash;
		MmlMusicTrack tracks[MMLMUSIC_MAX_TRACKS];
		uint8_t _nNumTracks;
		
//    const char* _mml;

public:
#if(OPT_MMLMUSIC_PLAYCALLBACK)
    void (*_playCallback)(const char* mml, bool fUseFlash);
#endif
    bool (*_toneCallback)(unsigned int frequency, unsigned long length, uint8_t nTrack, uint8_t nVolume);		// default: nTrack=0, nVolume=127
    void (*_completionCallback)(void);

};
#endif //__MMLMUSIC_H__
