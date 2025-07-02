/*
This file contains the definition of the gb_chip_state class.
*/
#pragma once

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

// https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
// https://gbdev.gg8.se/wiki/articles/Sound_Controller
/*
class gb_chip_state { // bools are used to check whether or not the value is undefined.
public:
	class gb_square1_state_struct {
	public:
		// Register, bits
		// NR10
		std::pair<uint8_t, bool> sweep_speed=std::make_pair(0, false); // 6-4
		std::pair<bool, bool> sweep_up_or_down=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> sweep_shift=std::make_pair(0, false); // 2-0
		// NR11
		std::pair<uint8_t, bool> duty_cycle=std::make_pair(0, false); // 7-6
		std::pair<uint8_t, bool> sound_length=std::make_pair(0, false); // 5-0
		// NR12
		std::pair<uint8_t, bool> env_start_vol=std::make_pair(0, false); // 7-4
		std::pair<bool, bool> env_down_or_up=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> env_length=std::make_pair(0, false); // 2-0. If the value is 0, envelope is disabled.
		// NR13
		std::pair<uint8_t, bool> pitchLSB=std::make_pair(0, false); // 7-0
		// NR14
		bool trigger=false; // 7
		std::pair<bool, bool> sound_length_enable=std::make_pair(0, false); // 6
		std::pair<uint8_t, bool> pitchMSB=std::make_pair(0, false); // 2-0
		
		uint16_t getPitch(){
			return (uint16_t)(pitchLSB.first) | ((uint16_t)(pitchMSB.first) << 8);
		}
		
		std::pair<uint8_t, bool> panning=std::make_pair(0, false); // panning for all channels is set by the sound control register NR51. For this variable: 0b10 is left, 0b01 is right, and 0b11 is center.
	};
	class gb_square2_state_struct {
	public:
		// NR21
		std::pair<uint8_t, bool> duty_cycle=std::make_pair(0, false); // 7-6
		std::pair<uint8_t, bool> sound_length=std::make_pair(0, false); // 5-0
		// NR22
		std::pair<uint8_t, bool> env_start_vol=std::make_pair(0, false); // 7-4
		std::pair<bool, bool> env_down_or_up=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> env_length=std::make_pair(0, false); // 2-0
		// NR23
		std::pair<uint8_t, bool> pitchLSB=std::make_pair(0, false); // 7-0
		// NR24
		bool trigger=false; // 7
		std::pair<bool, bool> sound_length_enable=std::make_pair(0, false); // 6
		std::pair<uint8_t, bool> pitchMSB=std::make_pair(0, false); // 2-0
		
		uint16_t getPitch(){
			return (uint16_t)(pitchLSB.first) | ((uint16_t)(pitchMSB.first) << 8);
		}
		
		std::pair<uint8_t, bool> panning=std::make_pair(0, false);
	};
	class gb_wave_state_struct {
	public:
		// NR31
		std::pair<uint8_t, bool> sound_length=std::make_pair(0, false); // 7-0. value ranges from 0-255, but furnace tracker only goes from 0-63. Convert value range to 0-63 later.
		// NR32
		std::pair<uint8_t, bool> volume=std::make_pair(0, false); // 6-5. Can only be 100, 50, 25, or 0%. For this variable: 0 is 0%, 1 is 100%, 2 is 50%, 3 is 25%
		// NR33
		std::pair<uint8_t, bool> pitchLSB=std::make_pair(0, false); // 7-0
		// NR34
		bool trigger=false; // 7
		std::pair<bool, bool> sound_length_enable=std::make_pair(0, false); // 6
		std::pair<uint8_t, bool> pitchMSB=std::make_pair(0, false); // 2-0
		
		uint16_t getPitch(){
			return (uint16_t)(pitchLSB.first) | ((uint16_t)(pitchMSB.first) << 8);
		}
		
		std::pair<uint8_t, bool> panning=std::make_pair(0, false);
		
		std::pair<std::array<uint8_t,32>, bool> wavetable=std::make_pair(std::array<uint8_t, 32>{}, false); // wave consists of 32 4-bit samples
	};
	class gb_noise_state_struct {
	public:
		// NR41
		std::pair<uint8_t, bool> sound_length=std::make_pair(0, false); // 5-0
		// NR42
		std::pair<uint8_t, bool> env_start_vol=std::make_pair(0, false); // 7-4
		std::pair<bool, bool> env_down_or_up=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> env_length=std::make_pair(0, false); // 2-0
		// NR43
		std::pair<bool, bool> noise_long_or_short=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> noise_pitch=std::make_pair(0, false); // same as the raw NR43 value, except without noise_long_or_short
		// NR44
		bool trigger=false; // 7
		std::pair<uint8_t, bool> sound_length_enable=std::make_pair(0, false); // 6
		
		std::pair<uint8_t, bool> panning=std::make_pair(0, false);
	};
	gb_square1_state_struct gb_square1_state;
	gb_square2_state_struct gb_square2_state;
	gb_wave_state_struct gb_wave_state;
	gb_noise_state_struct gb_noise_state;
};
*/

