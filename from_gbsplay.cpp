/*
This file contains the code that converts gbsplay's output to gb_chip_state objects.
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

bool gbsplayStdout2songData(std::vector<gb_chip_state>& songData, std::string gbsFileName, int subsongNum, const uint32_t QUANTIZER, int timeInSeconds){
auto start = std::chrono::high_resolution_clock::now();

#ifdef WIN32
std::string progPrefix = ".\\";
std::string progSuffix = ".exe";
#else
std::string progPrefix = "./";
std::string progSuffix = "";
#endif
std::string gbsplayCmd = progPrefix+"gbsplay"+progSuffix+" -t "+ std::to_string(timeInSeconds) +" -o iodumper -- "+gbsFileName+" "+std::to_string(subsongNum)+" "+std::to_string(subsongNum);
printf("DEBUG: going to call popen(%s)\n", gbsplayCmd.c_str());
FILE *gbsplayFile = popen(gbsplayCmd.c_str(), "r"); // https://stackoverflow.com/questions/125828/capturing-stdout-from-a-system-command-optimally
char line[1024];
uint64_t cyclesPassed=0;
uint64_t framesPassed=0;
gb_chip_state curState;
for (int i=0; i<2; i++){ // skip 2 lines
	fgets(line, sizeof(line), gbsplayFile);
}
while (fgets(line, sizeof(line), gbsplayFile)){ 
	// read cycleDiff, registerIndex, and registerValue from each line.
	uint32_t cycleDiff = std::stoul(std::string(line).substr(0, 8), nullptr, 16); // https://stackoverflow.com/questions/1070497/c-convert-hex-string-to-signed-integer
	uint16_t registerIndex = std::stoul(std::string(line).substr(9, 4), nullptr, 16);
	uint8_t registerValue = std::stoul(std::string(line).substr(14, 2), nullptr, 16);
	
	uint8_t waveIndex;
	
	
	switch (registerIndex){
		case 0xff10: // square 1
			curState.gb_square1_state.sweep_speed = std::make_pair((registerValue >> 4) & 7, true);
			curState.gb_square1_state.sweep_up_or_down = std::make_pair((registerValue >> 3) & 1, true);
			curState.gb_square1_state.sweep_shift = std::make_pair(registerValue & 7, true);
			break;
		case 0xff11:
			curState.gb_square1_state.duty_cycle = std::make_pair((registerValue >> 6) & 3, true);
			curState.gb_square1_state.sound_length = std::make_pair(registerValue & 0x3F, true);
			break;
		case 0xff12:
			curState.gb_square1_state.env_start_vol = std::make_pair((registerValue >> 4) & 0xF, true);
			curState.gb_square1_state.env_down_or_up = std::make_pair((registerValue >> 3) & 1, true);
			curState.gb_square1_state.env_length = std::make_pair(registerValue & 7, true);
			break;
		case 0xff13:
			//printf("0xff13 write 0x%x on frame %lu\n", registerValue, framesPassed);
			curState.gb_square1_state.pitchLSB.first = registerValue;
			curState.gb_square1_state.pitchLSB.second = true;
			break;
		case 0xff14:
			//printf("0xff13 write 0x%x on frame %lu, pitch MSB: 0x%x\n", registerValue, framesPassed, ((registerValue & 7)<<8));
			curState.gb_square1_state.pitchMSB.first = registerValue & 7;
			curState.gb_square1_state.pitchMSB.second = true;
			curState.gb_square1_state.trigger = (registerValue >> 7) & 1;
			curState.gb_square1_state.sound_length_enable = std::make_pair((registerValue >> 6) & 1, true);
			break;
		case 0xff16: // square 2
			curState.gb_square2_state.duty_cycle = std::make_pair((registerValue >> 6) & 3, true);
			curState.gb_square2_state.sound_length = std::make_pair(registerValue & 0x3F, true);
			break;
		case 0xff17:
			//printf("sq2 env value: %x\n", registerValue);
			curState.gb_square2_state.env_start_vol = std::make_pair((registerValue >> 4) & 0xF, true); // good
			curState.gb_square2_state.env_down_or_up = std::make_pair((registerValue >> 3) & 1, true);
			curState.gb_square2_state.env_length = std::make_pair(registerValue & 7, true);
			break;
		case 0xff18:
			curState.gb_square2_state.pitchLSB.first = registerValue;
			curState.gb_square2_state.pitchLSB.second = true;
			break;
		case 0xff19:
			curState.gb_square2_state.pitchMSB.first = registerValue & 7;
			curState.gb_square2_state.pitchMSB.second = true;
			curState.gb_square2_state.trigger = (registerValue >> 7) & 1;
			curState.gb_square2_state.sound_length_enable = std::make_pair((registerValue >> 6) & 1, true);
			break;
		case 0xff1B: // wave
			curState.gb_wave_state.sound_length = std::make_pair(registerValue, true);
			break;
		case 0xff1C:
			curState.gb_wave_state.volume = std::make_pair((registerValue & 0x60) >> 5, true);
			break;
		case 0xff1D:
			curState.gb_wave_state.pitchLSB.first = registerValue;
			curState.gb_wave_state.pitchLSB.second = true;
			break;
		case 0xff1E:
			curState.gb_wave_state.pitchMSB.first = registerValue & 7;
			curState.gb_wave_state.pitchMSB.second = true;
			curState.gb_wave_state.trigger = (registerValue >> 7) & 1;
			curState.gb_wave_state.sound_length_enable = std::make_pair((registerValue >> 6) & 1, true);
			break;
		case 0xff20: // noise
			curState.gb_noise_state.sound_length = std::make_pair(registerValue & 0x3F, true);
			break;
		case 0xff21:
			curState.gb_noise_state.env_start_vol = std::make_pair((registerValue >> 4) & 0xF, true);
			curState.gb_noise_state.env_down_or_up = std::make_pair((registerValue >> 3) & 1, true);
			curState.gb_noise_state.env_length = std::make_pair(registerValue & 7, true);
			break;
		case 0xff22:
			curState.gb_noise_state.noise_long_or_short = std::make_pair((registerValue & 8) >> 3, true);
			curState.gb_noise_state.noise_pitch = std::make_pair(registerValue & 0xF7, true);
			break;
		case 0xff23:
			curState.gb_noise_state.trigger = (registerValue >> 7) & 1;
			curState.gb_noise_state.sound_length_enable = std::make_pair((registerValue >> 6) & 1, true);
			break;
		case 0xff25: // control
			//printf("panning changed: %x\n", registerValue);
			curState.gb_square1_state.panning = std::make_pair(((registerValue & 16) >> 3) | (registerValue & 1), true);
			curState.gb_square2_state.panning = std::make_pair(((registerValue & 32) >> 4) | ((registerValue & 2) >> 1), true);
			curState.gb_wave_state.panning = std::make_pair(((registerValue & 64) >> 5) | ((registerValue & 4) >> 2), true);
			curState.gb_noise_state.panning = std::make_pair(((registerValue & 0x80) >> 6) | ((registerValue & 8) >> 3), true);
			//printf("curState.gb_square1_state.panning: %u, %u\n", curState.gb_square1_state.panning.first, curState.gb_square1_state.panning.second);
			//printf("curState.gb_square2_state.panning: %u, %u\n", curState.gb_square2_state.panning.first, curState.gb_square2_state.panning.second);
			//printf("curState.gb_wave_state.panning: %u, %u\n", curState.gb_wave_state.panning.first, curState.gb_wave_state.panning.second);
			//printf("curState.gb_noise_state.panning: %u, %u\n", curState.gb_noise_state.panning.first, curState.gb_noise_state.panning.second);
			break;
		case 0xff30: // wave table
		case 0xff31:
		case 0xff32:
		case 0xff33:
		case 0xff34:
		case 0xff35:
		case 0xff36:
		case 0xff37:
		case 0xff38:
		case 0xff39:
		case 0xff3A:
		case 0xff3B:
		case 0xff3C:
		case 0xff3D:
		case 0xff3E:
		case 0xff3F:
			waveIndex = (uint8_t)((registerIndex - 0xff30)*2);
			curState.gb_wave_state.wavetable.first[waveIndex] = (registerValue & 0xF0) >> 4;
			curState.gb_wave_state.wavetable.first[waveIndex+1] = registerValue & 0xF;
			curState.gb_wave_state.wavetable.second=true;
			break;
		default:
			break;
	}
	
	// add the value of cycleDiff to cyclesPassed on each line. Check if cyclesPassed is evenly divisible by CYCLES_PER_FRAME; if it is, increase framesPassed.
	cyclesPassed += cycleDiff;
	uint64_t curFramesPassed = cyclesPassed / QUANTIZER;
	if (curFramesPassed > framesPassed) {
		uint64_t frameDifference = curFramesPassed - framesPassed;
		//if (frameDifference>1) printf("frameDifference: %lu\n", frameDifference);
		for (int i=0; i<frameDifference-1; i++) {
			songData.push_back(gb_chip_state{});
		}
		framesPassed = curFramesPassed;
		songData.push_back(curState);
		curState = gb_chip_state{};
	}
	
	// for each loop, gradually build a frameChipState. Once a frame has passed, the current frameChipState is completed and pushed to songData.
	
	//printf("cycleDiff: 0x%08x, registerIndex: 0x%04x, registerValue: 0x%02x\n", cycleDiff, registerIndex, registerValue);
}
/*
for (struct gb_chip_state i: songData){
	printf("gb_square1_state.pitch %u\n", i.gb_square1_state.pitch);
}
*/

