/************************************************************************
 **                                                                    **
 **                           Whisper 1.0                              ** 
 **                 Copyright 2023 Steven D.Nichols                    **
 **    A steganographic tool for concealing data within audio files    **
 **                                                                    **
 **  Whisper can be found at http ://github.com/stevendnichols/whisper **
 **                                                                    ** 
 ************************************************************************/

#pragma once

#include <numeric>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>
#include <filesystem>
#include <string>
#include <cstring>
#include <algorithm>

using namespace std;
using namespace std::filesystem;

namespace whisper
{
	const bool ENCODE = true;
	const bool DECODE = false;

	const std::string default_inpath_str("WhisperFiles\\music_in");
	const std::string default_outpath_str("WhisperFiles\\music_out");
	const std::string default_data_inpath_str("WhisperFiles\\data_in");
	const std::string default_data_outpath_str("WhisperFiles\\data_out");

#pragma pack(push, 1)

	typedef struct RiffChunk
	{
		char	id[4];
		int32_t	size;
		char	format[4];
	} RiffChunk;

	typedef struct FormatChunk
	{
		char	id[4];
		int32_t	size;
		int16_t	format;
		int16_t numchannels;
		int32_t	samplerate;
		int32_t	byterate;
		int16_t	alignment;
		int16_t	numsamplebits;
	} FormatChunk;

	typedef struct DataHeader
	{
		char	id[4];
		int32_t	size;
	} DataHeader;

	typedef struct WavMetadata
	{
		RiffChunk	riff;
		FormatChunk	format;
		DataHeader	dataheader;
	} WavMetadata;

	typedef union SampleData
	{
		int16_t	channels[2];
		int32_t	data;
	} SampleData;

	typedef struct attribit_fields
	{
		uint32_t
			sample_bits_select : 2,			// sample_bits == 8 * (sample_bits_select + 1);  for now, this is always 1
			threshold_factor : 5,			// threshold = 1 << threshold_factor;   1 < threshold  <=   (1 << (sample_bits - 3))
			mask_factor : 2,				// mask = ((1 << mask_factor) - 1) & ((1 << threshold_factor) - 1)
			ignore_sign : 1,				// only default (false) is currently supported
			skip_min_neg_sample_value : 1,  // default is false, but this flag is currently ignored, so effectively, the sample is always skipped
			unused : 11,
			filename_size : 10;				// This allows for an excessive amount of metadata for which sufficient space may not be available: YMMV!
	} attribit_fields;

	typedef struct fixed_metadata
	{
		uint8_t  magic[7];                 //should be { 'W','H','I','S','P','E','R' }
		attribit_fields attribits;
		uint32_t data_byte_count;          // size of data to be embedded
	} fixed_metadata;
#pragma pack(pop)

		template<class SAMPLE_TYPE_T>
		class whisper_engine
		{
		private:
			SAMPLE_TYPE_T foo;
			fixed_metadata fixed_fields;
			int16_t max_sample_threshold;
			int16_t sample_threshold;
			uint8_t sample_bitmask;
			uint8_t sample_bitmask_size;
			filesystem::path datafilepath;
			std::string filename;
			filesystem::path infilepath;
			filesystem::path outfilepath;
			std::fstream infile;
			std::fstream outfile;
			std::fstream datafile;
			const int8_t THRESHOLD_FACTOR_MAX;
			WavMetadata wav_metadata;
		public:
			void calc_threshold(SAMPLE_TYPE_T &threshold);
			bool assemble_masked_dataOLD(SAMPLE_TYPE_T& sample, uint8_t& data_bit_pos, uint8_t& sample_bit_pos, uint8_t& data_mask, const SAMPLE_TYPE_T sample_mask, const uint8_t masked_data);
			bool assemble_masked_data(uint8_t& byte_data, SAMPLE_TYPE_T& sample, uint8_t& progress_mask);
			void calc_max_data_bitmask(SAMPLE_TYPE_T& bitmask);
			void calc_data_bitmask(SAMPLE_TYPE_T& bitmask);
			void calc_default_threshold(SAMPLE_TYPE_T& threshold);
			void calc_max_threshold(SAMPLE_TYPE_T &threshold);

