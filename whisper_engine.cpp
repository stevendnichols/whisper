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

fixed_metadata whisper_engine::get_whisper_metadata()
{
	return fixed_fields;
}

void whisper_engine::set_whisper_metadata(fixed_metadata whisper_fields)
{
	fixed_fields = whisper_fields;
}

ios_base::iostate whisper_engine::decode_whisper_metadata()
{
	uint8_t* md_ptr = (uint8_t *)&fixed_fields;
	uint32_t index = 0;
	ios_base::iostate status = 0;

	for(index = 0; !status && index < sizeof(fixed_fields); index++)
	{
		status = decode_data_byte(md_ptr[index]);
	}

	if (status || index < sizeof(fixed_fields))
	{
		cout << "Unexpected EOF" << endl;
		close_files();
		exit(-1);
	}

	if (fixed_fields.data_byte_count > 2340)
	{
		cout << "Possible misread of whisper metadata. Byte count: " << fixed_fields.data_byte_count << endl;
	}

	string m;

	for (index = 0; index < 7; index++)
	{
		m += (char) fixed_fields.magic[index];
	}
	if (m != "WHISPER")
	{
		cout << "No whisper data found" << endl;
		close_files();
		exit(-1);
	}
	else
	{
		cout << "Magic is " << m << endl;
	}

	return status;
}

ios_base::iostate whisper_engine::decode_whisper_embedded_filename()
{
	uint32_t index = 0;
	uint8_t data_byte = 0;
	ios_base::iostate status = 0;
	filename = "";
	cout << __FUNCTION__ << endl;
	cout << "fixed_fields.attribits.filename_size: " << fixed_fields.attribits.filename_size << endl;

	for (index = 0; !status && index < fixed_fields.attribits.filename_size; index++)
	{
		status = decode_data_byte(data_byte);
		char c = data_byte;
		filename += c;
		cout << c << endl;
	}

	cout << "filename: " << filename << endl;

	if (index < fixed_fields.attribits.filename_size)
	{
		cout << "Unexpected EOF" << endl;
		close_files();
		exit(-1);
	}

	return status;
}

int whisper_engine::decode_data_byte(uint8_t &data_byte)
{
	int16_t sample = 0;
	infile.read((char*)&sample, sizeof(sample));
	auto status = infile.rdstate();

	uint8_t bit_pos = 1;
	uint8_t index = 0;

	int16_t threshold = 0x800;

	data_byte = 0;

	while (!status) 
	{
		int16_t absamp = abs(sample);
		if (absamp >= threshold)
		{
			if (absamp & 1)
			{
				data_byte |= bit_pos;
			}
			bit_pos <<= 1;
			if (!bit_pos)
			{
				return ios_base::goodbit;
			}
		}
		infile.read((char*)&sample, sizeof(sample));
		status = infile.rdstate();
	}

	if (bit_pos)
	{
		cout << "Unexpected EOF" << endl;
		close_files();
		exit(-1);
	}

	return status;
}

int whisper_engine::decode_data()
{
	WavMetadata wav_metadata = { 0 };

	auto status = read_wav_metadata(wav_metadata);

	if (status)
	{
		cout << "WAV metadata read error" << endl;
		return status;
	}

	status = decode_whisper_metadata();

	if (status)
	{
		cout << "WHISPER metadata decode error" << endl;
		return status;
	}

	status = decode_whisper_embedded_filename();

	if (status)
	{
		cout << "WHISPER filename decode error" << endl;
		return status;
	}

	datafilepath /= filename;

	cout << "Creating file" << endl;
	cout << filename << endl;
	cout << datafilepath << endl;

	datafile.open(datafilepath, std::fstream::binary | std::fstream::out | std::fstream::trunc);
	
	if (datafile.fail() || datafile.bad())
	{
		infile.close();
		datafile.close();
		return -1;
	}

	status = decode_hidden_data();

	if (status)
	{
		cout << "Hidden data decode error" << endl;
		return status;
	}

	return status;
}

