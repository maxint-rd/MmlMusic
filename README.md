# MML Music
Arduino library to play multi-track MML music using different sound devices.

### Introduction
This library is a follow-up of the [ESP-MusicEngine library](https://github.com/maxint-rd/ESP-MusicEngine). It provides a device independant base to implement playing MML music on different MCUs using various devices.

### Device independant base class
Note that the library requires additional code via callback functions or an inherited child class to implement the actual production of sound on a particular device. The SN76489_MmlMusic example demonstrates using a callback function to produce multiple voice music on the SN76489 complex sound generator using the [mxUnifiedSN76489 library](https://github.com/maxint-rd/mxUnifiedSN76489). The [MmlMusicPWM](https://github.com/maxint-rd/MmlMusicPWM) library implements a child class using which you can play single voice music through a piezo speaker connected to an output pin.

### Installation/Usage
The current version can be downloaded as an Arduino library using the Sketch|Library menu. Just add the zipfile library and the enclosed examples should appear in the menu automatically. 

Initialisation outside of Setup():
```
  // include header and initialize class
  #include <MmlMusic.h>
  MmlMusic music();
```

Then to play music, call the play method where you want:
```
music.play("T240 L16 O6 C D E F G");
```

To continue playing the rest of the sequence, regular calls to continuePlaying() must be made, either in a timer interrupt or as part of a loop.
```
music.continuePlaying();
```

See the included examples for more detailed instructions on how to use this library.

### Supported MML Syntax
Short syntax overview:<br>

Command | Description
------------ | -------------
&nbsp;  Tnnn | Set tempo [32-255]. Examples: T120, T240<br>
&nbsp;  Vnnn | Set volume [0-128]. Note: limited effect on PWM-volume. Examples: V1, T120<br>
&nbsp;  Lnn  | Set default note length [1-64]. Examples: L8, L16<br>
&nbsp;  Mx   | Set timing. Mn=default, Ml=legato, Ms=staccato<br>
&nbsp;  On   | Set octave [0-7]. Examples: O6, O7<br>
&nbsp;  A-G  | Play whole note. Example: C<br>
&nbsp;  Ann-Gnn  | Play note of alternative length [1-64]. Example: C4, A16<br>
&nbsp;  Nnn  | Play frequency [0-96]. Example: N48<br>
&nbsp;  #    | Play sharp note. Example: C#<br>
&nbsp;  &plus;    | Alternative for #<br>
&nbsp;  &minus;   | Play flat note. Example: D-<br>
&nbsp;  R    | Rest. Example:  CDEC r CDEC<br>
&nbsp;  P    | Pause. Alternative for R. Example:  CDEC p CDEC<br>
&nbsp;  .    | Longer note. Example: CDEC.&nbsp;<br>
&nbsp;  &gt; | shift octave up.  Example: CDE&gt;CDE.&nbsp;<br>
&nbsp;  &lt; | shift octave down.  Example: CDE&lt;CDE.&nbsp;<br>
&nbsp;  , | play multiple tracks  Example: CDE&lt;CDE.,EDC&lt;ECD.&nbsp;<br>

The supported MML-commands are a subset that may not completely cover all available music scores.
If notes seem missing, check your score against the syntax above and replace unknown commands by equivalent supported alternatives. The music notation is case-insensitive. Spaces are not required but can be used for readability.

### Features & limitations
- This library was tested in the Arduino IDE v1.6.10 and v1.8.2. The current version of this library supports ESP8266, Atmel ATmega328 and ATmega168 MCUs. Support for ATtiny85 was also added, but since the ATtiny85 has limited resources, available memory limits it usage to simple applications.
- This version currently supports multi-channel playback, but performance is depending on the capabilities of the output device. Unless specified explicitely, each track uses default settings for tempo, volume, length and octave. Some downloaded multiple track MML music assumes subsequent tracks to use the settings of the first track, but the current implementation doesn't support such behavior.
- Track synchronization is not implemented (yet) and the synchronization bar | is not supported.

### Credits
This library is based on the MusicEngine library ported from mBed to Arduino. It is a follow-up of the [ESP-MusicEngine library](https://github.com/maxint-rd/ESP-MusicEngine).

MusicEngine class / Retro  Music Engine<br>
Original author: Chris Taylor (taylorza). Open source license: Apache 2.0<br>
see https://developer.mbed.org/users/taylorza/code/MusicEngine/<br>
Ported from mBed to Arduino by MMOLE (maxint-rd), inherited Apache license.

### Links
- Learn more about Music Macro Language (MML) on wikipedia:<br>
   http://en.wikipedia.org/wiki/Music_Macro_Language<br>
- For downloadable MML music see http://www.archeagemmllibrary.com/<br>
- Extensive MML reference guide (not all commands supported):<br>
   http://woolyss.com/chipmusic/chipmusic-mml/ppmck_guide.php<br>
- Info about using PWM and other methods to generate sound:<br>
   https://developer.mbed.org/users/4180_1/notebook/using-a-speaker-for-audio-output/

### Disclaimer
- All code on this GitHub account, including this library is provided to you on an as-is basis without guarantees and with all liability dismissed. It may be used at your own risk. Unfortunately I have no means to provide support.
