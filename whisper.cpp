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

void show_usage()
{
	cout << "Usage:" << endl;
	cout << "whisper encode <data_file_path> <sound_file_in_path> <sound_file_out_path>" << endl;
	cout << "whisper decode <sound_file_in_path> [data_out_path]" << endl;
}


int main(int argc, char **argv)
{
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
	std::string music_out = ""; 
	std::string music_out_path_string = "";

	fixed_metadata whisper_metadata = my_whisper.get_whisper_metadata();
	int16_t threshold_factor = 8;
	//my_whisper.calc_max_threshold_factor(threshold_factor);
	cout << threshold_factor << endl;

	whisper_metadata.attribits.mask_factor = 0;  // 1 bits per sample
	whisper_metadata.attribits.threshold_factor = threshold_factor;
	//whisper_metadata.attribits.threshold_factor

	//my_whisper.set_whisper_metadata(whisper_metadata);
	//my_whisper.show_whisper_metadata();

	//my_whisper.show_whisper_metadata();

	my_whisper.set_default_metadata();

	my_whisper.show_whisper_metadata();

	//whisper_metadata = my_whisper.get_whisper_metadata();

	if (cmd == "encode")
	{
		if (argc != 5)
		{
			show_usage();
			return -1;
		}

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
		my_whisper.open_files_for_encoding();
		whisper_metadata = my_whisper.get_whisper_metadata();
		whisper_metadata.attribits.threshold_factor += 2;
		whisper_metadata.attribits.mask_factor = 7;
		my_whisper.encode_data(whisper_metadata);
		my_whisper.close_files();

		cout << "Done" << endl;
		return 0;
	}
	else if (cmd == "decode")
	{
		string data_out;
		path p_data_out;

		my_whisper.set_default_metadata();

		if (argc < 3 || argc > 4)
		{
			show_usage();
			return -1;
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
			return -1;
		}
		if (!filesystem::is_regular_file(p_music_in))
		{
			cout << music_in << " must be a regular file" << endl;
			return -1;
		}
		if (!filesystem::exists(p_data_out))
		{
			cout << "Directory path does not exist: " << data_out << endl;
			return -1;
		}
		if (!filesystem::is_directory(p_data_out))
		{
			cout << "Data path is not a directory: " << data_out << endl;
			return -1;
		}
		if (p_music_in == p_data_out)
		{
			std::cout << "Source media and data file cannot be the same. Try again." << std::endl;
			return -1;
		}

		my_whisper.set_out_datapath(p_data_out);
		my_whisper.set_in_musicpath(p_music_in);
		my_whisper.open_files_for_decoding();
		my_whisper.decode_data();
	}
	cout << "Done" << endl;
	return status;
}