int whisper_engine::encode_data()
{
	//std::string outfilename = infilename;
	//auto pin = std::filesystem::path(default_inpath_str);
	//auto pout = std::filesystem::path(default_outpath_str);
	//auto pdata_in = std::filesystem::path(default_data_inpath_str);
	int status = 0;
	//Whisper::EmbeddedMetadata metadata;
	//metadata.data_byte_count = data_size;

	fixed_fields.data_byte_count = filesystem::file_size(datafilepath);

	filename = datafilepath.filename().string();
	
	cout << filename << endl;
	if (filename.length() >= 1023  ||  filename.length() < 1)
	{
		cout << "Datafile name must be less than 1023 characters" << endl;
		return -1;
	}
	//strncpy(metadata.file_name, datafilename.c_str(), 254);
	//metadata.file_name[255] = '\0';
	//metadata.max_data_mask = 0x0F;

	//std::cout << infilename << std::endl;
	//std::cout << outfilename << std::endl;
	//std::cout << datafilename << std::endl;

	//cout << pin.string() << endl;

	//if (!std::filesystem::is_directory(pin))
	//{
	//	std::cout << pin.string() << " not a directory" << std::endl;
	//	return -1;
	//}

	//if (!std::filesystem::is_directory(pout))
	//{
	//	std::cout << pout.string() << " not a directory" << std::endl;
	//	return -1;
	//}

	//pin /= infilename;

	//if (!std::filesystem::is_directory(pdata_in))
	//{
	//	std::cout << pdata_in.string() << " not a directory" << std::endl;
	//	return -1;
	//}

	//pdata_in /= datafilename;

	//if (!std::filesystem::is_regular_file(pdata_in))
	//{
	//	std::cout << pdata_in.string() << " not a file" << std::endl;
	//	return -1;
	//}

	//if (!std::filesystem::is_regular_file(pin))
	//{
	//	std::cout << pin.string() << " not a file" << std::endl;
	//	return -1;
	//}

	//pout /= outfilename;

	//std::cout << pin.string() << std::endl;
	//std::cout << pout.string() << std::endl;

	//std::fstream data_infile; 
	//std::fstream infile;
	//std::fstream outfile;

	//std::filesystem::copy_file(pin, pout);

	//if(OpenFiles(pin.string(), pout.string(), pdata_in.string(), infile, outfile, data_infile))
	//	return -1;

	status = copy_wav_metadata();
	//if (status)
	//{
	//	infile.close();
	//	outfile.close();
	//	datafile.close();
	//	return status;
	//}

	status = write_whisper_metadata();

	status = write_whisper_embedded_filename();
	//if (status)
	//{
	//	infile.close();
	//	outfile.close();
	//	datafile.close();
	//	return status;
	//}
	status = write_hidden_data();
	//if (status)
	//{
	//	infile.close();
	//	outfile.close();
	//	datafile.close();
	//	return status;
	//}
	status = copy_remaining_samples();
	close_files();
	return status;
}

void whisper_engine::close_files()
{
	infile.close();
	outfile.close();
	datafile.close();
}

int whisper_engine::open_files_for_decoding()
{
	set<path> unique_names;
	unique_names.insert(datafilepath);
	unique_names.insert(infilepath);

	cout << __FUNCTION__ << endl;

	if (unique_names.size() != 2)
	{
		cout << "Data and media files must be different" << endl;
		return -1;
	}

	cout << infilepath << endl;
	infile.open(infilepath, std::fstream::binary | std::fstream::in); //infilename.string(), std::fstream::binary | std::fstream::in);

	if (infile.eof() || infile.fail() || infile.bad())
	{
		infile.close();
		cout << "Could not open " << infilepath << endl;
		return -1;
	}

	//datafile.open(datafilepath, std::fstream::binary | std::fstream::out | std::fstream::trunc);
	//
	//if (datafile.fail() || datafile.bad())
	//{
	//	infile.close();
	//	datafile.close();
	//	return -1;
	//}

	return 0;
}

