/*
This file contains the code that converts songData to a furnace tracker file.

TODO: 
- Write more functions to get rid of copy-pasted code.
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

#include "to_fur.hpp"

class furFileClass {
public:
	class headerClass{
	public:
		char formatMagic[16]={'-','F','u','r','n','a','c','e',' ','m','o','d','u','l','e','-',};
		uint16_t formatVersion=228; // 228: furnace 0.6.8.1
		// 2 reserved
		uint32_t songInfoPointer=0x20; // this should always be 0x20, because the size of the header is consistent.
		// 8 reserved
		//uint8_t totalReservedBytes=10;
	};
	class songInfoClass{
	public:
		char blockID[4]={'I','N','F','O'};
		uint32_t size;
		uint8_t timebase=0; // TODO: what is this? It's 0 in my test file. 
		uint8_t speed1=1; // 1
		uint8_t speed2=1; // 1
		uint8_t initArpTime=1; // 1
		float ticksPerSecond=59.7;
		//float ticksPerSecond=(float)(((double)MASTER_CLOCK/(double)CYCLES_PER_FRAME)*(double)FRAME_DIVIDER);
		uint16_t patLen=64;
		uint16_t ordersLen=1;
		uint8_t highlightA=4;
		uint8_t highlightB=16;
		uint16_t insCount;
		uint16_t wavetableCount;
		uint16_t sampleCount=0; // 0
		uint32_t patCount=4/*gb channels*/; // total number of patterns in the song, across all channels.
		uint8_t soundChipsList[32]={4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // for this program, this should always be just Game Boy, which has 4 channels.
		// 32 reserved
		// 32 reserved
		// 128 sound chip flag pointers? just put in 128 bytes.
		std::string songName="song name placeholder"; // TODO: get from GBS. For now, I'll use placeholder values.
		std::string songAuthor="song author placeholder";
		float A4Tuning=440; // leave as default, 440
		uint8_t limitSlides=0; //? 0
		uint8_t linearPitch=2; //? 2
		uint8_t loopModality=2; //? 2
		uint8_t properNoiseLayout=1; //? 1
		uint8_t waveDutyIsVol=0; // should probably set this to false. 0
		uint8_t resetMacroOnPorta=0; // leave as default. 0
		uint8_t legacyVolumeSlides=0; // false. 0
		uint8_t compatibleArpeggio=0; // false? 0
		uint8_t noteOffResetsSlides=1; // default 1
		uint8_t targetResetsSlides=1; // default 1
		uint8_t arpInhibitsPortamento=0; // default 0
		uint8_t wackAlgoMacro=0; // default 0
		uint8_t brokenShortcutSlides=0; // default 0
		uint8_t ignoreDuplicateSlides=0; // default 0
		uint8_t stopPortaOnNoteOff=0; // default 0
		uint8_t continuousVibrato=0; // default 0
		uint8_t brokenDACmode=0; // default 0
		uint8_t oneTickCut=0; // ? 0
		uint8_t canInsChangeDuringPorta=1; // default 1
		uint8_t resetNoteBaseOnArpStop=1; // default 1
		std::vector<uint32_t> insPointers; // length stored in insCount
		std::vector<uint32_t> wavtablPointers;
		std::vector<uint32_t> samplePointers;
		std::vector<uint32_t> patPointers;
		std::vector<std::array<uint8_t, 4/*gameboy channels*/>> orders={{0,0,0,0}};
		uint8_t effectColumns[4/*gameboy channels*/]={6,4,4,4}; // number of effect columns in each channel.
		uint8_t channelHideStatus[4]={3,3,3,3}; // These are all 03 when all channels are visible. Accessed in "Channels" window
		uint8_t channelCollapseStatus[4]={0,0,0,0}; // should all be visible. 00 when visible.
		//std::string customChanNames[4]; // Accessed in "Channels" window. All are the single byte 0 when not set 
		//std::string customChanShortNames[4]; // Accessed in "Channels" window. All are the single byte 0 when not set 
		std::string songComment="song comment placeholder";
		float masterVol=1; // 1.0f = 100%. Set to 1.
		// extended compat flags
		uint8_t brokenSpeedSelection=0; // 0
		uint8_t noSlidesOnFirstTick=0; // 0
		uint8_t nextRowResetArpPos=0; // 0
		uint8_t ignoreJumpAtEnd=0; // 0
		uint8_t buggyPortaAfterSlide=0; // 0
		uint8_t newInsAffectsEnv=1; // game boy specific. TODO Look into this. default 1.
		uint8_t extChStateIsShared=1; // default 1
		uint8_t ingnoreDACmodeChangeOutsideChan=0; // 0
		uint8_t E1xyAndE2xyTakePriorityOverSlide00=0; // 0
		uint8_t newSegaPCM=1; // 1
		uint8_t weirdBlockBasedChipPitchSlides=0; // 0
		uint8_t SNdutyMacroAlwaysResetsPhase=0; // 0
		uint8_t pitchMacroIsLinear=1; // 1
		uint8_t pitchSlideSpeedInFullLinearPitchMode=4; // 4
		uint8_t oldOctaveBoundBehavior=0; // 0
		uint8_t disableOPN2DACvolControl=0; // 0
		uint8_t newVolScaling=1; // 1
		uint8_t volMacroStillAppliesAfterEnd=1; // 1
		uint8_t brokenOutVol=0; // 0
		uint8_t E1xyAndE2xyStopOnSameNote=0; // 0
		uint8_t brokenInitPosOfPortaAfterArp=0; // 0
		uint8_t SNperiodsUnder8areTreatedAs1=0; // 0
		uint8_t cutDelayEffectPolicy=2; // 2
		uint8_t m0Bor0DEffectTreatment=0; // 0
		uint8_t autoSysNameDetect=1; // 1
		uint8_t disableSampleMacro=0; // 0
		uint8_t brokenOutVol2=0; // 0
		uint8_t oldArp=0; // 0
		// end of compat flags
		uint16_t virtualTempoNumerator=150; // 0x96 aka 150
		uint16_t virtualTempoDenominator=150; // 0x96
		std::string firstSubsongName;
		std::string firstSubsongComment;
		uint8_t numOfOtherSubsongs=0; // always set to 0
		// 3 reserved
		std::string sysName="Game Boy"; // "Game Boy"
		std::string albumCatGameName;
		// four 0
		float chipVol=1; // 1
		float chipPan=0; // 0
		float chipFrontRear=0; //0
		uint32_t patchbayConnectionCount=0;
		uint8_t automaticPatchbay=1;
		uint8_t moreCompatFlags[8]={0,0,0,0,0,0,0,0};
		uint8_t lengthOfSpeedPattern=1;
		uint8_t speedPattern[16]={1,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
		uint8_t numberOfGrooveListEntries=0;
		uint32_t pointerToInstrumentDirectories;
		uint32_t pointerToWavetableDirectories;
		uint32_t pointerToSampleDirectories;
		
	//private:
	//	uint8_t totalReservedAndExtraBytes=4+3+4+4+128+32+32;
	//public:
		uint32_t calculateSize(){
			return
				//4 + // INFO. These aren't included in size
				//4 + // size
				1 +
				1+
				1+
				1+
				4+ // ticksPerSecond
				2+ //patLen
				2+ //ordersLen
				1+ // highlights
				1+
				2+ //insCount
				2+ //wavetableCount
				2+ //sampleCount
				4+ //patCount
				32+ // soundChipsList
				32+ // reserved
				32+ //reserved
				128+ // soudnChpFlagPointers
				songName.length()+1+
				songAuthor.length()+1+
				4+ //a4tuning
				20+ // various compat flags
				4*insCount+
				4*wavetableCount+
				4*sampleCount+
				4*patCount+
				ordersLen*4+ // 4 here is gb channels
				4+ //effect columns
				4+ // more channel stuff
				4+
				4+
				4+
				songComment.length()+1+
				4+ // masterVol
				28+ // extended compat flags
				2+ //virtualTempoNumerator
				2+ //virtualTempoDenominator
				firstSubsongName.length()+1+
				firstSubsongComment.length()+1+
				1+ // numOfOtherSubsongs
				3+ // reserved
				sysName.length()+1+
				albumCatGameName.length()+1+
				4+ // japanese strings
				4+ //chipVol
				4+ //chipPan
				4+ // chipFrontRear
				4+ // patchbayConnectionCount
				1+ // automaticPatchbay
				8+ // moreCompatFlags
				1+ // lengthOfSpeedPattern
				16+ // speedPattern
				1+ // numberOfGrooveListEntries
				4+ // pointerToInstrumentDirectories
				4+ // pointerToWavetableDirectories
				4 // pointerToSampleDirectories
				;
		}
	};
	class insFeatureGameBoyClass{
	public:
		//uint8_t envParams; // see docs
		// all envParams are part of the same byte
		uint8_t envLen; // bit 5-7
		uint8_t envDirection; // bit 4
		uint8_t envVol; // bit 0-3
		uint8_t soundLen;
		uint8_t flags=0; // I think all should be disabled.
		uint8_t hardwareSeqLen=0; // I'm always going to set this to 0.
	};
	class insFeatureClass{
	public:
		char featureCode[2]; // this is a string that contains 2 letters and no null0 terminator.
		uint16_t length=2+2+1+1+1+1/*hardcoded feature length*/; // TODO: is this the number of bytes that the block takes up, or is it something else?
		std::variant<std::string, insFeatureGameBoyClass> featureData;
	};
	class instrumentClass{
	public:
		char blockID[4]={'I','N','S','2'};
		uint32_t size=2+2+(2+2+1+1+1+1/*hardcoded instrument length for this program*/);
		uint16_t formatVersion=228;
		uint16_t insType=2; // for this program, should always be 2 (gameboy)
		std::vector<insFeatureClass> features;
		//uint32_t calculateSize(){return sizeof(blockID)+sizeof(size)+sizeof(formatVersion)+sizeof(insType)+(2+2+1+1+1+1/*hardcoded feature length for this program*/)}
	};
	class wavetableClass{
	public:
		char blockID[4]={'W','A','V','E'};
		uint32_t size;
		std::string wavetableName;
		uint32_t wavetableWidth=32;
		// 4 reserved;
		uint32_t wavetableHeight=15;
		std::vector<uint32_t> wavetableData;
	private:
		uint8_t totalReservedBytes=4;
	public:
		uint32_t calculateSize(){return wavetableName.length()+1+sizeof(wavetableWidth)+sizeof(wavetableHeight)+wavetableWidth*4+totalReservedBytes;}
	};
	class patRow{ // bools indicate if the variable has been written to or not.
	public:
		std::pair<uint8_t, bool> note = std::make_pair(0xFF, false);
		std::pair<uint8_t, bool> ins = std::make_pair(0, false);
		std::pair<uint8_t, bool> vol = std::make_pair(0, false);
		std::vector<std::pair<uint8_t, bool>> effects = std::vector<std::pair<uint8_t, bool>>(8); // https://stackoverflow.com/questions/11134497/constant-sized-vector . I think std::array would not be able to hold pairs.
		std::vector<std::pair<uint8_t, bool>> effectVal = std::vector<std::pair<uint8_t, bool>>(8);
		uint8_t skipRow=0;
	};
	class patternClass{
	public:
		char blockID[4]={'P','A','T','N'};
		uint32_t size;
		uint8_t subsong=0;
		uint8_t channel;
		uint16_t patIndex;
		std::string patName;
		std::vector<patRow> patData;
		uint32_t calculateSize(){
			uint32_t patSize=patName.length()+1 + 1+1+2;
			for (patRow curPatRow : patData) {
				uint32_t curRowSize=0;
				curRowSize+=1; // firstByte
				if (curPatRow.effects[1].second==true || curPatRow.effects[2].second==true || curPatRow.effects[3].second==true) {
					curRowSize+=1; // secondByte
				}
				if (curPatRow.effects[4].second==true || curPatRow.effects[5].second==true || curPatRow.effects[6].second==true || curPatRow.effects[7].second==true) {
					curRowSize+=1; // thirdByte
				}
				if (curPatRow.note.second==true) curRowSize+=1;
				if (curPatRow.ins.second==true) curRowSize+=1;
				if (curPatRow.vol.second==true) curRowSize+=1;
				for (int effI=0; effI<8; effI++){
					if (curPatRow.effects[effI].second==true) curRowSize+=1;
					if (curPatRow.effectVal[effI].second==true) curRowSize+=1;
				}
				patSize+=curRowSize;
			}
			patSize+=1; // end of data byte
			return patSize;
		}
	};
	headerClass header;
	songInfoClass songInfo;
	uint8_t directoryData[15]={0x41,0x44,0x49,0x52,0x07,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00}; // this repeats three times in the file
	std::vector<class instrumentClass> instruments;
	std::vector<class wavetableClass> wavetables;
	std::vector<class patternClass> patterns;
	void writeNullTermStringToFile(std::string inString, FILE* outfile){
		fprintf(outfile, "%s", inString.c_str()); // silences -Wformat-security
		uint8_t nullTerminator = 0;
		fwrite(&nullTerminator,1,1,outfile);
	}
	void writeFurFile(FILE* outfile, std::string outfilename){
		outfile = fopen(outfilename.c_str(), "wb");
		
		uint8_t emptyBytes[128];
		memset(emptyBytes, 0, 128);
		
		fwrite(header.formatMagic,1,16,outfile);
		fwrite(&(header.formatVersion),2,1,outfile);
		fwrite(emptyBytes,1,2,outfile);
		fwrite(&(header.songInfoPointer),4,1,outfile);
		fwrite(emptyBytes,1,8,outfile);
		
		fwrite(songInfo.blockID,1,4,outfile);
		fwrite(&(songInfo.size),4,1,outfile);
		fwrite(&(songInfo.timebase),1,1,outfile);
		fwrite(&(songInfo.speed1),1,1,outfile);
		fwrite(&(songInfo.speed2),1,1,outfile);
		fwrite(&(songInfo.initArpTime),1,1,outfile);
		fwrite(&(songInfo.ticksPerSecond),4,1,outfile);
		fwrite(&(songInfo.patLen),2,1,outfile);
		fwrite(&(songInfo.ordersLen),2,1,outfile);
		fwrite(&(songInfo.highlightA),1,1,outfile);
		fwrite(&(songInfo.highlightB),1,1,outfile);
		fwrite(&(songInfo.insCount),2,1,outfile);
		fwrite(&(songInfo.wavetableCount),2,1,outfile);
		fwrite(&(songInfo.sampleCount),2,1,outfile);
		fwrite(&(songInfo.patCount),4,1,outfile);
		fwrite(songInfo.soundChipsList,1,32,outfile);
		fwrite(emptyBytes,1,32,outfile);
		fwrite(emptyBytes,1,32,outfile);
		fwrite(emptyBytes,1,128,outfile);
		writeNullTermStringToFile(songInfo.songName, outfile);
		writeNullTermStringToFile(songInfo.songAuthor, outfile);
		fwrite(&(songInfo.A4Tuning),4,1,outfile);
		fwrite(&(songInfo.limitSlides),1,1,outfile);
		fwrite(&(songInfo.linearPitch),1,1,outfile);
		fwrite(&(songInfo.loopModality),1,1,outfile);
		fwrite(&(songInfo.properNoiseLayout),1,1,outfile);
		fwrite(&(songInfo.waveDutyIsVol),1,1,outfile);
		fwrite(&(songInfo.resetMacroOnPorta),1,1,outfile);
		fwrite(&(songInfo.legacyVolumeSlides),1,1,outfile);
		fwrite(&(songInfo.compatibleArpeggio),1,1,outfile);
		fwrite(&(songInfo.noteOffResetsSlides),1,1,outfile);
		fwrite(&(songInfo.targetResetsSlides),1,1,outfile);
		fwrite(&(songInfo.arpInhibitsPortamento),1,1,outfile);
		fwrite(&(songInfo.wackAlgoMacro),1,1,outfile);
		fwrite(&(songInfo.brokenShortcutSlides),1,1,outfile);
		fwrite(&(songInfo.ignoreDuplicateSlides),1,1,outfile);
		fwrite(&(songInfo.stopPortaOnNoteOff),1,1,outfile);
		fwrite(&(songInfo.continuousVibrato),1,1,outfile);
		fwrite(&(songInfo.brokenDACmode),1,1,outfile);
		fwrite(&(songInfo.oneTickCut),1,1,outfile);
		fwrite(&(songInfo.canInsChangeDuringPorta),1,1,outfile);
		fwrite(&(songInfo.resetNoteBaseOnArpStop),1,1,outfile);
		
		for (int i=0; i<songInfo.insCount; i++){
			fwrite(&(songInfo.insPointers[i]),4,1,outfile);
		}
		for (int i=0; i<songInfo.wavetableCount; i++){
			fwrite(&(songInfo.wavtablPointers[i]),4,1,outfile);
		}
		// for this program I won't write samples
		for (int i=0; i<songInfo.patCount; i++){
			fwrite(&(songInfo.patPointers[i]),4,1,outfile);
		}
		for (int i=0; i<4/*gameboy channels*/; i++){
			for (int i2=0; i2<songInfo.ordersLen; i2++){
				fwrite(&(songInfo.orders[i2][i]),1,1,outfile);
			}
		}
		
		fwrite(songInfo.effectColumns,1,4,outfile);
		fwrite(songInfo.channelHideStatus,1,4,outfile);
		fwrite(songInfo.channelCollapseStatus,1,4,outfile);
		fwrite(emptyBytes,1,4,outfile);
		fwrite(emptyBytes,1,4,outfile);
		writeNullTermStringToFile(songInfo.songComment, outfile);
		fwrite(&(songInfo.masterVol),4,1,outfile);
		
		fwrite(&(songInfo.brokenSpeedSelection),1,1,outfile);
		fwrite(&(songInfo.noSlidesOnFirstTick),1,1,outfile);
		fwrite(&(songInfo.nextRowResetArpPos),1,1,outfile);
		fwrite(&(songInfo.ignoreJumpAtEnd),1,1,outfile);
		fwrite(&(songInfo.buggyPortaAfterSlide),1,1,outfile);
		fwrite(&(songInfo.newInsAffectsEnv),1,1,outfile);
		fwrite(&(songInfo.extChStateIsShared),1,1,outfile);
		fwrite(&(songInfo.ingnoreDACmodeChangeOutsideChan),1,1,outfile);
		fwrite(&(songInfo.E1xyAndE2xyTakePriorityOverSlide00),1,1,outfile);
		fwrite(&(songInfo.newSegaPCM),1,1,outfile);
		fwrite(&(songInfo.weirdBlockBasedChipPitchSlides),1,1,outfile);
		fwrite(&(songInfo.SNdutyMacroAlwaysResetsPhase),1,1,outfile);
		fwrite(&(songInfo.pitchMacroIsLinear),1,1,outfile);
		fwrite(&(songInfo.pitchSlideSpeedInFullLinearPitchMode),1,1,outfile);
		fwrite(&(songInfo.oldOctaveBoundBehavior),1,1,outfile);
		fwrite(&(songInfo.disableOPN2DACvolControl),1,1,outfile);
		fwrite(&(songInfo.newVolScaling),1,1,outfile);
		fwrite(&(songInfo.volMacroStillAppliesAfterEnd),1,1,outfile);
		fwrite(&(songInfo.brokenOutVol),1,1,outfile);
		fwrite(&(songInfo.E1xyAndE2xyStopOnSameNote),1,1,outfile);
		fwrite(&(songInfo.brokenInitPosOfPortaAfterArp),1,1,outfile);
		fwrite(&(songInfo.SNperiodsUnder8areTreatedAs1),1,1,outfile);
		fwrite(&(songInfo.cutDelayEffectPolicy),1,1,outfile);
		fwrite(&(songInfo.m0Bor0DEffectTreatment),1,1,outfile);
		fwrite(&(songInfo.autoSysNameDetect),1,1,outfile);
		fwrite(&(songInfo.disableSampleMacro),1,1,outfile);
		fwrite(&(songInfo.brokenOutVol2),1,1,outfile);
		fwrite(&(songInfo.oldArp),1,1,outfile);
		
		fwrite(&(songInfo.virtualTempoNumerator),2,1,outfile);
		fwrite(&(songInfo.virtualTempoDenominator),2,1,outfile);
		writeNullTermStringToFile(songInfo.firstSubsongName, outfile);
		writeNullTermStringToFile(songInfo.firstSubsongComment, outfile);
		fwrite(&(songInfo.numOfOtherSubsongs),1,1,outfile);
		fwrite(emptyBytes,1,3,outfile);
		writeNullTermStringToFile(songInfo.sysName, outfile);
		writeNullTermStringToFile(songInfo.albumCatGameName, outfile);
		fwrite(emptyBytes,1,4,outfile);
		fwrite(&(songInfo.chipVol),4,1,outfile);
		fwrite(&(songInfo.chipPan),4,1,outfile);
		fwrite(&(songInfo.chipFrontRear),4,1,outfile);
		fwrite(&(songInfo.patchbayConnectionCount),4,1,outfile);
		fwrite(&(songInfo.automaticPatchbay),1,1,outfile);
		fwrite(emptyBytes,1,8,outfile);
		fwrite(&(songInfo.lengthOfSpeedPattern),1,1,outfile);
		fwrite(songInfo.speedPattern,1,16,outfile);
		fwrite(&(songInfo.numberOfGrooveListEntries),1,1,outfile);
		fwrite(&(songInfo.pointerToInstrumentDirectories),4,1,outfile);
		fwrite(&(songInfo.pointerToWavetableDirectories),4,1,outfile);
		fwrite(&(songInfo.pointerToSampleDirectories),4,1,outfile);
		fwrite(directoryData,1,15,outfile);
		fwrite(directoryData,1,15,outfile);
		fwrite(directoryData,1,15,outfile);
		
		for (int i=0; i<songInfo.insCount; i++){
			fwrite(instruments[i].blockID,1,4,outfile);
			fwrite(&(instruments[i].size),4,1,outfile);
			fwrite(&(instruments[i].formatVersion),2,1,outfile);
			fwrite(&(instruments[i].insType),2,1,outfile);
			fwrite(instruments[i].features[0].featureCode,1,2,outfile);
			fwrite(&(instruments[i].features[0].length),2,1,outfile);
			
			uint8_t envParams = (std::get<insFeatureGameBoyClass>(instruments[i].features[0].featureData).envLen << 5) | (std::get<insFeatureGameBoyClass>(instruments[i].features[0].featureData).envDirection << 4) | (std::get<insFeatureGameBoyClass>(instruments[i].features[0].featureData).envVol);
			fwrite(&(envParams),1,1,outfile);
			fwrite(&(std::get<insFeatureGameBoyClass>(instruments[i].features[0].featureData).soundLen),1,1,outfile);
			fwrite(&(std::get<insFeatureGameBoyClass>(instruments[i].features[0].featureData).flags),1,1,outfile);
			fwrite(&(std::get<insFeatureGameBoyClass>(instruments[i].features[0].featureData).hardwareSeqLen),1,1,outfile);
		}
		
		//printf("songInfo.wavetableCount: %u\n", songInfo.wavetableCount);
		for (int i=0; i<songInfo.wavetableCount; i++){
			//printf("writing wave %u to file...\n", i);
			fwrite(wavetables[i].blockID,1,4,outfile);
			fwrite(&(wavetables[i].size),4,1,outfile);
			writeNullTermStringToFile(wavetables[i].wavetableName, outfile);
			fwrite(&(wavetables[i].wavetableWidth),4,1,outfile);
			fwrite(emptyBytes,1,4,outfile);
			fwrite(&(wavetables[i].wavetableHeight),4,1,outfile);
			for (int i2=0; i2<wavetables[i].wavetableWidth; i2++){
				fwrite(&(wavetables[i].wavetableData[i2]),4,1,outfile);
			}
		}

		for (int i=0; i<songInfo.patCount; i++){
			fwrite(patterns[i].blockID,1,4,outfile);
			fwrite(&(patterns[i].size),4,1,outfile);
			fwrite(&(patterns[i].subsong),1,1,outfile);
			fwrite(&(patterns[i].channel),1,1,outfile);
			fwrite(&(patterns[i].patIndex),2,1,outfile);
			writeNullTermStringToFile(patterns[i].patName, outfile);
			// patData patRow writing loop goes here
			for (patRow curPatRow : patterns[i].patData) {
				// bytes that indicate what is present. See fur spec
				uint8_t firstByte=0;
				uint8_t secondByte=0;
				uint8_t thirdByte=0;
				bool writeSecondByte=false;
				bool writeThirdByte=false;
				if (curPatRow.skipRow!=0) {
					firstByte = 0b10000000 | (curPatRow.skipRow & 0x7F);
					fwrite(&firstByte,1,1,outfile);
					continue;
				}
				if (curPatRow.effects[1].second==true || curPatRow.effects[2].second==true || curPatRow.effects[3].second==true) {
					firstByte |= 0b00100000;
					writeSecondByte=true;
				}
				if (curPatRow.effects[4].second==true || curPatRow.effects[5].second==true || curPatRow.effects[6].second==true || curPatRow.effects[7].second==true) {
					firstByte |= 0b01000000;
					writeThirdByte=true;
				}
				if (curPatRow.note.second==true) firstByte|=0b1;
				if (curPatRow.ins.second==true) firstByte|=0b10;
				if (curPatRow.vol.second==true) firstByte|=0b100;
				if (curPatRow.effects[0].second==true) {firstByte|=0b1000; secondByte|=0b00000001;}
				if (curPatRow.effectVal[0].second==true) {firstByte|=0b10000; secondByte|=0b00000010;}
				
				for (int effI=1; effI<=3; effI++) {
					if (curPatRow.effects[effI].second==true) secondByte |= (0b1 << (effI*2));
					if (curPatRow.effectVal[effI].second==true) secondByte |= (0b1 << (effI*2+1));
				}
				for (int effI=4; effI<=7; effI++) {
					if (curPatRow.effects[effI].second==true) thirdByte |= (0b1 << (effI*2 - 8));
					if (curPatRow.effectVal[effI].second==true) thirdByte |= (0b1 << ((effI*2 - 8)+1));
				}
				fwrite(&firstByte,1,1,outfile);
				if (writeSecondByte) fwrite(&secondByte,1,1,outfile);
				if (writeThirdByte) fwrite(&thirdByte,1,1,outfile);
				
				// now write the actual values
				if (curPatRow.note.second==true) fwrite(&(curPatRow.note.first),1,1,outfile);
				if (curPatRow.ins.second==true) fwrite(&(curPatRow.ins.first),1,1,outfile);
				if (curPatRow.vol.second==true) fwrite(&(curPatRow.vol.first),1,1,outfile);
				for (int effI=0; effI<8; effI++){
					if (curPatRow.effects[effI].second==true) fwrite(&(curPatRow.effects[effI].first),1,1,outfile);
					if (curPatRow.effectVal[effI].second==true) fwrite(&(curPatRow.effectVal[effI].first),1,1,outfile);
				}
			}
			uint8_t endOfDataByte=0xFF;
			fwrite(&(endOfDataByte),1,1,outfile);
		}
		
		fclose(outfile);
	}
};

