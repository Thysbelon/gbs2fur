/*
KNOWN ISSUES:
- Noise pitch sounds a bit off, and I don't know if it's possible to fix this. The way gb noise works in furnace tracker is that each note in furnace corresponds to a specific gb noise pitch. However, furnace doesn't assign a note to every possible value of gb noise; so when converting from gb noise -> furnace note, you're limited to the list of gb noise pitches that furnace supports. If there is a way to make furnace play any arbitrary noise pitch, I would like to use it in this converter program.
- DMG-AKBJ-JPN.gbs subsong 0. staccato notes are too quiet. I have learned that it is possible to change env_start_vol *mid-frame* and have it immediately take effect. I experimented with quantizing the song to subframes to account for this, but even with FRAME_DIVIDER set to 32, it still wasn't perfect (and you run out of space far too quickly). This may be unfixable. I tried changing it around so that only the FIRST write to any given value in any given frame is preserved, but it sounded awful.
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
#include <chrono> // for measuring performance

#include "gb_chip_state.hpp"
#include "from_gbsplay.hpp"
#include "to_fur.hpp"
#include "to_midi.hpp"

//const uint8_t FRAME_DIVIDER = 1; // usually 1
//const uint32_t CYCLES_PER_FRAME = 65536 /*cycles per volume envelope updating*/;
//const uint32_t CYCLES_PER_FRAME = 70224; // cycles per video frame
//const uint32_t CYCLES_PER_FRAME = 8192; // cycles per DIV-APU tick (512 Hz) https://gbdev.io/pandocs/Audio_details.html#div-apu
//const uint32_t CYCLES_PER_FRAME = 1000; // This does a good job of capturing the staccatos in DMG-AKBJ-JPN. Still completely impractical for music production in a tracker though. Maybe I can use this amount of quantization when I write a midi converter. CYCLES_PER_FRAME = 8192 and FRAME_DIVIDER = 8 also accomplishes the same thing.
const uint32_t MASTER_CLOCK = 0x400000; // game boy cycles per second. 4194304
// MASTER_CLOCK / (CYCLES_PER_FRAME / FRAME_DIVIDER) = number of quantized frames in a second

int main(int argc, char *const argv[]){
	
if (argc<4) {
	printf("./gbs2fur file.gbs subsongNumber outfile.fur [timeInSeconds] [patternLength] [enablePanMute] \n");
	return 1;
}

uint16_t patternLength = 64;
bool disablePanMute=true;
// the song "Big Forest" from Kirby's Dream Land 2 functions strangely. When played via gbsplay, during the intro, square 1 is muted, and this seems to happen because square 1's panning is set to 0 0. However, emulators and real hardware will play square 1: https://www.youtube.com/watch?v=e2_Ly1cBMR4
if (argc>=6) {patternLength = atoi(argv[5]);}
if (argc>=7) {
	if (std::string(argv[6]) == "enablePanMute") disablePanMute=false;
}
	
//const uint32_t CYCLES_PER_FRAME = 70224; // cycles per video frame
//const uint32_t CYCLES_PER_FRAME = 70224 / FRAME_DIVIDER;
//const uint32_t CYCLES_PER_FRAME = 65536 /*cycles per volume envelope updating*/ / FRAME_DIVIDER;


// songData will be quantized to frames
std::vector<gb_chip_state> songData;

std::string outfilename = std::string(argv[3]);

if (outfilename.substr(outfilename.length()-4, 4) == ".fur") {
	uint8_t FRAME_DIVIDER = 1;
	uint32_t CYCLES_PER_FRAME = 70224;
	const uint32_t QUANTIZER = CYCLES_PER_FRAME / FRAME_DIVIDER;
	gbsplayStdout2songData(songData, std::string(argv[1]), atoi(argv[2]), QUANTIZER, argc >= 5 ? atoi(argv[4]) : 150);
	float inGBframesPerSecond = (float)(((double)MASTER_CLOCK/(double)CYCLES_PER_FRAME)*(double)FRAME_DIVIDER);
	// converting songData to a fur file is a separate function
	songData2fur(songData, outfilename, patternLength, disablePanMute, inGBframesPerSecond);
} else if (outfilename.substr(outfilename.length()-4, 4) == ".mid") {
	uint8_t FRAME_DIVIDER = 8;
	uint32_t CYCLES_PER_FRAME = 8192;
	const uint32_t QUANTIZER = CYCLES_PER_FRAME / FRAME_DIVIDER;
	gbsplayStdout2songData(songData, std::string(argv[1]), atoi(argv[2]), QUANTIZER, argc >= 5 ? atoi(argv[4]) : 150);
	float inGBframesPerSecond = (float)(((double)MASTER_CLOCK/(double)CYCLES_PER_FRAME)*(double)FRAME_DIVIDER);
	songData2midi(songData, inGBframesPerSecond, outfilename);
} else {
	printf("Valid output file extensions are .fur and .mid\n");
}

return 0;
}