//int EmbeddedMetadata::OpenFilesForEncoding(std::filesystem::path infilename, std::filesystem::path outfilename)
int whisper_engine::open_files_for_encoding() //std::filesystem::path infilename, std::filesystem::path outfilename)
{
	//std::set<std::string> unique_names;
	//unique_names.insert(infilename.string());
	//unique_names.insert(datafilename.string());
	//unique_names.insert(outfilename.string());

	set<path> unique_names;
	unique_names.insert(datafilepath);
	unique_names.insert(infilepath);
	unique_names.insert(outfilepath);

	if (unique_names.size() != 3)
	{
		return -1;
	}

	infile.open(infilepath, std::fstream::binary | std::fstream::in); //infilename.string(), std::fstream::binary | std::fstream::in);

	if (infile.eof() || infile.fail() || infile.bad())
	{
		infile.close();
		return -1;
	}

	datafile.open(datafilepath, std::fstream::binary | std::fstream::in);

	if (datafile.eof() || datafile.fail() || datafile.bad())
	{
		infile.close();
		datafile.close();
		return -1;
	}

	filename = datafilepath.filename().string();
	fixed_fields.attribits.filename_size = filename.length();
	fixed_fields.data_byte_count = filesystem::file_size(datafilepath);

	outfile.open(outfilepath, std::fstream::binary | std::fstream::out | std::fstream::trunc);

	if (outfile.fail() || outfile.bad())
	{
		infile.close();
		datafile.close();
		outfile.close();
		return -1;
	}

	return 0;
}

std::ios_base::fmtflags whisper_engine::copy_remaining_samples() // expects open files and does not close them
{
	int16_t sample = 0;
	infile.read((char*)&sample, sizeof(sample));
	auto state = infile.rdstate();
	int rt_val = -__LINE__; 

	while (!state) /// !infile.bad() && !infile.eof() && !infile.fail())
	{
		outfile.write((const char*)&sample, sizeof(sample));
		state = outfile.rdstate();
		if (outfile.eof())
		{
			cout << "outfile.eof" << endl;
			return -__LINE__;
		}
		if (outfile.bad())
		{
			cout << "outfile.bad" << endl;
			return -__LINE__;
		}
		if (outfile.fail())
		{
			cout << "outfile.fail" << endl;
			return -__LINE__;
		}
		rt_val = -__LINE__;
		if(state)
			break;
		infile.read((char*)&sample, sizeof(sample));
		state = infile.rdstate();
	}

	if (state)
	{
		infile.close();
		outfile.close();
		datafile.close();
	}

	return rt_val;
}

std::ios_base::fmtflags whisper_engine::read_wav_metadata(WavMetadata &wav_metadata) // expects open files and does not close them
{
	wav_metadata = { 0 };
	infile.read((char*)&wav_metadata, sizeof(wav_metadata));
	auto state = infile.rdstate();
	if (state)
	{
		cout << "WAV metadata read error " << endl;
		close_files();
		exit (-1);
	}

	return state;
}


std::ios_base::fmtflags whisper_engine::copy_wav_metadata() // expects open files and does not close them
{
	whisper::WavMetadata wav_metadata = { 0 };
	auto state = read_wav_metadata(wav_metadata);

	if (state)
	{
		close_files();
		cout << "Error reading wav file metadata " << endl;
		exit(-1);
	}

	if (wav_metadata.format.format != 1)
	{
		cout << "Source wav file: unsupported format. Not a PCM file. " << endl;
		close_files();
		exit(-1);
	}

	if (wav_metadata.format.numsamplebits % 8 || (wav_metadata.format.numsamplebits != 16))
	{
		close_files();
		cout << "Unsupported bits-per-sample: " << wav_metadata.format.numsamplebits << endl;
		exit(-1);
	}

	if (wav_metadata.format.alignment != wav_metadata.format.numchannels * wav_metadata.format.numsamplebits / 8)
	{
		close_files();
		cout << "Incorrect sample alignmnet in WAV metadata" << endl;
		exit(-1);
	}

	fixed_fields.attribits.sample_bits_select = (wav_metadata.format.numsamplebits / 8) - 1;  
	fixed_fields.attribits.threshold_factor =    wav_metadata.format.numsamplebits / 2;  // default
	fixed_fields.attribits.mask_factor = 1;
	fixed_fields.attribits.skip_min_neg_sample_value = 1;
	fixed_fields.attribits.ignore_sign = 0;

	outfile.write((char*)&wav_metadata, sizeof(wav_metadata));
	state = outfile.rdstate();
	if (state)
	{
		cout << "Failed to write WAV metadata " << endl;
		close_files();
		exit(-1);
	}
	return state;
}