struct envAndSoundLen{
	uint8_t sound_length; // 5-0
	
	uint8_t env_start_vol; // 7-4
	bool env_down_or_up; // 3
	uint8_t env_length; // 2-0
	
	bool sound_length_enable; // 6
	
	bool operator==(const envAndSoundLen& other) const {
		return sound_length == other.sound_length &&
			env_start_vol == other.env_start_vol &&
			env_down_or_up == other.env_down_or_up &&
			env_length == other.env_length &&
			sound_length_enable == other.sound_length_enable;
	}
};
static bool compareEnvAndSoundLenExceptVol(envAndSoundLen firstIn, envAndSoundLen secondIn){
	envAndSoundLen temp1 = firstIn;
	envAndSoundLen temp2 = secondIn;
	temp1.env_start_vol=0;
	temp2.env_start_vol=0;
	return temp1 == temp2;
}
static bool isInsSettingInVector(std::vector<struct envAndSoundLen>* uniqueInsSettings, envAndSoundLen* curInsSetting){ // TODO: see if this function can be replaced with an std function now that envAndSoundLen has a == operator.
	for (envAndSoundLen curUniqueSetting : *uniqueInsSettings){
		if ( curUniqueSetting == *curInsSetting ) {
			return true;
		}
	}
	return false;
}
static std::tuple<envAndSoundLen, envAndSoundLen, envAndSoundLen> gbChipState2envAndSoundLen(gb_chip_state curState){
	envAndSoundLen curSettingsSq1;
	curSettingsSq1.sound_length = curState.gb_square1_state.sound_length.first;
	curSettingsSq1.env_start_vol = curState.gb_square1_state.env_start_vol.first;
	curSettingsSq1.env_down_or_up = curState.gb_square1_state.env_down_or_up.first;
	curSettingsSq1.env_length = curState.gb_square1_state.env_length.first;
	curSettingsSq1.sound_length_enable = curState.gb_square1_state.sound_length_enable.first;
	envAndSoundLen curSettingsSq2;
	curSettingsSq2.sound_length = curState.gb_square2_state.sound_length.first;
	curSettingsSq2.env_start_vol = curState.gb_square2_state.env_start_vol.first;
	curSettingsSq2.env_down_or_up = curState.gb_square2_state.env_down_or_up.first;
	curSettingsSq2.env_length = curState.gb_square2_state.env_length.first;
	curSettingsSq2.sound_length_enable = curState.gb_square2_state.sound_length_enable.first;
	envAndSoundLen curSettingsNoi;
	curSettingsNoi.sound_length = curState.gb_noise_state.sound_length.first;
	curSettingsNoi.env_start_vol = curState.gb_noise_state.env_start_vol.first;
	curSettingsNoi.env_down_or_up = curState.gb_noise_state.env_down_or_up.first;
	curSettingsNoi.env_length = curState.gb_noise_state.env_length.first;
	curSettingsNoi.sound_length_enable = curState.gb_noise_state.sound_length_enable.first;
	
	return std::make_tuple(curSettingsSq1, curSettingsSq2, curSettingsNoi);
}
static int closest(std::vector<uint16_t> const& vec, int value) { // https://stackoverflow.com/questions/8647635/elegant-way-to-find-closest-value-in-a-vector-from-above
	auto const it = std::lower_bound(vec.begin(), vec.end(), value);
	if (it == vec.end()) { return -1; }
	return *it;
}
static int closest(std::vector<uint8_t> const& vec, int value) { // https://stackoverflow.com/questions/8647635/elegant-way-to-find-closest-value-in-a-vector-from-above
	auto const it = std::lower_bound(vec.begin(), vec.end(), value);
	if (it == vec.end()) { return -1; }
	return *it;
}
static int closest(std::vector<float> const& vec, float value) { // https://stackoverflow.com/questions/8647635/elegant-way-to-find-closest-value-in-a-vector-from-above
	auto const it = std::lower_bound(vec.begin(), vec.end(), value);
	if (it == vec.end()) { return -1; }
	return *it;
}
static std::pair<uint8_t, uint8_t> gbPitch2noteAndPitch(uint16_t gbPitch){ // NOTE: C2 in furnace seems to be equivalent to C3 in the sound table. https://www.devrs.com/gb/files/sndtab.html
	//uint32_t frequency = 131072/(2048-gbPitch);
	// gbPitch = (-131072 / freq) + 2048
	uint8_t note;
	uint8_t pitchAdjust;
	std::vector<uint16_t> gbPitchArray = {44,156,262,363,457,547,631,710,786,854,923,986,1046,1102,1155,1205,1253,1297,1339,1379,1417,1452,1486,1517,1546,1575,1602,1627,1650,1673,1694,1714,1732,1750,1767,1783,1798,1812,1825,1837,1849,1860,1871,1881,1890,1899,1907,1915,1923,1930,1936,1943,1949,1954,1959,1964,1969,1974,1978,1982,1985,1988,1992,1995,1998,2001,2004,2006,2009,2011,2013,2015}; // length: 72
	uint8_t noteC2=84; // furnace note number
	uint16_t closestGbpitch = closest(gbPitchArray, gbPitch);
	uint8_t gbPitchArrayIndex=std::distance( gbPitchArray.begin(), std::find(gbPitchArray.begin(), gbPitchArray.end(), closestGbpitch) );
	note = noteC2 + gbPitchArrayIndex;
	int pitchDifference = gbPitch - closestGbpitch;
	
	
	if (pitchDifference > 0) {
		if (gbPitchArrayIndex+1 >= gbPitchArray.size()) {
			pitchAdjust = 0x80;
		} else {
			uint16_t totalSemitoneDiff = gbPitchArray[gbPitchArrayIndex+1] - gbPitchArray[gbPitchArrayIndex];
			pitchAdjust = 0x80 + (float)0x80 * ((float)pitchDifference / totalSemitoneDiff);
		}
	} else if (pitchDifference < 0) {
		if (gbPitchArrayIndex <= 0) {
			pitchAdjust = 0x80;
		} else {
			/*
			uint16_t totalSemitoneDiff = gbPitchArray[gbPitchArrayIndex] - gbPitchArray[gbPitchArrayIndex-1];
			pitchAdjust = 0x80 - (float)0x80 * ((float)std::abs(pitchDifference) / totalSemitoneDiff);
			*/
			
			// NOTE: the closest() function above picks the closest value numerically, but that doesn't mean it picks the closest note melodically. The function prefers outputting "C-4 E504" instead of something more readable like "B-3 E584" (from KDL2 title wav channel). The below "pitchAdjustAlter" math corrects the output to something more readable.
			uint16_t totalSemitoneDiff = gbPitchArray[gbPitchArrayIndex] - gbPitchArray[gbPitchArrayIndex-1];
			int pitchAdjustAlter = (float)0x80 * ((float)std::abs(pitchDifference) / totalSemitoneDiff);
			if (pitchAdjustAlter > (0x80 / 2)) { // TODO: make this check possible to disable on the command line.
				note--;
				//pitchAdjust = 0x100 - pitchAdjustAlter;
				pitchAdjust = 0xFF - pitchAdjustAlter;
			} else {
				pitchAdjust = 0x80 - pitchAdjustAlter;
			}
			
		}
	} else {
		pitchAdjust = 0x80;
	}
	return std::make_pair(note, pitchAdjust);
}
static float noisePitch2frequency(uint8_t noisePitch){
	uint8_t s = (noisePitch & 0xF0)>>4;
	float r = noisePitch & 7;
	if (r==0) {r=0.5;}
	uint32_t squaredVal = std::pow(2, s+1);
	return (float)524288 / r / (float)squaredVal;
}
static uint8_t noisePitch2note(uint8_t noisePitch){ // https://github.com/tildearrow/furnace/blob/16920e0e3176787e7684d2643271f4ed89c8a395/src/engine/platform/gb.cpp#L152
	/*
	for long noise:
	C0 through c sharp negative 5 are all the same. They are all FF22 == 00.
	c#0 is 0xf7; d0 is 0xf6;
	d#0 f5
	e0 f4
	f0 e7
	f#0 e6
	g0 e5
	g#0 e4
	a0 d7
	g#5 and above are all FF22==00.
	*/
	uint8_t noteCS0=61;
	const uint8_t VALID_NOISE_PITCHES_SIZE = 68;
	std::vector<uint8_t> validNoisePitches(VALID_NOISE_PITCHES_SIZE);
	std::vector<float> validNoiseFrequencies(VALID_NOISE_PITCHES_SIZE); // stores calculated frequency
	validNoisePitches[0]=0x00;
	validNoisePitches[1]=0x01;
	validNoisePitches[2]=0x02;
	validNoisePitches[3]=0x03;
	std::map<uint8_t, uint8_t> noisePitch2noteTable; // length: 69. noisePitch is the key, furnace note is the value. TODO: get rid of this maybe; validNoisePitches can accomplish the same thing.
	for (int i=0; i<64; i+=4){
		noisePitch2noteTable[0xF7-(i/4)*0x10] = noteCS0+0+i;
		noisePitch2noteTable[0xF6-(i/4)*0x10] = noteCS0+1+i;
		noisePitch2noteTable[0xF5-(i/4)*0x10] = noteCS0+2+i;
		noisePitch2noteTable[0xF4-(i/4)*0x10] = noteCS0+3+i;
		
		validNoisePitches[0+i+4] = 0x04+(i/4)*0x10;
		validNoisePitches[1+i+4] = 0x05+(i/4)*0x10;
		validNoisePitches[2+i+4] = 0x06+(i/4)*0x10;
		validNoisePitches[3+i+4] = 0x07+(i/4)*0x10;
	}
	noisePitch2noteTable[0x03] = 125; // noteF5
	noisePitch2noteTable[0x02] = 126; 
	noisePitch2noteTable[0x01] = 127;
	noisePitch2noteTable[0x00] = 128;
	
	for (int i=0; i<VALID_NOISE_PITCHES_SIZE; i++){
		validNoiseFrequencies[VALID_NOISE_PITCHES_SIZE-1-i] = noisePitch2frequency(validNoisePitches[i]);
		//printf("validNoiseFrequencies[%u]: %f\n", 69-1-i, validNoiseFrequencies[69-1-i]);
		//printf("validNoisePitches[%u]: %u\n", i, validNoisePitches[i]);
	}
	
	uint8_t retNote=128;
	
	if (std::find(validNoisePitches.begin(), validNoisePitches.end(), noisePitch) == validNoisePitches.end()){
		float closestValidFreq = closest(validNoiseFrequencies, noisePitch2frequency(noisePitch));
		uint8_t closestValidFreqIndex = std::distance(validNoiseFrequencies.begin(), std::find(validNoiseFrequencies.begin(), validNoiseFrequencies.end(), closestValidFreq));
		uint8_t closestValidNoisePitch = validNoisePitches[(VALID_NOISE_PITCHES_SIZE-1)-closestValidFreqIndex];
		retNote=noisePitch2noteTable.at(closestValidNoisePitch); // NOTE: please let me know if there is a way to make furnace accept any arbitrary noisePitch instead of being forced to pick from one of the available noisePitches in the note list.
	} else {
		retNote=noisePitch2noteTable.at(noisePitch);
	}
	
	/*
	try {
		retNote=noisePitch2noteTable.at(noisePitch);
	} catch (std::out_of_range e) {
		fprintf(stderr, "noisePitch 0x%X is outside the range of noisePitch2noteTable\n", noisePitch);
	}
	*/
	
	return retNote;
}
static void writeVar(std::pair<uint8_t, bool>& inVar, uint8_t inVal){
	inVar.first=inVal;
	inVar.second=true;
	return;
}
static void pushPatternsToFile(furFileClass::patternClass& curSQ1Pat, furFileClass::patternClass& curSQ2Pat, furFileClass::patternClass& curWavPat, furFileClass::patternClass& curNoiPat, furFileClass& furFile, uint32_t& prevPatsSize, uint16_t& patIndex){
	curSQ1Pat.channel=0;
	curSQ2Pat.channel=1;
	curWavPat.channel=2;
	curNoiPat.channel=3;
	
	curSQ1Pat.patIndex=patIndex;
	curSQ2Pat.patIndex=patIndex;
	curWavPat.patIndex=patIndex;
	curNoiPat.patIndex=patIndex;
	
	curSQ1Pat.size=curSQ1Pat.calculateSize();
	curSQ2Pat.size=curSQ2Pat.calculateSize();
	curWavPat.size=curWavPat.calculateSize();
	curNoiPat.size=curNoiPat.calculateSize();
	
	// calculate pattern pointers. TODO: replace instruments.size() with songInfo.insCount (and do same with wavetables)?
	furFile.songInfo.patPointers.push_back(/*furFile.songInfo.size*/ 0 + furFile.instruments.size()*(furFile.instruments[0].size+4+4) + furFile.wavetables.size()*(furFile.wavetables[0].size+4+4) + prevPatsSize + 15*3 + 40);
	prevPatsSize += curSQ1Pat.size+4+4;
	furFile.songInfo.patPointers.push_back(0 + furFile.instruments.size()*(furFile.instruments[0].size+4+4) + furFile.wavetables.size()*(furFile.wavetables[0].size+4+4) + prevPatsSize + 15*3 + 40);
	prevPatsSize += curSQ2Pat.size+4+4;
	furFile.songInfo.patPointers.push_back(0 + furFile.instruments.size()*(furFile.instruments[0].size+4+4) + furFile.wavetables.size()*(furFile.wavetables[0].size+4+4) + prevPatsSize + 15*3 + 40);
	prevPatsSize += curWavPat.size+4+4;
	furFile.songInfo.patPointers.push_back(0 + furFile.instruments.size()*(furFile.instruments[0].size+4+4) + furFile.wavetables.size()*(furFile.wavetables[0].size+4+4) + prevPatsSize + 15*3 + 40);
	prevPatsSize += curNoiPat.size+4+4;
	
	patIndex++;
	std::array<uint8_t, 4/*gameboy channels*/> newOrder = {(uint8_t)patIndex, (uint8_t)patIndex, (uint8_t)patIndex, (uint8_t)patIndex};
	furFile.songInfo.orders.push_back(newOrder);
	
	furFile.patterns.push_back(curSQ1Pat);
	furFile.patterns.push_back(curSQ2Pat);
	furFile.patterns.push_back(curWavPat);
	furFile.patterns.push_back(curNoiPat);
	curSQ1Pat={};
	curSQ2Pat={};
	curWavPat={};
	curNoiPat={};
}
bool songData2fur(std::vector<gb_chip_state>& songData, std::string outfilename, uint16_t inPatLen, bool disablePanMute, float inTicksPerSecond){
	const uint8_t NOTEOFF=180;
	
	furFileClass furFile;
	furFile.songInfo.patLen = inPatLen;
	furFile.songInfo.ticksPerSecond = inTicksPerSecond;
	std::set<std::array<uint8_t,32>> uniqueWavetables;
	//std::unordered_set<struct envAndSoundLen> uniqueInsSettings; // unordered_set requires the item to have a == operator, which neither structs nor classes come with.
	std::vector<struct envAndSoundLen> uniqueInsSettings;
	for (gb_chip_state curState : songData){
		std::tuple<envAndSoundLen, envAndSoundLen, envAndSoundLen> curInsSettings = gbChipState2envAndSoundLen(curState);
		
		if (!isInsSettingInVector(&uniqueInsSettings, &(std::get<0>(curInsSettings)))) {uniqueInsSettings.push_back(std::get<0>(curInsSettings));}
		if (!isInsSettingInVector(&uniqueInsSettings, &(std::get<1>(curInsSettings)))) {uniqueInsSettings.push_back(std::get<1>(curInsSettings));}
		if (!isInsSettingInVector(&uniqueInsSettings, &(std::get<2>(curInsSettings)))) {uniqueInsSettings.push_back(std::get<2>(curInsSettings));}
		
		if(curState.gb_wave_state.wavetable.second) uniqueWavetables.insert(curState.gb_wave_state.wavetable.first);
	}
	
	// Many GB songs use env_start_vol just to manually change the volume of a channel. Detect if there are multiple instrument settings that are all the same except for env_start_vol, then re-sort them in uniqueInsSettings so that their insIndex (roughly) matches their env_start_vol. This makes the output fur file easier to read.
	std::vector<uint8_t> similarInsTable(uniqueInsSettings.size()); // keeps track of which insSetting has the highest number of similar insSettings.
	for (int i=0; i<uniqueInsSettings.size(); i++){
		uint8_t curSimilars=0;
		for (int i2=0; i2<uniqueInsSettings.size(); i2++){
			if (compareEnvAndSoundLenExceptVol(uniqueInsSettings[i], uniqueInsSettings[i2])) {curSimilars++;}
		}
		similarInsTable[i]=curSimilars;
	}
	envAndSoundLen insSettingWithMostSimilars = uniqueInsSettings[std::distance(similarInsTable.begin(), std::max_element(similarInsTable.begin(), similarInsTable.end()) )];
	std::vector<struct envAndSoundLen> newUniqueInsSettings;
	for (int curVol=0; curVol<=0x0F; curVol++){ // volume
		for (int i2=0; i2<uniqueInsSettings.size(); i2++){
			if (compareEnvAndSoundLenExceptVol(insSettingWithMostSimilars, uniqueInsSettings[i2]) && uniqueInsSettings[i2].env_start_vol==curVol && !isInsSettingInVector(&newUniqueInsSettings, &uniqueInsSettings[i2])) {newUniqueInsSettings.push_back(uniqueInsSettings[i2]);}
		}
	}
	for (int i=0; i<uniqueInsSettings.size(); i++){
		if (!isInsSettingInVector(&newUniqueInsSettings, &uniqueInsSettings[i])) newUniqueInsSettings.push_back(uniqueInsSettings[i]);
	}
	
	// remove garbage all 0 and all 0xFF wavetables from set.
	//std::array<uint8_t,32> all0array = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//std::array<uint8_t,32> all0xFarray = {0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF};
	//uniqueWavetables.erase(all0array);
	//uniqueWavetables.erase(all0xFarray);
	
	/*
	for (std::array<uint8_t,32> curWavetable : uniqueWavetables) {
		for (int i=0; i<32; i++){
			printf("%u ", curWavetable[i]);
		}
		printf("\n");
	}
	*/
	
	/*
	printf("uniqueInsSettings:\n");
	for (envAndSoundLen curSettingPrinted : uniqueInsSettings) {
		printf("%u, %u, %u, %u, %u\n", curSettingPrinted.sound_length, curSettingPrinted.env_start_vol, curSettingPrinted.env_down_or_up, curSettingPrinted.env_length, curSettingPrinted.sound_length_enable);
	}
	printf("\nnewUniqueInsSettings:\n");
	for (envAndSoundLen curSettingPrinted : newUniqueInsSettings) {
		printf("%u, %u, %u, %u, %u\n", curSettingPrinted.sound_length, curSettingPrinted.env_start_vol, curSettingPrinted.env_down_or_up, curSettingPrinted.env_length, curSettingPrinted.sound_length_enable);
	}
	*/
	
	uniqueInsSettings = newUniqueInsSettings;
	
	// add instruments to fur
	for (envAndSoundLen curInsSetting : uniqueInsSettings) {
		furFileClass::instrumentClass curIns;
		furFileClass::insFeatureClass curInsFeature;
		//curInsFeature.featureCode = {'G','B'};
		memcpy(curInsFeature.featureCode, "GB", 2);
		furFileClass::insFeatureGameBoyClass curInsFeatureGB;
		curInsFeatureGB.envLen = curInsSetting.env_length;
		curInsFeatureGB.envDirection = curInsSetting.env_down_or_up;
		curInsFeatureGB.envVol = curInsSetting.env_start_vol;
		curInsFeatureGB.soundLen = curInsSetting.sound_length_enable ? (63 - curInsSetting.sound_length) : 64; // "The higher the length timer, the shorter the time before the channel is cut.". Furnace reverses this to make it more intuitive. furnace sound length 1 == gb sound length 62. for non-wave channels. Furnace ignores wave channel sound length
		curInsFeature.featureData = curInsFeatureGB;
		curIns.features.push_back(curInsFeature);
		//curIns.size; 
		
		furFile.instruments.push_back(curIns);
	}
	
	// add wavetables to fur
	for (std::array<uint8_t,32> curWavetable : uniqueWavetables) {
		furFileClass::wavetableClass curFurWave;
		for (int i=0; i<32; i++){
			curFurWave.wavetableData.push_back(curWavetable[i]);
		}
		curFurWave.size = curFurWave.calculateSize();
		furFile.wavetables.push_back(curFurWave);
	}
	
	furFile.songInfo.insCount=furFile.instruments.size();
	furFile.songInfo.wavetableCount=furFile.wavetables.size();
	
	// below is placeholder code that inserts empty patterns into the file
	/*
	for (int i=0; i<furFile.songInfo.patCount; i++){
		furFileClass::patternClass emptyPat;
		emptyPat.channel=i;
		emptyPat.patIndex=i;
		//furFileClass::patRow curPatRow;
		emptyPat.size=emptyPat.calculateSize();
		furFile.patterns.push_back(emptyPat);
	}
	for (int i=0; i<furFile.patterns.size(); i++){
		furFile.songInfo.patPointers.push_back(furFile.songInfo.size + furFile.instruments.size()*(furFile.instruments[0].size+4+4) + furFile.wavetables.size()*(furFile.wavetables[0].size+4+4) + (furFile.patterns[0].size+4+4)*i + 15*3 + 40);
	}
	*/
	
	// TODO: (maybe) change songData loop to an index loop so I can look at the previous state instead of having all these prev variables.
	uint8_t prevWavetableIndex=0xFF;
	uint8_t prevPulse1insIndex=0xFF;
	uint8_t prevPulse2insIndex=0xFF;
	uint8_t prevNoiseinsIndex=0xFF;
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
	bool SQ1Mute=false;
	bool SQ2Mute=false;
	bool WavMute=false;
	bool NoiMute=false;
	furFileClass::patternClass curSQ1Pat;
	furFileClass::patternClass curSQ2Pat;
	furFileClass::patternClass curWavPat;
	furFileClass::patternClass curNoiPat;
	uint16_t patIndex=0;
	uint32_t prevPatsSize=0;
	for (gb_chip_state curState : songData){
		furFileClass::patRow curSQ1PatRow;
		furFileClass::patRow curSQ2PatRow;
		furFileClass::patRow curWavPatRow;
		furFileClass::patRow curNoiPatRow;
		
		std::tuple<envAndSoundLen, envAndSoundLen, envAndSoundLen> curInsSettings = gbChipState2envAndSoundLen(curState);
		//printf("curInsSettings (sq2): %u, %u, %u, %u, %u\n", std::get<1>(curInsSettings).sound_length, std::get<1>(curInsSettings).env_start_vol, std::get<1>(curInsSettings).env_down_or_up, std::get<1>(curInsSettings).env_length, std::get<1>(curInsSettings).sound_length_enable);
		
		if (curState.gb_wave_state.wavetable.second) {
			uint8_t wavetableIndex = std::distance(std::begin(uniqueWavetables), uniqueWavetables.find(curState.gb_wave_state.wavetable.first));
			if (wavetableIndex != prevWavetableIndex) {
				/*insert wave change into pattern*/
				writeVar(curWavPatRow.effects[2],0x10);
				writeVar(curWavPatRow.effectVal[2],wavetableIndex);
				prevWavetableIndex = wavetableIndex;
			}
		}
		
		// panning
		if (curState.gb_square1_state.panning.second) {
			//printf("valid panning\n");
			uint8_t curSQ1pan=0;
			switch (curState.gb_square1_state.panning.first) {
				case 0:
					curSQ1pan=0x00;
					break;
				case 0b10:
					curSQ1pan=0xF0;
					break;
				case 0b01:
					curSQ1pan=0x0F;
					break;
				case 0b11:
					curSQ1pan=0xFF;
					break;
			}
			//printf("curSQ1pan: 0x%X, curState.gb_square1_state.panning.first: %u\n", curSQ1pan, curState.gb_square1_state.panning.first);
			if(curSQ1pan!=prevSQ1pan){
				//printf("sq1 panning changed\n");
				if (curSQ1pan==0){
					if (!disablePanMute) {
						SQ1Mute=true;
						writeVar(curSQ1PatRow.note,NOTEOFF);
						writeVar(curSQ1PatRow.ins,0);
					}
				} else if (curSQ1pan!=0 && prevSQ1pan==0) {
					SQ1Mute=false;
				}
				writeVar(curSQ1PatRow.effects[4], 0x08);
				writeVar(curSQ1PatRow.effectVal[4], curSQ1pan);
				prevSQ1pan=curSQ1pan;
			}
		}
		if (curState.gb_square2_state.panning.second) {
			uint8_t curSQ2pan=0;
			switch (curState.gb_square2_state.panning.first) {
				case 0:
					curSQ2pan=0x00;
					break;
				case 0b10:
					curSQ2pan=0xF0;
					break;
				case 0b01:
					curSQ2pan=0x0F;
					break;
				case 0b11:
					curSQ2pan=0xFF;
					break;
			}
			//printf("curSQ2pan: 0x%X, curState.gb_square2_state.panning.first: %u\n", curSQ2pan, curState.gb_square2_state.panning.first);
			if(curSQ2pan!=prevSQ2pan){
				//printf("sq2 panning changed\n");
				if (curSQ2pan==0){
					if (!disablePanMute) {
						SQ2Mute=true;
						writeVar(curSQ2PatRow.note,NOTEOFF);
						writeVar(curSQ2PatRow.ins,0); // DMG-AKBJ-JPN.gbs subsong 1. The sq2 wasn't being properly muted at the start (I wonder if it's a bug with Furnace?). I just have to hope that instrument 0 is always a silent instrument.
					}
				} else if (curSQ2pan!=0 && prevSQ2pan==0) {
					SQ2Mute=false;
				}
				writeVar(curSQ2PatRow.effects[2], 0x08);
				writeVar(curSQ2PatRow.effectVal[2], curSQ2pan);
				prevSQ2pan=curSQ2pan;
			}
		}
		if (curState.gb_wave_state.panning.second) {
			uint8_t curWavPan=0;
			switch (curState.gb_wave_state.panning.first) {
				case 0:
					curWavPan=0x00;
					break;
				case 0b10:
					curWavPan=0xF0;
					break;
				case 0b01:
					curWavPan=0x0F;
					break;
				case 0b11:
					curWavPan=0xFF;
					break;
			}
			if(curWavPan!=prevWavPan){
				if (curWavPan==0){
					if (!disablePanMute) {
						WavMute=true;
						//writeVar(curWavPatRow.note,NOTEOFF); // with this: wave gets muted when it shouldn't be.
						writeVar(curWavPatRow.note,84/*noteC2*/);
						writeVar(curWavPatRow.vol,0);
						prevWaveVol=0; // so wave can set itself back to the correct volume once mute has ended.
					}
				} else if (curWavPan!=0 && prevWavPan==0) {
					WavMute=false;
				}
				writeVar(curWavPatRow.effects[1], 0x08);
				writeVar(curWavPatRow.effectVal[1], curWavPan);
				prevWavPan=curWavPan;
			}
		}
		if (curState.gb_noise_state.panning.second) {
			uint8_t curNoiPan=0;
			switch (curState.gb_noise_state.panning.first) {
				case 0:
					curNoiPan=0x00;
					break;
				case 0b10:
					curNoiPan=0xF0;
					break;
				case 0b01:
					curNoiPan=0x0F;
					break;
				case 0b11:
					curNoiPan=0xFF;
					break;
			}
			if(curNoiPan!=prevNoiPan){
				if (curNoiPan==0){
					if (!disablePanMute) {
						NoiMute=true;
						writeVar(curNoiPatRow.note,NOTEOFF);
						writeVar(curNoiPatRow.ins,0);
					}
				} else if (curNoiPan!=0 && prevNoiPan==0) {
					NoiMute=false;
				}
				writeVar(curNoiPatRow.effects[2], 0x08);
				writeVar(curNoiPatRow.effectVal[2], curNoiPan);
				prevNoiPan=curNoiPan;
			}
		}
		
		uint8_t pulse1insIndex = std::distance(std::begin(uniqueInsSettings), std::find(uniqueInsSettings.begin(), uniqueInsSettings.end(), std::get<0>(curInsSettings)));
		uint8_t pulse2insIndex = std::distance(std::begin(uniqueInsSettings), std::find(uniqueInsSettings.begin(), uniqueInsSettings.end(), std::get<1>(curInsSettings)));
		uint8_t noiseInsIndex = std::distance(std::begin(uniqueInsSettings), std::find(uniqueInsSettings.begin(), uniqueInsSettings.end(), std::get<2>(curInsSettings)));
		if (!SQ1Mute) {
			if (pulse1insIndex != prevPulse1insIndex) {
				/*insert instrument change into pattern*/
				writeVar(curSQ1PatRow.ins, pulse1insIndex);
				prevPulse1insIndex = pulse1insIndex;
			}
		}
		if (!SQ2Mute) {
			if (pulse2insIndex != prevPulse2insIndex) {
				/*insert instrument change into pattern*/
				writeVar(curSQ2PatRow.ins, pulse2insIndex);
				prevPulse2insIndex = pulse2insIndex;
			}
		}
		if (!NoiMute) {
			if (noiseInsIndex != prevNoiseinsIndex) {
				/*insert instrument change into pattern*/
				writeVar(curNoiPatRow.ins, noiseInsIndex);
				prevNoiseinsIndex = noiseInsIndex;
			}
		}
		
		if (!WavMute){
			if (curState.gb_wave_state.volume.second) {
				uint8_t curWaveVol;
				switch(curState.gb_wave_state.volume.first){
					case 0:
						curWaveVol=0;
						break;
					case 1:
						curWaveVol=0x0F;
						break;
					case 2:
						curWaveVol=0x08;
						break;
					case 3:
						curWaveVol=0x04;
						break;
				}
				if (curWaveVol!=prevWaveVol){
					writeVar(curWavPatRow.vol,curWaveVol);
					prevWaveVol=curWaveVol;
				}
			}
		}
		
		// note/pitch. trigger
		//printf("%u, %u, %u\n", curState.gb_square2_state.pitch, prevSQ2pitch, curState.gb_square2_state.pitch == prevSQ2pitch);
		//printf("%u, %u, %u\n", curState.gb_noise_state.noise_pitch, prevNoiPitch, curState.gb_noise_state.noise_pitch == prevNoiPitch);
		if (!SQ1Mute) {
			if (curState.gb_square1_state.getPitch() != prevSQ1pitch && curState.gb_square1_state.pitchLSB.second && curState.gb_square1_state.pitchMSB.second) {
				//printf("sq1 pitch is different and valid. curState.gb_square1_state.pitch.first: %u, prevSQ1pitch: %u\n", curState.gb_square1_state.pitch.first, prevSQ1pitch);
				if (curState.gb_square1_state.trigger==0 && SQ1legato==false) {
					//printf("pitch is being changed without triggering note & legato is false.\n");
					writeVar(curSQ1PatRow.effects[5],0xEA);
					writeVar(curSQ1PatRow.effectVal[5],0xFF);
					SQ1legato=true;
				} else if (curState.gb_square1_state.trigger==1 && SQ1legato==true) {
					//printf("note is being triggered & legato is true.\n");
					writeVar(curSQ1PatRow.effects[5],0xEA);
					writeVar(curSQ1PatRow.effectVal[5],0x00);
					SQ1legato=false;
				}
				// insert note
				std::pair<uint8_t, uint8_t> noteAndPitchAdjust = gbPitch2noteAndPitch(curState.gb_square1_state.getPitch());
				writeVar(curSQ1PatRow.note,noteAndPitchAdjust.first);
				writeVar(curSQ1PatRow.effects[3],0xE5);
				writeVar(curSQ1PatRow.effectVal[3],noteAndPitchAdjust.second);
				prevSQ1pitch=curState.gb_square1_state.getPitch();
			} else { // ?
				//printf("sq1 pitch is the same and/or invalid. curState.gb_square1_state.pitch.first: %u, prevSQ1pitch: %u\n", curState.gb_square1_state.pitch.first, prevSQ1pitch);
				if (curState.gb_square1_state.trigger==1) {
					//printf("note %u is being triggered again\n", curState.gb_square1_state.pitch.first);
					if (SQ1legato==true){
						writeVar(curSQ1PatRow.effects[5],0xEA);
						writeVar(curSQ1PatRow.effectVal[5],0x00);
						SQ1legato=false;
					}
					// insert note
					std::pair<uint8_t, uint8_t> noteAndPitchAdjust = gbPitch2noteAndPitch(curState.gb_square1_state.getPitch());
					writeVar(curSQ1PatRow.note,noteAndPitchAdjust.first);
					writeVar(curSQ1PatRow.effects[3],0xE5);
					writeVar(curSQ1PatRow.effectVal[3],noteAndPitchAdjust.second);
				}
			}
		}
		if (!SQ2Mute) {
			if (curState.gb_square2_state.getPitch() != prevSQ2pitch && curState.gb_square2_state.pitchLSB.second && curState.gb_square2_state.pitchMSB.second) {
				//printf("sq2 pitch is different: %u -> %u\n", prevSQ2pitch, curState.gb_square2_state.pitch);
				if (curState.gb_square2_state.trigger==0 && SQ2legato==false) {
					writeVar(curSQ2PatRow.effects[3], 0xEA);
					writeVar(curSQ2PatRow.effectVal[3], 0xFF);
					SQ2legato=true;
				} else if (curState.gb_square2_state.trigger==1 && SQ2legato==true) {
					writeVar(curSQ2PatRow.effects[3], 0xEA);
					writeVar(curSQ2PatRow.effectVal[3], 0x00);
					SQ2legato=false;
				}
				// insert note
				std::pair<uint8_t, uint8_t> noteAndPitchAdjust = gbPitch2noteAndPitch(curState.gb_square2_state.getPitch());
				writeVar(curSQ2PatRow.note, noteAndPitchAdjust.first);
				writeVar(curSQ2PatRow.effects[1], 0xE5);
				writeVar(curSQ2PatRow.effectVal[1], noteAndPitchAdjust.second);
				prevSQ2pitch=curState.gb_square2_state.getPitch();
			} else {
				//printf("sq2 pitch is the same: %u -> %u\n", prevSQ2pitch, curState.gb_square2_state.pitch);
				if (curState.gb_square2_state.trigger==1) {
					//printf("sq2 is being triggered\n");
					if (SQ2legato==true){
						writeVar(curSQ2PatRow.effects[3], 0xEA);
						writeVar(curSQ2PatRow.effectVal[3], 0x00);
						SQ2legato=false;
					}
					// insert note
					std::pair<uint8_t, uint8_t> noteAndPitchAdjust = gbPitch2noteAndPitch(curState.gb_square2_state.getPitch());
					writeVar(curSQ2PatRow.note, noteAndPitchAdjust.first);
					writeVar(curSQ2PatRow.effects[1], 0xE5);
					writeVar(curSQ2PatRow.effectVal[1], noteAndPitchAdjust.second);
				}
			}
		}
		if (!WavMute) {
			if (curState.gb_wave_state.getPitch() != prevWavPitch && curState.gb_wave_state.pitchLSB.second && curState.gb_wave_state.pitchMSB.second) {
				if (curState.gb_wave_state.trigger==0 && WavLegato==false) {
					writeVar(curWavPatRow.effects[3], 0xEA);
					writeVar(curWavPatRow.effectVal[3], 0xFF);
					WavLegato=true;
				} else if (curState.gb_wave_state.trigger==1 && WavLegato==true) {
					writeVar(curWavPatRow.effects[3], 0xEA);
					writeVar(curWavPatRow.effectVal[3], 0x00);
					WavLegato=false;
				}
				// insert note
				std::pair<uint8_t, uint8_t> noteAndPitchAdjust = gbPitch2noteAndPitch(curState.gb_wave_state.getPitch());
				writeVar(curWavPatRow.note, noteAndPitchAdjust.first);
				writeVar(curWavPatRow.effects[0], 0xE5);
				writeVar(curWavPatRow.effectVal[0], noteAndPitchAdjust.second);
				prevWavPitch=curState.gb_wave_state.getPitch();
			} else {
				if (curState.gb_wave_state.trigger==1) {
					if (WavLegato==true){
						writeVar(curWavPatRow.effects[3], 0xEA);
						writeVar(curWavPatRow.effectVal[3], 0x00);
						WavLegato=false;
					}
					// insert note
					std::pair<uint8_t, uint8_t> noteAndPitchAdjust = gbPitch2noteAndPitch(curState.gb_wave_state.getPitch());
					writeVar(curWavPatRow.note, noteAndPitchAdjust.first);
					writeVar(curWavPatRow.effects[0], 0xE5);
					writeVar(curWavPatRow.effectVal[0], noteAndPitchAdjust.second);
				}
			}
		}
		if (!NoiMute) {
			if (curState.gb_noise_state.noise_pitch.first != prevNoiPitch && curState.gb_noise_state.noise_pitch.second) {
				if (curState.gb_noise_state.trigger==0 && NoiLegato==false) {
					writeVar(curNoiPatRow.effects[3], 0xEA);
					writeVar(curNoiPatRow.effectVal[3], 0xFF);
					NoiLegato=true;
				} else if (curState.gb_noise_state.trigger==1 && NoiLegato==true) {
					writeVar(curNoiPatRow.effects[3], 0xEA);
					writeVar(curNoiPatRow.effectVal[3], 0x00);
					NoiLegato=false;
				}
				// insert note
				uint8_t note = noisePitch2note(curState.gb_noise_state.noise_pitch.first);
				writeVar(curNoiPatRow.note, note);
				prevNoiPitch=curState.gb_noise_state.noise_pitch.first;
			} else {
				if (curState.gb_noise_state.trigger==1) {
					if (NoiLegato==true){
						writeVar(curNoiPatRow.effects[3], 0xEA);
						writeVar(curNoiPatRow.effectVal[3], 0x00);
						NoiLegato=false;
					}
					// insert note
					uint8_t note = noisePitch2note(curState.gb_noise_state.noise_pitch.first);
					writeVar(curNoiPatRow.note, note);
					prevNoiPitch=curState.gb_noise_state.noise_pitch.first;
				}
			}
		}
		
		// sweep speed
		// sweep shift
		if ((curState.gb_square1_state.sweep_speed.first != prevSweepSpeed && curState.gb_square1_state.sweep_speed.second) || (curState.gb_square1_state.sweep_shift.first != prevSweepShift && curState.gb_square1_state.sweep_shift.second)) {
			writeVar(curSQ1PatRow.effects[0], 0x13);
			writeVar(curSQ1PatRow.effectVal[0],  (curState.gb_square1_state.sweep_speed.first<<4) | curState.gb_square1_state.sweep_shift.first);
			prevSweepSpeed=curState.gb_square1_state.sweep_speed.first;
			prevSweepShift=curState.gb_square1_state.sweep_shift.first;
		}
		
		// sweep up or down
		if (curState.gb_square1_state.sweep_up_or_down.first != prevSweepDirection && curState.gb_square1_state.sweep_up_or_down.second) {
			writeVar(curSQ1PatRow.effects[1], 0x14);
			writeVar(curSQ1PatRow.effectVal[1], curState.gb_square1_state.sweep_up_or_down.first);
			prevSweepDirection=curState.gb_square1_state.sweep_up_or_down.first;
		}
		
		// duty cycle
		if(curState.gb_square1_state.duty_cycle.first != prevSQ1duty && curState.gb_square1_state.duty_cycle.second){
			writeVar(curSQ1PatRow.effects[2], 0x12);
			writeVar(curSQ1PatRow.effectVal[2], curState.gb_square1_state.duty_cycle.first);
			prevSQ1duty=curState.gb_square1_state.duty_cycle.first;
		}
		if(curState.gb_square2_state.duty_cycle.first != prevSQ2duty && curState.gb_square2_state.duty_cycle.second){
			writeVar(curSQ2PatRow.effects[0], 0x12);
			writeVar(curSQ2PatRow.effectVal[0], curState.gb_square2_state.duty_cycle.first);
			prevSQ2duty=curState.gb_square2_state.duty_cycle.first;
		}
		
		// noise long or short
		if (curState.gb_noise_state.noise_long_or_short.second) {
			if(curState.gb_noise_state.noise_long_or_short.first!=prevNoiseLen){
				writeVar(curNoiPatRow.effects[0], 0x11);
				writeVar(curNoiPatRow.effectVal[0], curState.gb_noise_state.noise_long_or_short.first);
				prevNoiseLen=curState.gb_noise_state.noise_long_or_short.first;
			}
		}
		
		curSQ1Pat.patData.push_back(curSQ1PatRow);
		curSQ2Pat.patData.push_back(curSQ2PatRow);
		curWavPat.patData.push_back(curWavPatRow);
		curNoiPat.patData.push_back(curNoiPatRow);
		
		if (furFile.patterns.size() == 0) { // if this is the first pattern, finish it with one row. All converted songs start with one "garbage", but this garbage row may be neccessary to correctly mute unused channels, thus it is included in the final fur but separated into its own pattern so that pattern length can be used to accurately separate measures.
			furFileClass::patRow jumpPatRow;
			writeVar(jumpPatRow.effects[0], 0x0D);
			curSQ1Pat.patData.push_back(jumpPatRow);
			curSQ2Pat.patData.push_back(jumpPatRow);
			curWavPat.patData.push_back(jumpPatRow);
			curNoiPat.patData.push_back(jumpPatRow);
			
			pushPatternsToFile(curSQ1Pat, curSQ2Pat, curWavPat, curNoiPat, furFile, prevPatsSize, patIndex);
			continue;
		}
		
		// creates a new pattern when we run out of space in the current pattern.
		if (curSQ1Pat.patData.size()>=furFile.songInfo.patLen){ // if one pat is out of space, they should all be out of space.
			pushPatternsToFile(curSQ1Pat, curSQ2Pat, curWavPat, curNoiPat, furFile, prevPatsSize, patIndex);
		}
	}
	furFile.songInfo.patCount = furFile.patterns.size();
	furFile.songInfo.ordersLen = furFile.songInfo.orders.size();
	//if (furFile.songInfo.patCount>256) {fprintf(stderr, "pat count is greater than furnace's maximum. try running for less time. Aborting...\n"); exit(1);}
	
	furFile.songInfo.size = furFile.songInfo.calculateSize();
	
	for (int i=0; i<furFile.songInfo.patPointers.size(); i++){
		furFile.songInfo.patPointers[i]+=furFile.songInfo.size;
	}
	
	furFile.songInfo.pointerToInstrumentDirectories = furFile.songInfo.size + 40;
	furFile.songInfo.pointerToWavetableDirectories = furFile.songInfo.size + 15 + 40;
	furFile.songInfo.pointerToSampleDirectories = furFile.songInfo.size + 15*2 + 40;
	
	// 15*3 is directories
	
	for (int i=0; i<furFile.instruments.size(); i++){
		furFile.songInfo.insPointers.push_back(furFile.songInfo.size + (furFile.instruments[0].size+4+4) * i + 15*3 + 40/*size of header and start of infoBlock*/); // I can do it this way only because instrument size is constant in this program
	}
	for (int i=0; i<furFile.wavetables.size(); i++){
		furFile.songInfo.wavtablPointers.push_back(furFile.songInfo.size + furFile.instruments.size()*(furFile.instruments[0].size+4+4) + (furFile.wavetables[0].size+4+4) * i + 15*3 + 40);
	}
	
	
	FILE* outfile;
	furFile.writeFurFile(outfile, outfilename);
	
	return true;
}