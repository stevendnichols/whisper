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
			sample_bits_select : 3,			// sample_bits == 8 * (sample_bits_select + 1);  for now, this is always 1
			threshold_factor : 5,			// threshold = 1 << threshold_factor;   1 < threshold  <=   (1 << (sample_bits - 3))
			mask_factor : 3,				// mask = ((1 << mask_factor) - 1) & ((1 << threshold_factor) - 1)
			ignore_sign : 1,				// only default (false) is currently supported
			skip_min_neg_sample_value : 1,  // default is false, but this flag is currently ignored, so effectively, the sample is always skipped
			unused : 9,
			filename_size : 10;				// This allows for an excessive amount of metadata for which sufficient space may not be available: YMMV!
	} attribit_fields;

	typedef struct fixed_metadata
	{
		uint8_t  magic[7];                 //should be { 'W','H','I','S','P','E','R' }
		attribit_fields attribits;
		uint32_t data_byte_count;          // size of data to be embedded
	} fixed_metadata;
#pragma pack(pop)

	class whisper_engine
	{
	private:
		fixed_metadata fixed_fields;
		filesystem::path datafilepath;
		std::string filename;
		filesystem::path infilepath;
		filesystem::path outfilepath;
		std::fstream infile;
		std::fstream outfile;
		std::fstream datafile;
		WavMetadata wav_metadata;
	public:
		template<typename SAMPLE_TYPE_T>
		void calc_threshold(SAMPLE_TYPE_T& threshold);
		template<typename SAMPLE_TYPE_T>
		bool calc_max_data_bitmask(SAMPLE_TYPE_T& bitmask);
		template<typename SAMPLE_TYPE_T>
		bool calc_data_bitmask(SAMPLE_TYPE_T& bitmask);
		template<typename SAMPLE_TYPE_T>
		bool calc_default_threshold(SAMPLE_TYPE_T& threshold);
		template<typename SAMPLE_TYPE_T>
		void calc_max_threshold_factor(SAMPLE_TYPE_T& threshold_factor);
		template<typename SAMPLE_TYPE_T>
		void calc_max_threshold(SAMPLE_TYPE_T& threshold);

		whisper_engine()
		{
			fixed_fields = { { 'W','H','I','S','P','E','R' }, {0}, 0 };
			fixed_fields.attribits.threshold_factor = 8;
			fixed_fields.attribits.sample_bits_select = 1;
			fixed_fields.attribits.skip_min_neg_sample_value = true;
			wav_metadata = { 0 };
		}
		int encode_data();
		int decode_data();
		int open_files_for_decoding();
		int open_files_for_encoding(); 
		void close_files();
		fixed_metadata get_whisper_metadata();
		void set_whisper_metadata(fixed_metadata whisper_fields);
		ios_base::iostate  decode_whisper_metadata();
		ios_base::iostate  decode_data_byte(uint8_t& data_byte);
		ios_base::iostate decode_whisper_embedded_filename();
		std::ios_base::fmtflags copy_wav_metadata();

		std::ios_base::fmtflags write_whisper_metadata();
		std::ios_base::fmtflags write_whisper_embedded_filename();

		std::ios_base::fmtflags write_hidden_data();

		std::ios_base::fmtflags write_single_hidden_datum(uint8_t* data, int32_t data_width);

		std::ios_base::fmtflags decode_hidden_data();

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


}