std::ios_base::fmtflags whisper_engine::write_whisper_embedded_filename() // expects open files and does not close them
{
	int16_t sample = 0;
	infile.read((char*)&sample, sizeof(sample));

	//uint8_t* chrptr = (uint8_t*)&fixed_fields;

	uint32_t str_size = datafilepath.filename().string().length();
	const string tmp_filename = datafilepath.filename().string();
	cout << tmp_filename << endl;
	auto chrptr = tmp_filename.c_str();
	uint8_t bit_pos = 1;
	uint8_t index = 0;

	int16_t threshold = 0x800;

	while (!infile.rdstate()) /// !infile.bad() && !infile.eof() && !infile.fail())
	{
		int16_t absamp = abs(sample);
		if (absamp >= threshold)
		{
			absamp &= ~1;
			if (chrptr[index] & bit_pos)
			{
				absamp |= 1;
			}
			if (sample >= 0)
			{
				sample = absamp;
			}
			else
			{
				sample = -absamp;
			}
			bit_pos <<= 1;
			if (!bit_pos)
			{
				cout << chrptr[index] << endl;
				index++;
				bit_pos = 1;
			}
		}
		outfile.write((const char*)&sample, sizeof(sample));
		if (outfile.rdstate())
			break;
		if (index >= str_size)
			return 0;
		infile.read((char*)&sample, sizeof(sample));
	}

	if (index < sizeof(str_size))   // not enough sample space for filename
	{
		cout << "not enough space for filename???" << endl;
		return -1;
	}

	return infile.rdstate() | outfile.rdstate();
}

std::ios_base::fmtflags whisper_engine::write_whisper_metadata() // expects open files and does not close them
{
	int16_t sample = 0;
	infile.read((char*)&sample, sizeof(sample));

	uint8_t* ptr_metadata = (uint8_t*)&fixed_fields;
	uint8_t bit_pos = 1;
	uint8_t md_index = 0;

	int16_t threshold = 0x800;

	while (!infile.rdstate()) /// !infile.bad() && !infile.eof() && !infile.fail())
	{
		int16_t absamp = abs(sample);
		if (absamp >= threshold)
		{
			absamp &= ~1;
			if (ptr_metadata[md_index] & bit_pos)
			{
				absamp |= 1;
			}
			if (sample >= 0)
			{
				sample = absamp;
			}
			else
			{
				sample = -absamp;
			}
			bit_pos <<= 1;
			if (!bit_pos) 
			{
				md_index++;
				bit_pos = 1;
			}
		}
		outfile.write((const char*)&sample, sizeof(sample));
		if (outfile.rdstate())
			break;
		if (md_index >= sizeof(fixed_fields))
			return 0;
		infile.read((char*)&sample, sizeof(sample));
	}

	if (md_index < sizeof(fixed_fields))   // not enough sample space for metadata
	{
		return -1;
	}

	return infile.rdstate() | outfile.rdstate();
}

std::ios_base::fmtflags whisper_engine::write_hidden_data() // expects open files and does not close them
{
	uint8_t single_datum; 
	ios_base::iostate status = 0;
	datafile.read((char *)&single_datum, sizeof(single_datum));
	while (!(status = datafile.rdstate()))
	{
		status = write_single_hidden_datum(&single_datum, sizeof(single_datum));
		if (status)
			break;
		datafile.read((char *)&single_datum, sizeof(single_datum));
	}
	return status;
}

std::ios_base::fmtflags whisper_engine::write_single_hidden_datum(uint8_t *data, int32_t data_width) // expects open files and does not close them
{
	int16_t sample = 0;
	if (data_width < 1)
	{
		return 0;
	}
	infile.read((char*)&sample, sizeof(sample));

	uint8_t* ptr_data = data;
	uint8_t bit_pos = 1;
	uint8_t index = 0;

	int16_t threshold = 0x800;

	while (!infile.rdstate()) /// !infile.bad() && !infile.eof() && !infile.fail())
	{
		int16_t absamp = abs(sample);
		if (absamp >= threshold)
		{
			absamp &= ~1;
			if (ptr_data[index] & bit_pos)
			{
				absamp |= 1;
			}
			if (sample >= 0)
			{
				sample = absamp;
			}
			else
			{
				sample = -absamp;
			}
			bit_pos <<= 1;
			if (!bit_pos)
			{
				index++;
				bit_pos = 1;
			}
		}
		outfile.write((const char*)&sample, sizeof(sample));
		if (outfile.rdstate())
			break;
		if (index >= data_width)
			return 0;
		infile.read((char*)&sample, sizeof(sample));
	}

	if (index < data_width)   // not enough sample space for hidden data
	{
		return -1;
	}

	return infile.rdstate() | outfile.rdstate();
}


