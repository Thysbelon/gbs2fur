/*
This file contains the code that converts songData to a midi file.

NOTE:
I think this is finished for now.
TODO: write the game boy LV2 plugin to go with this. Use LMMS's freeboy as a reference.
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
#include "libsmfc.h"
#include "libsmfcx.h"

#include "to_midi.hpp"

static int closest(std::vector<uint16_t> const& vec, int value) { // https://stackoverflow.com/questions/8647635/elegant-way-to-find-closest-value-in-a-vector-from-above
	auto const it = std::lower_bound(vec.begin(), vec.end(), value);
	if (it == vec.end()) { return -1; }
	return *it;
}
static std::pair<int, int> gbPitch2noteAndPitch(uint16_t gbPitch){ // https://www.devrs.com/gb/files/sndtab.html
	//uint32_t frequency = 131072/(2048-gbPitch);
	// gbPitch = (-131072 / freq) + 2048
	int note;
	int pitchAdjust;
	std::vector<uint16_t> gbPitchArray = {44,156,262,363,457,547,631,710,786,854,923,986,1046,1102,1155,1205,1253,1297,1339,1379,1417,1452,1486,1517,1546,1575,1602,1627,1650,1673,1694,1714,1732,1750,1767,1783,1798,1812,1825,1837,1849,1860,1871,1881,1890,1899,1907,1915,1923,1930,1936,1943,1949,1954,1959,1964,1969,1974,1978,1982,1985,1988,1992,1995,1998,2001,2004,2006,2009,2011,2013,2015}; // length: 72
	uint8_t noteC2=36; // midi note number
	uint16_t closestGbpitch = closest(gbPitchArray, gbPitch);
	uint8_t gbPitchArrayIndex=std::distance( gbPitchArray.begin(), std::find(gbPitchArray.begin(), gbPitchArray.end(), closestGbpitch) );
	note = noteC2 + gbPitchArrayIndex;
	int pitchDifference = (int)gbPitch - closestGbpitch;
	
	
	if (pitchDifference > 0) {
		if (gbPitchArrayIndex+1 >= gbPitchArray.size()) {
			pitchAdjust = 0;
		} else {
			uint16_t totalSemitoneDiff = gbPitchArray[gbPitchArrayIndex+1] - gbPitchArray[gbPitchArrayIndex];
			pitchAdjust = 0 + (float)0x1000 * ((float)pitchDifference / totalSemitoneDiff);
		}
	} else if (pitchDifference < 0) {
		//printf("negative pitchDifference: %d\n", pitchDifference);
		if (gbPitchArrayIndex <= 0) {
			pitchAdjust = 0;
		} else {
			uint16_t totalSemitoneDiff = gbPitchArray[gbPitchArrayIndex] - gbPitchArray[gbPitchArrayIndex-1];
			int pitchAdjustAlter = (float)0x1000 * ((float)std::abs(pitchDifference) / totalSemitoneDiff);
			if (pitchAdjustAlter > 0x1000 / 2) { // TODO: make this checks possible to disable on the command line.
				note--;
				pitchAdjust = 0x1000 - pitchAdjustAlter;
			} else {
				pitchAdjust = 0 - pitchAdjustAlter;
			}
			//printf("totalSemitoneDiff: %u, pitchAdjust: %d\n", totalSemitoneDiff, pitchAdjust);
		}
	} else {
		pitchAdjust = 0;
	}
	
	return std::make_pair(note, pitchAdjust);
}
static uint8_t noisePitch2note(uint8_t noisePitch, std::vector<uint8_t> const& NOISE_PITCH_LIST){
	uint8_t note = std::distance(NOISE_PITCH_LIST.begin(), std::find(NOISE_PITCH_LIST.begin(), NOISE_PITCH_LIST.end(), noisePitch));
	return note;
}
template <typename T>
static void internal1insertGBnoteInMidi(gb_chip_state::base_chan_class* chanState, T& prevPitch, Smf* midiFile, uint64_t const& midiTicksPassed, const uint64_t& midiTicksPerSoundLenTick, uint8_t chanIndex, std::vector<uint8_t> const& NOISE_PITCH_LIST){
	smfInsertNoteOff(midiFile, midiTicksPassed, chanIndex, chanIndex, chanIndex == 3 ? noisePitch2note(prevPitch, NOISE_PITCH_LIST) : gbPitch2noteAndPitch(prevPitch).first, 0x7F); // end previous note. NOTE: this line currently does not check if the previous note already had a noteOff. I'll see if there's any problems with inserting multiple note offs for the same note.
	// insert note
	
	// check if note is being triggered just to set volume to 0. TODO: make the If logic in insertGBnoteInMidi better?
	if (chanIndex != 2) {
		auto tempPointer = dynamic_cast<gb_chip_state::channels_with_env*>(chanState);
		if (chanState->trigger==1 && tempPointer->env_start_vol.first==0 && tempPointer->env_start_vol.second && tempPointer->env_length.first==0 && tempPointer->env_length.second) return;
	}
	
	int note=0;
	if (chanIndex!=3) {
		std::pair<int, int> noteAndPitchAdjust = gbPitch2noteAndPitch(dynamic_cast<gb_chip_state::melodic_channels*>(chanState)->getPitch());
		smfInsertPitchBend(midiFile, midiTicksPassed, chanIndex, chanIndex, noteAndPitchAdjust.second);
		note = noteAndPitchAdjust.first;
	} else {
		note = noisePitch2note(dynamic_cast<gb_chip_state::noise*>(chanState)->noise_pitch.first, NOISE_PITCH_LIST);
	}
	smfInsertNoteOn(midiFile, midiTicksPassed, chanIndex, chanIndex, note, 0x7F);
	if (chanState->sound_length.first && chanState->sound_length.second && chanState->sound_length.second && chanState->trigger==1) {
		smfInsertNoteOff(midiFile, midiTicksPassed + ((chanIndex == 2 ? 256 : 64) - chanState->sound_length.first) * midiTicksPerSoundLenTick, 0, 0, note, 0x7F);
	}
}
template <typename T>
static void insertGBnoteInMidi(gb_chip_state::base_chan_class* chanState, T& prevPitch, Smf* midiFile, uint64_t const& midiTicksPassed, bool& chanLegato, const uint64_t& midiTicksPerSoundLenTick, uint8_t chanIndex /*0 and 1 is square, 2 is wave, 3 is noise*/, std::vector<uint8_t> const& NOISE_PITCH_LIST){
	T curRegisterPitch=0;
	bool isPitchValid=false;
	if (chanIndex == 3){
		auto tempPointer = dynamic_cast<gb_chip_state::noise*>(chanState);
		curRegisterPitch = tempPointer->noise_pitch.first;
		isPitchValid = tempPointer->noise_pitch.second;
	} else {
		auto tempPointer = dynamic_cast<gb_chip_state::melodic_channels*>(chanState);
		//curRegisterPitch = dynamic_cast<gb_chip_state::melodic_channels*>(static_cast<gb_chip_state::base_chan_class*>(chanState))->getPitch();
		curRegisterPitch = tempPointer->getPitch();
		isPitchValid = tempPointer->pitchLSB.second && tempPointer->pitchMSB.second;
	}
	if (curRegisterPitch != prevPitch && isPitchValid) {
		//printf("sq1 pitch is different and valid. chanState->pitch.first: %u, prevPitch: %u\n", chanState->pitch.first, prevPitch);
		/*
		if (chanIndex==0) {
			printf("curRegisterPitch: %u, chanIndex: %u\n", curRegisterPitch, chanIndex);
			//printf("((gb_chip_state::square_1*)chanState)->getPitch(): %u, chanIndex: %u\n", ((gb_chip_state::square_1*)chanState)->getPitch(), chanIndex);
		}
		*/
		if (chanState->trigger==0 && chanLegato==false) {
			//printf("pitch is being changed without triggering note & legato is false.\n");
			smfInsertControl(midiFile, midiTicksPassed, chanIndex, chanIndex, 68, 0x7F);
			chanLegato=true;
		} else if (chanState->trigger==1 && chanLegato==true) {
			//printf("note is being triggered & legato is true.\n");
			smfInsertControl(midiFile, midiTicksPassed, chanIndex, chanIndex, 68, 0);
			chanLegato=false;
		}
		internal1insertGBnoteInMidi(chanState, prevPitch, midiFile, midiTicksPassed, midiTicksPerSoundLenTick, chanIndex, NOISE_PITCH_LIST);
		prevPitch=curRegisterPitch;
		//printf("prevPitch==curRegisterPitch: %u\n", prevPitch==curRegisterPitch);
	} else { // ?
		//printf("sq1 pitch is the same and/or invalid. chanState->pitch.first: %u, prevPitch: %u\n", chanState->pitch.first, prevPitch);
		if (chanState->trigger==1) {
			//printf("note %u is being triggered again\n", chanState->pitch.first);
			if (chanLegato==true){
				smfInsertControl(midiFile, midiTicksPassed, chanIndex, chanIndex, 68, 0);
				chanLegato=false;
			}
			internal1insertGBnoteInMidi(chanState, prevPitch, midiFile, midiTicksPassed, midiTicksPerSoundLenTick, chanIndex, NOISE_PITCH_LIST);
		}
	}
}
bool songData2midi(std::vector<gb_chip_state>& songData, float inGBframesPerSecond, std::string outfilename){
	auto start = std::chrono::high_resolution_clock::now();
	
	std::vector<uint8_t> NOISE_PITCH_LIST;
	for (uint8_t i=0; i<0xF7; i++){
		if ((i & 8) == 0) {
			NOISE_PITCH_LIST.push_back(i);
		}
	}
	
	const int SECONDS_IN_A_MINUTE=60;
	const int MIDI_BPM=240;
	const int MIDI_PPQN=960;
	Smf* midiFile = smfCreate();
	smfSetTimebase(midiFile, MIDI_PPQN); // timebase should be high to make adjusting the song easy.
	smfInsertTempoBPM(midiFile, 0, 0, MIDI_BPM);
	// MASTER_CLOCK / (CYCLES_PER_FRAME / FRAME_DIVIDER) = number of quantized gb frames in a second
	// tempo of midi file will always be 120 bpm
	// 960 is the number of ticks in each quarter note. tempo is 120 bpm, 120 / 60 = 2 beats/quarter notes a second. PPQN * (bpm / SECONDS_IN_A_MINUTE) = ticks per second. 960 * 2 = 1920 ticks per second.
	// (midi ticks per second) / (number of quantized gb frames in a second) = number of midi ticks per gb frame
	const uint64_t midiTicksPerSecond = (float)MIDI_PPQN * ((float)MIDI_BPM / SECONDS_IN_A_MINUTE);
	printf("midiTicksPerSecond: %lu\n", midiTicksPerSecond);
	printf("inGBframesPerSecond: %f\n", inGBframesPerSecond);
	const uint64_t midiTicksPerGBframe = round((float)midiTicksPerSecond / inGBframesPerSecond);
	printf("midiTicksPerGBframe: %lu\n", midiTicksPerGBframe);
	const uint64_t midiTicksPerSoundLenTick = round((float)midiTicksPerSecond / 256);
	
	/*
	std::set<std::array<uint8_t,32>> uniqueWavetables;
	
	// add wavetables to fur
	for (std::array<uint8_t,32> curWavetable : uniqueWavetables) {
		furFileClass::wavetableClass curFurWave;
		for (int i=0; i<32; i++){
			curFurWave.wavetableData.push_back(curWavetable[i]);
		}
		curFurWave.size = curFurWave.calculateSize();
		furFile.wavetables.push_back(curFurWave);
	}
	*/
		
	uint8_t prevWavetableIndex=0xFF;
	uint8_t prevWaveVol=0x0F;
	uint16_t prevSQ1pitch=0xFFFF;
	uint16_t prevSQ2pitch=0xFFFF;
	uint16_t prevWavPitch=0xFFFF;
	uint8_t prevNoiPitch=0xFF;
	uint8_t prevSweepSpeed=0;
	uint8_t prevSweepShift=0;
	uint8_t prevSweepDirection=0;
	uint8_t prevSQ1duty=0;
	uint8_t prevSQ2duty=0;
	uint8_t prevSQ1pan=0xFF;
	uint8_t prevSQ2pan=0xFF;
	uint8_t prevWavPan=0xFF;
	uint8_t prevNoiPan=0xFF;
	uint8_t prevNoiseLen=0;
	bool SQ1legato=false;
	bool SQ2legato=false;
	bool WavLegato=false;
	bool NoiLegato=false;
	uint64_t midiTicksPassed=0;
	//uint16_t patIndex=0;
	//uint32_t prevPatsSize=0;
	for (int stateIndex=0; stateIndex<songData.size(); stateIndex++){
		gb_chip_state& curState = songData[stateIndex];
		
		//std::tuple<envAndSoundLen, envAndSoundLen, envAndSoundLen> curInsSettings = gbChipState2envAndSoundLen(curState);
		//printf("curInsSettings (sq2): %u, %u, %u, %u, %u\n", std::get<1>(curInsSettings).sound_length, std::get<1>(curInsSettings).env_start_vol, std::get<1>(curInsSettings).env_down_or_up, std::get<1>(curInsSettings).env_length, std::get<1>(curInsSettings).sound_length_enable);
		
		//if (curState.gb_wave_state.wavetable.second) {
		//	uint8_t wavetableIndex = std::distance(std::begin(uniqueWavetables), uniqueWavetables.find(curState.gb_wave_state.wavetable.first));
		//	if (wavetableIndex != prevWavetableIndex) {
		//		/*insert wave change into pattern*/
		//		writeVar(curWavPatRow.effects[2],0x10);
		//		writeVar(curWavPatRow.effectVal[2],wavetableIndex);
		//		prevWavetableIndex = wavetableIndex;
		//	}
		//}
		
		// panning
		if (curState.gb_square1_state.panning.second) {
			uint8_t curSQ1pan=64;
			switch (curState.gb_square1_state.panning.first) {
				case 0:
					curSQ1pan=0xFF;
					break;
				case 0b10:
					curSQ1pan=0;
					break;
				case 0b01:
					curSQ1pan=127;
					break;
				case 0b11:
					curSQ1pan=64;
					break;
			}
			//printf("curSQ1pan: 0x%X, curState.gb_square1_state.panning.first: %u\n", curSQ1pan, curState.gb_square1_state.panning.first);
			if(curSQ1pan!=prevSQ1pan){
				//printf("sq1 panning changed\n");
				if (curSQ1pan==0xFF){
					smfInsertControl(midiFile, midiTicksPassed, 0, 0, 9, 0x7F);
				} else if (curSQ1pan!=0xFF && prevSQ1pan==0xFF) {
					smfInsertControl(midiFile, midiTicksPassed, 0, 0, 9, 0);
					smfInsertControl(midiFile, midiTicksPassed, 0, 0, SMF_CONTROL_PANPOT, curSQ1pan);
				} else { // curSQ1pan!=0xFF
					smfInsertControl(midiFile, midiTicksPassed, 0, 0, SMF_CONTROL_PANPOT, curSQ1pan);
				}
				prevSQ1pan=curSQ1pan;
			}
		}
		if (curState.gb_square2_state.panning.second) {
			uint8_t curSQ2pan=64;
			switch (curState.gb_square2_state.panning.first) {
				case 0:
					curSQ2pan=0xFF;
					break;
				case 0b10:
					curSQ2pan=0;
					break;
				case 0b01:
					curSQ2pan=127;
					break;
				case 0b11:
					curSQ2pan=64;
					break;
			}
			if(curSQ2pan!=prevSQ2pan){
				if (curSQ2pan==0xFF){
					smfInsertControl(midiFile, midiTicksPassed, 1, 1, 9, 0x7F);
				} else if (curSQ2pan!=0xFF && prevSQ2pan==0xFF) {
					smfInsertControl(midiFile, midiTicksPassed, 1, 1, 9, 0);
					smfInsertControl(midiFile, midiTicksPassed, 1, 1, SMF_CONTROL_PANPOT, curSQ2pan);
				} else {
					smfInsertControl(midiFile, midiTicksPassed, 1, 1, SMF_CONTROL_PANPOT, curSQ2pan);
				}
				prevSQ2pan=curSQ2pan;
			}
		}
		if (curState.gb_wave_state.panning.second) {
			uint8_t curWavPan=64;
			switch (curState.gb_wave_state.panning.first) {
				case 0:
					curWavPan=0xFF;
					break;
				case 0b10:
					curWavPan=0;
					break;
				case 0b01:
					curWavPan=127;
					break;
				case 0b11:
					curWavPan=64;
					break;
			}
			if(curWavPan!=prevWavPan){
				if (curWavPan==0xFF){
					smfInsertControl(midiFile, midiTicksPassed, 2, 2, 9, 0x7F);
				} else if (curWavPan!=0xFF && prevWavPan==0xFF) {
					smfInsertControl(midiFile, midiTicksPassed, 2, 2, 9, 0);
					smfInsertControl(midiFile, midiTicksPassed, 2, 2, SMF_CONTROL_PANPOT, curWavPan);
				} else {
					smfInsertControl(midiFile, midiTicksPassed, 2, 2, SMF_CONTROL_PANPOT, curWavPan);
				}
				prevWavPan=curWavPan;
			}
		}
		if (curState.gb_noise_state.panning.second) {
			uint8_t curNoiPan=64;
			switch (curState.gb_noise_state.panning.first) {
				case 0:
					curNoiPan=0xFF;
					break;
				case 0b10:
					curNoiPan=0;
					break;
				case 0b01:
					curNoiPan=127;
					break;
				case 0b11:
					curNoiPan=64;
					break;
			}
			if(curNoiPan!=prevNoiPan){
				if (curNoiPan==0xFF){
					smfInsertControl(midiFile, midiTicksPassed, 3, 3, 9, 0x7F);
				} else if (curNoiPan!=0xFF && prevNoiPan==0xFF) {
					smfInsertControl(midiFile, midiTicksPassed, 3, 3, 9, 0);
					smfInsertControl(midiFile, midiTicksPassed, 3, 3, SMF_CONTROL_PANPOT, curNoiPan);
				} else {
					smfInsertControl(midiFile, midiTicksPassed, 3, 3, SMF_CONTROL_PANPOT, curNoiPan);
				}
				prevNoiPan=curNoiPan;
			}
		}
		
		// envelope starting volume
		if (curState.gb_square1_state.env_start_vol.second){
			if (stateIndex==0 || curState.gb_square1_state.env_start_vol.first != songData[stateIndex-1].gb_square1_state.env_start_vol.first) {
				uint8_t curEnvVol = 0x7F * ((float)curState.gb_square1_state.env_start_vol.first / 0x0F);
				smfInsertControl(midiFile, midiTicksPassed, 0, 0, SMF_CONTROL_VOLUME, curEnvVol);
			}
		}
		if (curState.gb_square2_state.env_start_vol.second){
			if (stateIndex==0 || curState.gb_square2_state.env_start_vol.first != songData[stateIndex-1].gb_square2_state.env_start_vol.first) {
				uint8_t curEnvVol = 0x7F * ((float)curState.gb_square2_state.env_start_vol.first / 0x0F);
				smfInsertControl(midiFile, midiTicksPassed, 1, 1, SMF_CONTROL_VOLUME, curEnvVol);
			}
		}
		if (curState.gb_noise_state.env_start_vol.second){
			if (stateIndex==0 || curState.gb_noise_state.env_start_vol.first != songData[stateIndex-1].gb_noise_state.env_start_vol.first) {
				uint8_t curEnvVol = 0x7F * ((float)curState.gb_noise_state.env_start_vol.first / 0x0F);
				smfInsertControl(midiFile, midiTicksPassed, 3, 3, SMF_CONTROL_VOLUME, curEnvVol);
			}
		}
		
		// envelope direction
		if (curState.gb_square1_state.env_down_or_up.second){
			if (stateIndex==0 || curState.gb_square1_state.env_down_or_up.first != songData[stateIndex-1].gb_square1_state.env_down_or_up.first) {
				smfInsertControl(midiFile, midiTicksPassed, 0, 0, 12, curState.gb_square1_state.env_down_or_up.first == 1 ? 0x7F : 0);
			}
		}
		if (curState.gb_square2_state.env_down_or_up.second){
			if (stateIndex==0 || curState.gb_square2_state.env_down_or_up.first != songData[stateIndex-1].gb_square2_state.env_down_or_up.first) {
				smfInsertControl(midiFile, midiTicksPassed, 1, 1, 12, curState.gb_square2_state.env_down_or_up.first == 1 ? 0x7F : 0);
			}
		}
		if (curState.gb_noise_state.env_down_or_up.second){
			if (stateIndex==0 || curState.gb_noise_state.env_down_or_up.first != songData[stateIndex-1].gb_noise_state.env_down_or_up.first) {
				smfInsertControl(midiFile, midiTicksPassed, 3, 3, 12, curState.gb_noise_state.env_down_or_up.first == 1 ? 0x7F : 0);
			}
		}
		
		// envelope length
		if (curState.gb_square1_state.env_length.second){
			if (stateIndex==0 || curState.gb_square1_state.env_length.first != songData[stateIndex-1].gb_square1_state.env_length.first) {
				smfInsertControl(midiFile, midiTicksPassed, 0, 0, 13, round((float)127 * ((float)curState.gb_square1_state.env_length.first / 7)));
			}
		}
		if (curState.gb_square2_state.env_length.second){
			if (stateIndex==0 || curState.gb_square2_state.env_length.first != songData[stateIndex-1].gb_square2_state.env_length.first) {
				smfInsertControl(midiFile, midiTicksPassed, 1, 1, 13, round((float)127 * ((float)curState.gb_square2_state.env_length.first / 7)));
			}
		}
		if (curState.gb_noise_state.env_length.second){
			if (stateIndex==0 || curState.gb_noise_state.env_length.first != songData[stateIndex-1].gb_noise_state.env_length.first) {
				smfInsertControl(midiFile, midiTicksPassed, 3, 3, 13, round((float)127 * ((float)curState.gb_noise_state.env_length.first / 7)));
			}
		}
		
		// wave volume
		if (curState.gb_wave_state.volume.second) {
			uint8_t curWaveVol=0;
			switch(curState.gb_wave_state.volume.first){
				case 0:
					curWaveVol=0;
					//smfInsertNoteOff(midiFile, midiTicksPassed, 2, 2, gbPitch2noteAndPitch(prevWavPitch).first, 0x7F);
					break;
				case 1:
					curWaveVol=127;
					break;
				case 2:
					curWaveVol=64;
					break;
				case 3:
					curWaveVol=32;
					break;
			}
			if (curWaveVol!=prevWaveVol){
				smfInsertControl(midiFile, midiTicksPassed, 2, 2, SMF_CONTROL_VOLUME, curWaveVol);
				prevWaveVol=curWaveVol;
			}
		}
		
		// note/pitch. trigger
		//printf("%u, %u, %u\n", curState.gb_square2_state.pitch, prevSQ2pitch, curState.gb_square2_state.pitch == prevSQ2pitch);
		//printf("%u, %u, %u\n", curState.gb_noise_state.noise_pitch, prevNoiPitch, curState.gb_noise_state.noise_pitch == prevNoiPitch);
		//if (curState.gb_square1_state.getPitch() != prevSQ1pitch && curState.gb_square1_state.pitchLSB.second && curState.gb_square1_state.pitchLSB.second) printf("curState.gb_square1_state.getPitch(): %u\n", curState.gb_square1_state.getPitch());
		insertGBnoteInMidi<uint16_t>(&(curState.gb_square1_state), prevSQ1pitch, midiFile, midiTicksPassed, SQ1legato, midiTicksPerSoundLenTick, 0, NOISE_PITCH_LIST);
		//printf("prevSQ1pitch==curState.gb_square1_state.getPitch(): %u\n", prevSQ1pitch==curState.gb_square1_state.getPitch());
		insertGBnoteInMidi<uint16_t>(&(curState.gb_square2_state), prevSQ2pitch, midiFile, midiTicksPassed, SQ2legato, midiTicksPerSoundLenTick, 1, NOISE_PITCH_LIST);
		insertGBnoteInMidi<uint16_t>(&(curState.gb_wave_state), prevWavPitch, midiFile, midiTicksPassed, WavLegato, midiTicksPerSoundLenTick, 2, NOISE_PITCH_LIST);
		insertGBnoteInMidi<uint8_t>(&(curState.gb_noise_state), prevNoiPitch, midiFile, midiTicksPassed, NoiLegato, midiTicksPerSoundLenTick, 3, NOISE_PITCH_LIST);
		
		// sound length enable
		if (curState.gb_square1_state.sound_length_enable.second){
			if (stateIndex==0 || curState.gb_square1_state.sound_length_enable.first != songData[stateIndex-1].gb_square1_state.sound_length_enable.first) {
				smfInsertControl(midiFile, midiTicksPassed, 0, 0, 14, curState.gb_square1_state.sound_length_enable.first == 1 ? 0x7F : 0);
			}
		}
		if (curState.gb_square2_state.sound_length_enable.second){
			if (stateIndex==0 || curState.gb_square2_state.sound_length_enable.first != songData[stateIndex-1].gb_square2_state.sound_length_enable.first) {
				smfInsertControl(midiFile, midiTicksPassed, 1, 1, 14, curState.gb_square2_state.sound_length_enable.first == 1 ? 0x7F : 0);
			}
		}
		if (curState.gb_noise_state.sound_length_enable.second){
			if (stateIndex==0 || curState.gb_noise_state.sound_length_enable.first != songData[stateIndex-1].gb_noise_state.sound_length_enable.first) {
				smfInsertControl(midiFile, midiTicksPassed, 3, 3, 14, curState.gb_noise_state.sound_length_enable.first == 1 ? 0x7F : 0);
			}
		}
		
		// sound length 
		if (curState.gb_square1_state.sound_length.second){
			if (stateIndex==0 || curState.gb_square1_state.sound_length.first != songData[stateIndex-1].gb_square1_state.sound_length.first) {
				smfInsertControl(midiFile, midiTicksPassed, 0, 0, 15, (float)127 * ((float)curState.gb_square1_state.sound_length.first / 63));
			}
		}
		if (curState.gb_square2_state.sound_length.second){
			if (stateIndex==0 || curState.gb_square2_state.sound_length.first != songData[stateIndex-1].gb_square2_state.sound_length.first) {
				smfInsertControl(midiFile, midiTicksPassed, 1, 1, 15, (float)127 * ((float)curState.gb_square2_state.sound_length.first / 63));
			}
		}
		if (curState.gb_noise_state.sound_length.second){
			if (stateIndex==0 || curState.gb_noise_state.sound_length.first != songData[stateIndex-1].gb_noise_state.sound_length.first) {
				smfInsertControl(midiFile, midiTicksPassed, 3, 3, 15, (float)127 * ((float)curState.gb_noise_state.sound_length.first / 63));
			}
		}
		
		// sweep speed
		// sweep shift
		if ((curState.gb_square1_state.sweep_speed.first != prevSweepSpeed && curState.gb_square1_state.sweep_speed.second) || (curState.gb_square1_state.sweep_shift.first != prevSweepShift && curState.gb_square1_state.sweep_shift.second)) {
			smfInsertControl(midiFile, midiTicksPassed, 0, 0, 16, round((float)127 * ((float)curState.gb_square1_state.sweep_speed.first / 7)));
			smfInsertControl(midiFile, midiTicksPassed, 0, 0, 17, round((float)127 * ((float)curState.gb_square1_state.sweep_shift.first / 7)));
			prevSweepSpeed=curState.gb_square1_state.sweep_speed.first;
			prevSweepShift=curState.gb_square1_state.sweep_shift.first;
		}
		
		// sweep up or down
		if (curState.gb_square1_state.sweep_up_or_down.first != prevSweepDirection && curState.gb_square1_state.sweep_up_or_down.second) {
			smfInsertControl(midiFile, midiTicksPassed, 0, 0, 18, curState.gb_square1_state.sweep_up_or_down.first == 1 ? 0x7F : 0);
			prevSweepDirection=curState.gb_square1_state.sweep_up_or_down.first;
		}
		
		// duty cycle
		if(curState.gb_square1_state.duty_cycle.first != prevSQ1duty && curState.gb_square1_state.duty_cycle.second){
			smfInsertControl(midiFile, midiTicksPassed, 0, 0, 19, round((float)127 * ((float)curState.gb_square1_state.duty_cycle.first / 3)));
			prevSQ1duty=curState.gb_square1_state.duty_cycle.first;
		}
		if(curState.gb_square2_state.duty_cycle.first != prevSQ2duty && curState.gb_square2_state.duty_cycle.second){
			smfInsertControl(midiFile, midiTicksPassed, 1, 1, 19, round((float)127 * ((float)curState.gb_square2_state.duty_cycle.first / 3)));
			prevSQ2duty=curState.gb_square2_state.duty_cycle.first;
		}
		
		// noise long or short
		if (curState.gb_noise_state.noise_long_or_short.second) {
			if(curState.gb_noise_state.noise_long_or_short.first!=prevNoiseLen){
				smfInsertControl(midiFile, midiTicksPassed, 3, 3, 20, curState.gb_noise_state.noise_long_or_short.first == 1 ? 0x7F : 0);
				prevNoiseLen=curState.gb_noise_state.noise_long_or_short.first;
			}
		}
		
		midiTicksPassed+=midiTicksPerGBframe;
		//if (stateIndex % 5000 == 0) {printf("midiTicksPassed: %lu\n", midiTicksPassed);}
	}
	smfSetEndTimingOfTrack(midiFile, 0, midiTicksPassed);
	smfSetEndTimingOfTrack(midiFile, 1, midiTicksPassed);
	smfSetEndTimingOfTrack(midiFile, 2, midiTicksPassed);
	smfSetEndTimingOfTrack(midiFile, 3, midiTicksPassed);
	smfWriteFile(midiFile, outfilename.c_str());
	
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	printf("songData2midi: %ld milliseconds.\n", duration.count());
	return true;
}