// replace undefined values with valid values from previous frames.
for (int i=1; i<songData.size(); i++){
	if (songData[i].gb_square1_state.sweep_speed.second==false && songData[i-1].gb_square1_state.sweep_speed.second==true) {
		songData[i].gb_square1_state.sweep_speed=songData[i-1].gb_square1_state.sweep_speed;
		songData[i].gb_square1_state.sweep_up_or_down=songData[i-1].gb_square1_state.sweep_up_or_down;
		songData[i].gb_square1_state.sweep_shift=songData[i-1].gb_square1_state.sweep_shift;
	}
	if (songData[i].gb_square1_state.duty_cycle.second==false && songData[i-1].gb_square1_state.duty_cycle.second==true) {
		songData[i].gb_square1_state.duty_cycle=songData[i-1].gb_square1_state.duty_cycle;
		songData[i].gb_square1_state.sound_length=songData[i-1].gb_square1_state.sound_length;
	}
	if (songData[i].gb_square1_state.env_start_vol.second==false && songData[i-1].gb_square1_state.env_start_vol.second==true) {
		songData[i].gb_square1_state.env_start_vol=songData[i-1].gb_square1_state.env_start_vol;
		songData[i].gb_square1_state.env_down_or_up=songData[i-1].gb_square1_state.env_down_or_up;
		songData[i].gb_square1_state.env_length=songData[i-1].gb_square1_state.env_length;
	}
	if (songData[i].gb_square1_state.pitchLSB.second==false && songData[i-1].gb_square1_state.pitchLSB.second==true) {
		songData[i].gb_square1_state.pitchLSB=songData[i-1].gb_square1_state.pitchLSB;
	}
	if (songData[i].gb_square1_state.sound_length_enable.second==false && songData[i-1].gb_square1_state.sound_length_enable.second==true) {
		songData[i].gb_square1_state.sound_length_enable=songData[i-1].gb_square1_state.sound_length_enable;
		songData[i].gb_square1_state.pitchMSB=songData[i-1].gb_square1_state.pitchMSB;
	}
	if (songData[i].gb_square1_state.panning.second==false && songData[i-1].gb_square1_state.panning.second==true) {
		songData[i].gb_square1_state.panning=songData[i-1].gb_square1_state.panning;
	}
	
	if (songData[i].gb_square2_state.duty_cycle.second==false && songData[i-1].gb_square2_state.duty_cycle.second==true) {
		songData[i].gb_square2_state.duty_cycle=songData[i-1].gb_square2_state.duty_cycle;
		songData[i].gb_square2_state.sound_length=songData[i-1].gb_square2_state.sound_length;
	}
	if (songData[i].gb_square2_state.env_start_vol.second==false && songData[i-1].gb_square2_state.env_start_vol.second==true) {
		songData[i].gb_square2_state.env_start_vol=songData[i-1].gb_square2_state.env_start_vol;
		songData[i].gb_square2_state.env_down_or_up=songData[i-1].gb_square2_state.env_down_or_up;
		songData[i].gb_square2_state.env_length=songData[i-1].gb_square2_state.env_length;
	}
	if (songData[i].gb_square2_state.pitchLSB.second==false && songData[i-1].gb_square2_state.pitchLSB.second==true) {
		songData[i].gb_square2_state.pitchLSB=songData[i-1].gb_square2_state.pitchLSB;
	}
	if (songData[i].gb_square2_state.sound_length_enable.second==false && songData[i-1].gb_square2_state.sound_length_enable.second==true) {
		songData[i].gb_square2_state.sound_length_enable=songData[i-1].gb_square2_state.sound_length_enable;
		songData[i].gb_square2_state.pitchMSB=songData[i-1].gb_square2_state.pitchMSB;
	}
	if (songData[i].gb_square2_state.panning.second==false && songData[i-1].gb_square2_state.panning.second==true) {
		songData[i].gb_square2_state.panning=songData[i-1].gb_square2_state.panning;
	}
	
	if (songData[i].gb_wave_state.sound_length.second==false && songData[i-1].gb_wave_state.sound_length.second==true) {
		songData[i].gb_wave_state.sound_length=songData[i-1].gb_wave_state.sound_length;
	}
	if (songData[i].gb_wave_state.volume.second==false && songData[i-1].gb_wave_state.volume.second==true) {
		songData[i].gb_wave_state.volume=songData[i-1].gb_wave_state.volume;
	}
	if (songData[i].gb_wave_state.pitchLSB.second==false && songData[i-1].gb_wave_state.pitchLSB.second==true) {
		songData[i].gb_wave_state.pitchLSB=songData[i-1].gb_wave_state.pitchLSB;
	}
	if (songData[i].gb_wave_state.sound_length_enable.second==false && songData[i-1].gb_wave_state.sound_length_enable.second==true) {
		songData[i].gb_wave_state.sound_length_enable=songData[i-1].gb_wave_state.sound_length_enable;
		songData[i].gb_wave_state.pitchMSB=songData[i-1].gb_wave_state.pitchMSB;
	}
	if (songData[i].gb_wave_state.panning.second==false && songData[i-1].gb_wave_state.panning.second==true) {
		songData[i].gb_wave_state.panning=songData[i-1].gb_wave_state.panning;
	}
	if (songData[i].gb_wave_state.wavetable.second==false && songData[i-1].gb_wave_state.wavetable.second==true) {
		songData[i].gb_wave_state.wavetable=songData[i-1].gb_wave_state.wavetable;
	}
	
	if (songData[i].gb_noise_state.sound_length.second==false && songData[i-1].gb_noise_state.sound_length.second==true) {
		songData[i].gb_noise_state.sound_length=songData[i-1].gb_noise_state.sound_length;
	}
	if (songData[i].gb_noise_state.env_start_vol.second==false && songData[i-1].gb_noise_state.env_start_vol.second==true) {
		songData[i].gb_noise_state.env_start_vol=songData[i-1].gb_noise_state.env_start_vol;
		songData[i].gb_noise_state.env_down_or_up=songData[i-1].gb_noise_state.env_down_or_up;
		songData[i].gb_noise_state.env_length=songData[i-1].gb_noise_state.env_length;
	}
	if (songData[i].gb_noise_state.noise_long_or_short.second==false && songData[i-1].gb_noise_state.noise_long_or_short.second==true) {
		songData[i].gb_noise_state.noise_long_or_short=songData[i-1].gb_noise_state.noise_long_or_short;
		songData[i].gb_noise_state.noise_pitch=songData[i-1].gb_noise_state.noise_pitch;
	}
	if (songData[i].gb_noise_state.sound_length_enable.second==false && songData[i-1].gb_noise_state.sound_length_enable.second==true) {
		songData[i].gb_noise_state.sound_length_enable=songData[i-1].gb_noise_state.sound_length_enable;
	}
	if (songData[i].gb_noise_state.panning.second==false && songData[i-1].gb_noise_state.panning.second==true) {
		songData[i].gb_noise_state.panning=songData[i-1].gb_noise_state.panning;
	}
}

auto stop = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
printf("gbsplayStdout2songData: %ld milliseconds.\n", duration.count());
return true;
}