std::ios_base::fmtflags whisper_engine::decode_hidden_data() // expects open files and does not close them
{
	uint32_t index = 0;
	uint8_t data_byte = 0;
	ios_base::iostate state = 0;
	cout << __FUNCTION__ << endl;

	for (index = 0; !state && index < fixed_fields.data_byte_count; index++)
	{
		state = decode_data_byte(data_byte);
		if (state)
			break;
		datafile.write((const char*)(&data_byte), 1);
		state = datafile.rdstate();
		if (state)
		{
			cout << "ERROR writing output" << endl;
			close_files();
			exit(-1);
		}
	}

	if (index < fixed_fields.data_byte_count)
	{
		cout << "Unexpected EOF" << endl;
		close_files();
		exit(-1);
	}

	return state;
}


template<typename SAMPLE_TYPE_T>
bool whisper_engine::calc_threshold(SAMPLE_TYPE_T &threshold)
{
	uint8_t max_sample_bits = sizeof(threshold) * 8;
	uint8_t effective_sample_bits = min(sample_bits(), max_sample_bits);
	uint8_t effective_threshold_factor = fixed_fields.attribits.threshold_factor;
	uint8_t max_threshold_factor = effective_sample_bits - 3;
	bool ret_val = effective_threshold_factor <= max_threshold_factor;

	effective_threshold_factor = min(effective_threshold_factor, max_threshold_factor);
	threshold = (effective_threshold_factor ? 1 << effective_threshold_factor: 1);

	return ret_val;
}

template<typename SAMPLE_TYPE_T>
bool whisper_engine::calc_max_data_bitmask(SAMPLE_TYPE_T &bitmask)
{
	SAMPLE_TYPE_T threshold = 0;
	bool ret_val = calc_threshold(threshold);
	bitmask = threshold - 1;
	return ret_val;
}

template<typename SAMPLE_TYPE_T>
bool whisper_engine::get_data_bitmask(SAMPLE_TYPE_T &bitmask)
{
	if(!fixed_fields.attribits.threshold_factor)
		fixed_fields.attribits.threshold_factor = wav_metadata.format.numsamplebits / 2;  // default
	if (fixed_fields.attribits.mask_factor > fixed_fields.attribits.threshold_factor)
		fixed_fields.attribits.mask_factor = fixed_fields.attribits.threshold_factor;
	if (!fixed_fields.attribits.mask_factor)
		fixed_fields.attribits.mask_factor = 1;
	auto mask_factor = fixed_fields.attribits.mask_factor;
	auto threshold_factor = fixed_fields.attribits.threshold_factor;

	SAMPLE_TYPE_T max_bitmask = 0;
	calc_max_data_bitmask(max_bitmask);
	bitmask = ((1 << mask_factor) - 1) & ((1 << threshold_factor) - 1) & max_bitmask;
	return  true;
}

template<typename SAMPLE_TYPE_T>
bool whisper_engine::calc_default_threshold(SAMPLE_TYPE_T &threshold)
{
	uint8_t sizebytes = sizeof(threshold);
	uint8_t shift_factor = (min(sizebytes, sample_bytes()) * 4);

	threshold =  1 << shift_factor;

	return true;
}

uint16_t whisper_engine::data_mask_bitcount()
{
	//threshold_val = calc_nearest_threshold_value(threshold_val);
	uint16_t	bit = 1;
	int16_t		count = 0;
	uint16_t data_mask = 1;

	get_data_bitmask(data_mask);

	while (bit)
	{
		if (data_mask & bit)
			count++;
		bit <<= 1;
	}

	return count;
}

