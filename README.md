# gbs2fur
Convert GBS (Game Boy music file) to fur (Furnace Tracker module).

Please see [this video](https://www.youtube.com/watch?v=BZKgEGXditk) for a demonstration of what gbs2fur is capable of    
(Notice: the exact command-line parameters for gbs2fur have changed since this video was uploaded. To view the current command-line parameters, run gbs2fur on its own without any command-line arguments)

The gbsplay executable must be in the same folder as the gbs2fur executable for the program to work

## Known Issues
The following are known issues that may be impossible to fix in the furnace module mode.
- Noise pitch sounds a bit off.      
There seems to be no way to fix this.     
**Explanation**: The way gb noise works in furnace tracker is that each note in furnace corresponds to a specific gb noise pitch. However, furnace doesn't assign a note to every possible value of gb noise; so when converting from gb noise -> furnace note, you're limited to the list of gb noise pitches that furnace supports. If there is a way to make furnace play any arbitrary noise pitch, I would like to use it in this converter program.
- Certain staccato notes are too quiet. This is most noticeable with the Kirby Dream Land 2 soundtrack.    
The user can add back staccato to the fur file manually.     
**Explanation**: It is possible to change the volume of a channel *mid-frame* and have it immediately take effect. I experimented with quantizing the song to subframes to account for this, but even with FRAME_DIVIDER set to 32, it still wasn't perfect (and you run out of space far too quickly). I tried changing it around so that only the FIRST write to any given value in any given frame is preserved, but it sounded awful.

## Unfinished Midi Mode
gbs2fur can also convert gbs to midi by setting the output file extension to ".mid".     
This feature is unfinished, and there is currently no way to play back the midi file in a way that sounds accurate to the Game Boy.    
In the future, I plan to create an LV2 plugin to play back these midi files. I do not have any experience with creating DAW plugins, so any help would be greatly appreciated.

Once the midi mode is ready, it should be more accurate than the furnace module mode, because it should be able to solve the above known issues. Please view this [video demonstration of the midi mode](https://www.youtube.com/watch?v=c9pDOH-yaII), which showcases how the midi mode can accurately recreate fast volume and pulse width (a.k.a. duty cycle) changes.

Output midi files do not follow any tempo. To work with these files, I recommend that you open the midi files in a DAW and set the midi files to ignore the DAW's tempo.    
Here is how to do that in Reaper:   
1. Leave your Reaper project at the default BPM of 120
2. Open the midi file in Reaper
3. Select all midi items (e.g. by shift-clicking), open the right-click menu, Item settings > Set item timebase to time
4. With all the midi items still selected: right-click > Source properties > OK > Ignore project tempo, use 120 BPM
5. Now you can freely change the Reaper project's tempo, and add tempo change events, without affecting the speed of the midi. Set the project's tempo to a value that musically matches the midi file.

## Credits
- This program uses [gbsplay](https://github.com/mmitch/gbsplay) to convert GBS files to a list of sound chip register writes, which my program then converts to a fur file.
- [libsmf from sseq2mid](https://github.com/Thysbelon/sseq2mid), originally written by [loveemu](https://github.com/loveemu/loveemu-lab/tree/master/nds/sseq2mid/src).