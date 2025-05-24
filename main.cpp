/*
KNOWN ISSUES:
- Noise pitch sounds a bit off, and I don't know if it's possible to fix this. The way gb noise works in furnace tracker is that each note in furnace corresponds to a specific gb noise pitch. However, furnace doesn't assign a note to every possible value of gb noise; so when converting from gb noise -> furnace note, you're limited to the list of gb noise pitches that furnace supports. If there is a way to make furnace play any arbitrary noise pitch, I would like to use it in this converter program.
- DMG-AKBJ-JPN.gbs subsong 0. staccato notes are too quiet. I have learned that it is possible to change env_start_vol *mid-frame* and have it immediately take effect. I experimented with quantizing the song to subframes to account for this, but even with FRAME_DIVIDER set to 32, it still wasn't perfect (and you run out of space far too quickly). This may be unfixable. I tried changing it around so that only the FIRST write to any given value in any given frame is preserved, but it sounded awful.

TODO: 
- refactor code. gbsplay code, furnace code, and gb_chip_state code should all be separated into their own files. Write more functions to get rid of copy-pasted code.
*/

#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <cmath>
#include <set>
#include <array>
#include <tuple>
#include <cstring>
#include <variant>
#include <algorithm> // std::find
#include <map>

#include "gb_chip_state.hpp"
#include "from_gbsplay.hpp"
#include "to_fur.hpp"

const uint8_t FRAME_DIVIDER = 1; // usually 1
//const uint32_t CYCLES_PER_FRAME = 65536 /*cycles per volume envelope updating*/;
const uint32_t CYCLES_PER_FRAME = 70224; // cycles per video frame
//const uint32_t CYCLES_PER_FRAME = 8192; // cycles per DIV-APU tick (512 Hz) https://gbdev.io/pandocs/Audio_details.html#div-apu
//const uint32_t CYCLES_PER_FRAME = 1000; // This does a good job of capturing the staccatos in DMG-AKBJ-JPN. Still completely impractical for music production in a tracker though. Maybe I can use this amount of quantization when I write a midi converter. CYCLES_PER_FRAME = 8192 and FRAME_DIVIDER = 8 also accomplishes the same thing.
const uint32_t MASTER_CLOCK = 0x400000; // game boy cycles per second

int main(int argc, char *const argv[]){
	
if (argc<3) {
	printf("./gbs2fur file.gbs subsongNumber [patternLength] [timeInSeconds] [disablePanMute] \n");
	return 1;
}

// ./gbs2fur file.gbs subsongNumber [patternLength] [timeInSeconds] [disablePanMute]
uint16_t patternLength = 64;
bool disablePanMute=false;
if (argc>=4) {patternLength = atoi(argv[3]);}
if (argc>=6) {
	if (std::string(argv[5]) == "disablePanMute") disablePanMute=true;
}
	
//const uint32_t CYCLES_PER_FRAME = 70224; // cycles per video frame
//const uint32_t CYCLES_PER_FRAME = 70224 / FRAME_DIVIDER;
//const uint32_t CYCLES_PER_FRAME = 65536 /*cycles per volume envelope updating*/ / FRAME_DIVIDER;
const uint32_t QUANTIZER = CYCLES_PER_FRAME / FRAME_DIVIDER;

// songData will be quantized to frames
std::vector<gb_chip_state> songData;

gbsplayStdout2songData(songData, std::string(argv[1]), atoi(argv[2]), QUANTIZER, argc >= 5 ? atoi(argv[4]) : 150);

// converting songData to a fur file is a separate function
songData2fur(songData, patternLength, disablePanMute, (float)(((double)MASTER_CLOCK/(double)CYCLES_PER_FRAME)*(double)FRAME_DIVIDER) );

return 0;
}