bool whisper_engine::datafile_exists()
{
	return filesystem::exists(datafilepath);
}

bool whisper_engine::create_datafile(filesystem::path filepath)
{
	fstream outfile;

	if (filesystem::exists(filepath))
	{
		std::cout << "File already exists: " << filepath.string() << endl;
		exit(-1);
	}
	if (filepath.filename().string().length() >= 1023)
	{
		cout << "Filename is too long: " << filepath.filename().string() << endl;
		exit(-1);
	}
	outfile.open(filepath.string(), std::fstream::binary | std::fstream::out | std::fstream::trunc);

	if (outfile.fail() || outfile.bad())
	{
		cout << "Unable to create datafile: " << filepath.filename().string() << endl;
		exit(-1);
	}

	filename = filepath.filename().string();
	filepath = filepath;
	outfile.close();

	return 0;
}

bool whisper_engine::set_datafile_name(filesystem::path file_path, bool read_only)
{
	if (file_path.filename().string().length() >= 1023)
	{
		cout << "Filename is too long: " << file_path.filename().string() << endl;
		exit(-1);
	}
	if (read_only)
	{
		if (!filesystem::exists(file_path))
		{
			cout << "Input datafile could not be found" << endl;
			exit(-1);
		}
		if (!filesystem::is_regular_file(file_path))
		{
			cout << "Input datafile is not a regular file" << endl;
			exit(-1);
		}
	}
	else if (filesystem::exists(file_path))
	{
		if (!filesystem::is_directory(file_path))
		{
			cout << "A datafile already exists at that path. " << endl;
			exit(-1);
		}
	}
	filename = "";  // file_path.filename().string();
	datafilepath = file_path;
	return true;
}

bool whisper_engine::set_in_datafile_name(filesystem::path file_path)
{
	return set_datafile_name(file_path, true);
}

bool whisper_engine::set_out_datapath(filesystem::path file_path)
{
	return set_datafile_name(file_path, false);
}

bool whisper_engine::set_out_datadir(filesystem::path file_path)
{
	if (!filesystem::exists(file_path) && !filesystem::create_directories(file_path))
	{
		cout << "Output directory could not be found" << endl;
		exit(-1);
	}
	if (!filesystem::is_directory(file_path))
	{
		cout << "Output filesystem object is not a directory" << endl;
		exit(-1);
	}
	datafilepath = file_path;
	return true;
}

bool whisper_engine::set_in_musicpath(filesystem::path file_path)
{
	if (!filesystem::exists(file_path))
	{
		cout << "Input media file could not be found" << endl;
		exit(-1);
	}
	if (!filesystem::is_regular_file(file_path))
	{
		cout << "Input media file is not a regular file" << endl;
		exit(-1);
	}
	infilepath = file_path;
	return true;
}

bool whisper_engine::set_out_musicpath(filesystem::path file_path)
{
	if (filesystem::exists(file_path))
	{
		cout << "Output media file already exists at this path" << endl;
		exit(-1);
	}
	outfilepath = file_path;
	return true;
}

uint8_t whisper_engine::data_mask_bit_count()
{
	return fixed_fields.attribits.mask_factor;
}

int64_t whisper_engine::precision_mask()
{
	return (1i64 << (sample_bits() - 2)) - 1;
}

bool whisper_engine::magic_is_valid()
{
	char whisper[] = "WHISPER";
	return !memcmp(whisper, fixed_fields.magic, 7);
}

inline uint8_t whisper_engine::sample_bytes()
{
	return 1 + fixed_fields.attribits.sample_bits_select;
}

inline uint8_t whisper_engine::sample_bits()
{
	return 8 * sample_bytes();
}

int32_t whisper_engine::threshold()
{
	return 1 << fixed_fields.attribits.threshold_factor;
}

bool whisper_engine::threshold_is_valid()
{
	return fixed_fields.attribits.threshold_factor && (threshold() > 1 && threshold() < sample_bits() - 2);
}

bool whisper_engine::data_mask_is_valid()
{
	return fixed_fields.attribits.mask_factor && fixed_fields.attribits.mask_factor <= fixed_fields.attribits.threshold_factor;  
}
