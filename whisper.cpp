/************************************************************************
 **                                                                    **
 **                           Whisper 1.0                              **
 **                 Copyright 2023 Steven D.Nichols                    **
 **    A steganographic tool for concealing data within audio files    **
 **                                                                    **
 **  Whisper can be found at http ://github.com/stevendnichols/whisper **
 **                                                                    **
 ************************************************************************/

#include "whisper.h"

using namespace whisper;

/*
namespace attribit_fields
{
	const uint16_t	MASK_IS_FIXED = 0x1;
	const uint16_t	IGNORE_SAMPLE_VALUES = 0x3;
	const uint16_t	IGNORE_THRESHOLDS = 0x7;
	const uint16_t	IGNORE_SIGN_OF_INT16_MIN = 0x8;
	const uint16_t	DO_NOT_ALTER_SAMPS_INT16_MIN = 0x8;
	const uint16_t	IGNORE_SIGN = 0x18;
	const uint16_t	SKIP_SAMPLE_VAL_INT16_MIN = 0x20;
	const uint16_t	DO_NOT_INTERLEAVE_PACKED_DATA = 0x40;
	const uint16_t	OMIT_ONE_CHANNEL = 0x80;
	const uint16_t	ENCODE_CHANNEL_ONE_FIRST = 0x100;
}
*/

void show_usage()
{
	cout << "Usage:" << endl;
	cout << "whisper encode <data_file_path> <sound_file_in_path> <sound_file_out_path>" << endl;
	cout << "whisper decode <sound_file_in_path> [data_out_path]" << endl;
}


int main(int argc, char **argv)
{
    //std::is_in

	whisper_engine my_whisper;

	if (argc < 3)
	{
		show_usage();
		return -__LINE__;
	}

	int status = 0;
	std::set<std::string> cmds;

	cmds.insert("encode");
	cmds.insert("decode");

    std::string cmd = argv[1];

	if (cmds.count(cmd) < 1)
	{
		show_usage();
		return -__LINE__;
	}

	std::string music_in = "";
	std::string data_in = "";
	std::string music_out = ""; //"music_out.wav";
	std::string music_out_path_string = "";

	//char data[] = "We wonder how much data we can hide in this way";
	//char extracted_data[128];

	if (cmd == "encode")
	{
		if (argc != 5)
		{
			show_usage();
			return -__LINE__;
		}

		fixed_metadata whisper_metadata = my_whisper.get_whisper_metadata();

		whisper_metadata.attribits.mask_factor = 1;
		whisper_metadata.attribits.ignore_sign = false;
		whisper_metadata.attribits.sample_bits_select = 1;
		whisper_metadata.attribits.skip_min_neg_sample_value = true;
		whisper_metadata.attribits.threshold_factor = 8;

		my_whisper.set_whisper_metadata(whisper_metadata);

		data_in = argv[2];
		music_in = argv[3];
		music_out = argv[4];

		auto p_music_in = std::filesystem::path(music_in);
		auto p_data_in = std::filesystem::path(data_in);
		auto p_music_out = std::filesystem::path(music_out);

		if (!std::filesystem::exists(p_music_in))
		{
			std::cout << "No such file:  " << music_in << std::endl;
			return -__LINE__;
		}
		if (!std::filesystem::is_regular_file(p_music_in))
		{
			std::cout << music_in << " must be a regular file" << std::endl;
			return -__LINE__;
		}
		if (!std::filesystem::exists(p_data_in))
		{
			std::cout << "No such file:  " << data_in << std::endl;
			return -__LINE__;
		}
		if (!std::filesystem::is_regular_file(p_data_in))
		{
			std::cout << data_in << " must be a regular file" << endl;
			return -__LINE__;
		}
		if (p_music_in == p_data_in)
		{
			std::cout << "Source data and media cannot be the same file. Try again." << std::endl;
			return -__LINE__;
		}
		if (p_music_out == p_data_in)
		{
			std::cout << "Source data and media cannot be the same file. Try again." << std::endl;
			return -__LINE__;
		}
		if (p_music_in == p_music_out)
		{
			std::cout << "Media files may not be the same. Try again." << std::endl;
			return -__LINE__;
		}

		my_whisper.set_in_datafile_name(p_data_in);
		my_whisper.set_in_musicpath(p_music_in);
		my_whisper.set_out_musicpath(p_music_out);

		status = my_whisper.open_files_for_encoding();
		if (status)
			cout << __LINE__ << endl;

		status = my_whisper.encode_data();

		if (status)
			cout << __LINE__ << endl;

		return status;

		//std::cout << "Enter a filename for the output: ";
		//std::cin >> music_out;

		//std::cout << "Enter a directory path for the output: ";
		//std::cin >> music_out_path_string;

		//p_music_out = std::filesystem::path(music_out_path_string);
		if (!std::filesystem::exists(p_music_out) && !std::filesystem::create_directories(p_music_out))
		{
			std::cout << music_out_path_string << endl << "Could not create direcotries for the given path" << endl;
			return -__LINE__;
		}

		p_music_out /= music_out;

		if (p_music_in == p_music_out)
		{
			std::cout << "Cannot copy a file onto itself. Try again." << endl;
			return -__LINE__;
		}
		if (p_data_in == p_music_out)
		{
			std::cout << "Cannot embed a file into itself. Try again." << endl;
			return -__LINE__;
		}

		status = my_whisper.open_files_for_encoding();

		if (status)
			return -__LINE__;

		cout << __LINE__ << endl;

		status = my_whisper.copy_remaining_samples();
		cout << __LINE__ << endl;

		return __LINE__;

		status = my_whisper.encode_data();
	}
	else if (cmd == "decode")
	{
		string data_out;
		path p_data_out;

		if (argc < 3 || argc > 4)
		{
			show_usage();
			return -__LINE__;
		}
		else if (argc < 4)
		{
			p_data_out = filesystem::current_path();
		}
		else
		{
			data_out = argv[3];
			p_data_out = path(data_out);
		}

		music_in = argv[2];

		auto p_music_in = filesystem::path(music_in);

		if (!filesystem::exists(p_music_in))
		{
			cout << "No such file:  " << music_in << endl;
			return -__LINE__;
		}
		if (!filesystem::is_regular_file(p_music_in))
		{
			cout << music_in << " must be a regular file" << endl;
			return -__LINE__;
		}
		if (!filesystem::exists(p_data_out))
		{
			cout << "Directory path does not exist: " << data_out << endl;
			return -__LINE__;
		}
		if (!filesystem::is_directory(p_data_out))
		{
			cout << "Data path is not a directory: " << data_out << endl;
			return -__LINE__;
		}
		if (p_music_in == p_data_out)
		{
			std::cout << "Source media and data file cannot be the same. Try again." << std::endl;
			return -__LINE__;
		}

		my_whisper.set_out_datapath(p_data_out);
		my_whisper.set_in_musicpath(p_music_in);

		status = my_whisper.open_files_for_decoding();
		if (status)
			cout << -__LINE__ << endl;

		status = my_whisper.decode_data(); 

		if (status)
			cout << -__LINE__ << endl;

		status = my_whisper.copy_remaining_samples();

		return status;
}