			static void check_sample_compatibility(uint8_t alternative_size = 8 * sizeof(SAMPLE_TYPE_T))
			{
				uint8_t word_size = 8 * sizeof(SAMPLE_TYPE_T);
				if (alternative_size == 0)
					alternative_size = word_size;
				if (alternative_size % 8)
				{
					cout << "Invalid sample size: " << alternative_size << "-bit" << endl;
					exit(-1);
				}
				if (alternative_size > word_size || word_size >= alternative_size * 2)
				{
					cout << "Requested sample size " << alternative_size << "-bit " << " not compatible with specified word size " << word_size << "-bit" << endl;
					exit(-1);
				}
				switch (word_size)
				{
				case 16: // for now, 16 is the only supported sample size
					break;
				case 8:
				case 32:
				case 64:
				case 128:
					cout << "Unsupported word size: " << word_size << "-bit" << endl;
					exit(-1);
				}
				switch (alternative_size)
				{
				case 16: // for now, 16 is the only supported sample size
					break;
				case 8:
				case 24:
				case 32:
				case 40:
				case 48:
				case 56:
				case 64:
				case 72:
				case 80:
				case 88:
				case 96:
				case 104:
				case 112:
				case 120:
				case 128:
					cout << "Unsupported sample size: " << alternative_size << "-bit" << endl;
					exit(-1);
				default: break;
				}
			}

			static uint8_t calc_absolute_max_threshold_factor(uint8_t alternative_sample_size = 0)
			{
				uint8_t word_size = 8 * sizeof(SAMPLE_TYPE_T);
				check_sample_compatibility(alternative_sample_size);
				if (alternative_sample_size == 0 || alternative_sample_size == word_size)
				{
					return word_size - 4;
				}
				return alternative_sample_size - 4;
			}

			//template<typename SAMPLE_TYPE_T>
			whisper_engine() : THRESHOLD_FACTOR_MAX(calc_absolute_max_threshold_factor())
			{
				uint8_t sample_size = 8 * sizeof(SAMPLE_TYPE_T);
				if (sample_size != 16)
				{
					cout << "Unsupported sample size: " << sample_size << "-bit" << endl;  // for now
				}
				//if (alternative_size < sizeof(SAMPLE_TYPE_T) && !(alternative_size % 8))
				{
					//8 * alternative_sample_size - 4;
					//void show_whisper_metadata();
				}

			}
			whisper_engine(uint8_t alternative_sample_size) : THRESHOLD_FACTOR_MAX(calc_absolute_max_threshold_factor(alternative_sample_size))
			{
				//sample_bitmask = 1;
				//sample_bitmask_size = 1;
				//sample_threshold = 0x100;
				wav_metadata = { 0 };
				set_default_metadata();
			}

			int decode_data();
			int encode_data(fixed_metadata metadata);
			int open_files_for_decoding();
			int open_files_for_encoding();
			void close_files();
			void show_whisper_metadata();
			//template <typename T> void show_whisper_metadata();
			void set_default_metadata();
			fixed_metadata get_whisper_metadata();
			void init_whisper_metadata();
			bool set_whisper_metadata(fixed_metadata whisper_fields);
			ios_base::iostate  decode_whisper_metadata();
			ios_base::iostate  decode_data_byte(uint8_t& data_byte);
			int decode_data_byteOLD(uint8_t& data_byte);
			ios_base::iostate decode_whisper_embedded_filename();
			int decode_metadata_byte(uint8_t& data_byte);
			std::ios_base::fmtflags copy_wav_metadata();

			std::ios_base::fmtflags write_whisper_metadata();
			std::ios_base::fmtflags write_whisper_embedded_filename();

			std::ios_base::fmtflags write_hidden_data();

			void calc_mask_num_bits(uint8_t& num_bits);

			//void calc_mask_num_bits(uint16_t& num_bits);

			std::ios_base::fmtflags write_single_hidden_datum(uint8_t& data, uint32_t& count);

			std::ios_base::fmtflags write_single_hidden_datumOLD2(uint8_t& data, uint32_t& count);

			std::ios_base::fmtflags write_single_hidden_datumOLD(uint8_t* data, int32_t data_width);

			std::ios_base::fmtflags decode_hidden_data();

			int8_t get_data_bitmask();

			uint16_t get_data_threshold();

			bool datafile_exists();

			bool create_datafile(filesystem::path filepath);

			bool set_datafile_name(filesystem::path file_path, bool read_only);

			bool set_in_datafile_name(filesystem::path file_path);

			bool set_out_datapath(filesystem::path file_path);

			bool set_out_datadir(filesystem::path file_path);

			bool set_in_musicpath(filesystem::path file_path);

			bool set_out_musicpath(filesystem::path file_path);

			uint8_t data_mask_bit_count();

			int64_t precision_mask();

			bool magic_is_valid();

			uint8_t sample_bytes();

			uint8_t sample_bits();

			std::ios_base::fmtflags copy_remaining_samples(); // expects open files and does not close them

			std::ios_base::fmtflags read_wav_metadata(WavMetadata& wav_metadata);

		};

};

#include "whisper_engine_implementation.h"