class gb_chip_state { // bools are used to check whether or not the value is undefined.
public:
	class base_chan_class {
	public:
		virtual ~base_chan_class() = default;
		
		// NRX1
		std::pair<uint8_t, bool> sound_length=std::make_pair(0, false); // 5-0
		// NRX4
		bool trigger=false; // 7
		std::pair<bool, bool> sound_length_enable=std::make_pair(0, false); // 6
		
		std::pair<uint8_t, bool> panning=std::make_pair(0, false); // panning for all channels is set by the sound control register NR51. For this variable: 0b10 is left, 0b01 is right, and 0b11 is center.
	};
	class channels_with_env : public virtual base_chan_class {
	public:
		// NRX2
		std::pair<uint8_t, bool> env_start_vol=std::make_pair(0, false); // 7-4
		std::pair<bool, bool> env_down_or_up=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> env_length=std::make_pair(0, false); // 2-0. If the value is 0, envelope is disabled.
	};
	class melodic_channels : public virtual base_chan_class {
	public:
		// NRX3
		std::pair<uint8_t, bool> pitchLSB=std::make_pair(0, false); // 7-0
		// NRX4
		std::pair<uint8_t, bool> pitchMSB=std::make_pair(0, false); // 2-0
		
		uint16_t getPitch(){
			return (uint16_t)(pitchLSB.first) | ((uint16_t)(pitchMSB.first) << 8);
		}
	};
	class square_channels : public channels_with_env, public melodic_channels {
	public:
		// NRX1
		std::pair<uint8_t, bool> duty_cycle=std::make_pair(0, false); // 7-6
	};
	class square_1 : public square_channels {
	public:
		// NR10
		std::pair<uint8_t, bool> sweep_speed=std::make_pair(0, false); // 6-4
		std::pair<bool, bool> sweep_up_or_down=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> sweep_shift=std::make_pair(0, false); // 2-0
	};
	class wave : public melodic_channels {
	public:
		// NR32
		std::pair<uint8_t, bool> volume=std::make_pair(0, false); // 6-5. Can only be 100, 50, 25, or 0%. For this variable: 0 is 0%, 1 is 100%, 2 is 50%, 3 is 25%
		
		std::pair<std::array<uint8_t,32>, bool> wavetable=std::make_pair(std::array<uint8_t, 32>{}, false); // wave consists of 32 4-bit samples
	};
	class noise : public channels_with_env {
	public:
		// NR43
		std::pair<bool, bool> noise_long_or_short=std::make_pair(0, false); // 3
		std::pair<uint8_t, bool> noise_pitch=std::make_pair(0, false); // same as the raw NR43 value, except without noise_long_or_short
	};
	square_1 gb_square1_state;
	square_channels gb_square2_state;
	wave gb_wave_state;
	noise gb_noise_state;
};
