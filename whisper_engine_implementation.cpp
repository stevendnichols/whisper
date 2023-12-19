#pragma once
/************************************************************************
 **                                                                    **
 **                           Whisper 1.0                              **
 **                 Copyright 2023 Steven D.Nichols                    **
 **    A steganographic tool for concealing data within audio files    **
 **                                                                    **
 **  Whisper can be found at http ://github.com/stevendnichols/whisper **
 **                                                                    **
 ************************************************************************/


using namespace whisper;

	template <class SAMPLE_TYPE_T>
	void whisper_engine<SAMPLE_TYPE_T>::show_whisper_metadata()
	{
		attribit_fields& ab = fixed_fields.attribits;

		cout << "filename_size: " << ab.filename_size << endl;
		cout << "ignore_sign: " << ab.ignore_sign << endl;
		cout << "mask_factor: " << ab.mask_factor << endl;
		cout << "sample_bits_select: " << ab.sample_bits_select << endl;
		cout << "skip_min_neg_sample_value: " << ab.skip_min_neg_sample_value << endl;
		cout << "threshold_factor: " << ab.threshold_factor << endl;

		cout << "data_byte_count: " << fixed_fields.data_byte_count << endl;

		cout << "sample_bitmask: " << (uint32_t)sample_bitmask << endl;
		cout << "sample_bitmask_size: " << (uint32_t)sample_bitmask_size << endl;
		cout << "sample_threshold: " << sample_threshold << endl << endl;

	}

	template <class SAMPLE_TYPE_T>
	void whisper_engine<SAMPLE_TYPE_T>::set_default_metadata()
	{
		fixed_metadata whisper_metadata = { 0 }; //  = get_whisper_metadata();
		//cout << threshold_factor << endl;
		fixed_fields = { { 'W','H','I','S','P','E','R' }, {0}, 0 };

		whisper_metadata = { { 'W','H','I','S','P','E','R' }, {0}, 0 };
		whisper_metadata.attribits.filename_size = 0;
		whisper_metadata.attribits.sample_bits_select = 1;
		whisper_metadata.attribits.ignore_sign = false;
		whisper_metadata.attribits.skip_min_neg_sample_value = 1;
		whisper_metadata.attribits.mask_factor = 0;  // 1 bits per sample
		whisper_metadata.attribits.threshold_factor = 8;
		whisper_metadata.data_byte_count = 0;

		set_whisper_metadata(whisper_metadata);

		show_whisper_metadata();
	}


	template <class SAMPLE_TYPE_T>
	fixed_metadata whisper_engine <SAMPLE_TYPE_T>::get_whisper_metadata()
	{
		return fixed_fields;
	}

	template <class SAMPLE_TYPE_T>
	void whisper_engine <SAMPLE_TYPE_T>::init_whisper_metadata()
	{
		fixed_metadata md = { { 'W','H','I','S','P','E','R' }, {0}, 0 };
		attribit_fields& a = md.attribits;
		a.filename_size = 0;
		a.ignore_sign = false;
		a.mask_factor = 0;
		a.sample_bits_select = 1;
		a.skip_min_neg_sample_value = true;
		//a.threshold_factor = 


	}

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::set_whisper_metadata(fixed_metadata whisper_fields)
	{
		fixed_fields = whisper_fields;

		//calc_max_data_bitmask(sample_bitmask); 
		//calc_data_bitmask(sample_bitmask);
		calc_threshold(sample_threshold);
		calc_mask_num_bits(sample_bitmask_size);

		if (sample_bitmask >= sample_threshold)
		{
			cout << "Mask factor: " << fixed_fields.attribits.mask_factor << endl;
			cout << "Threshold factor: " << fixed_fields.attribits.threshold_factor << endl;
			cout << "Data bitmask (" << (uint16_t)sample_bitmask << ")" << " exceeds sample threshold (" << sample_threshold << ")" << endl;
			exit(-1);
		}

		return true;
	}

	template <class SAMPLE_TYPE_T>
	ios_base::iostate whisper_engine <SAMPLE_TYPE_T>::decode_whisper_metadata()
	{
		fixed_metadata metadata;
		uint8_t* md_ptr = (uint8_t*)&metadata;
		uint32_t index = 0;
		ios_base::iostate status = 0;

		for (index = 0; !status && index < sizeof(metadata); index++)
		{
			status = decode_metadata_byte(md_ptr[index]);
		}

		if (status || index < sizeof(metadata))
		{
			cout << "Unexpected EOF" << endl;
			close_files();
			exit(-1);
		}

		//if (metadata.data_byte_count > 2340)
		//{
		//	cout << "Possible misread of whisper metadata. Byte count: " << metadata.data_byte_count << endl;
		//}

		string m;

		for (index = 0; index < 7; index++)
		{
			m += (char)metadata.magic[index];
		}
		if (m != "WHISPER")
		{
			cout << "No whisper data found" << endl;
			close_files();
			exit(-1);
		}
		else
		{
			cout << "Identified whisper content" << endl;
		}

		//set_whisper_metadata(metadata);
		set_default_metadata();
		//metadata =  get_whisper_metadata();
		//set_whisper_metadata(metadata);
		show_whisper_metadata();
		return status;
	}

	template <class SAMPLE_TYPE_T>
	ios_base::iostate whisper_engine <SAMPLE_TYPE_T>::decode_whisper_embedded_filename()
	{
		uint32_t index = 0;
		uint8_t data_byte = 0;
		ios_base::iostate status = 0;
		filename = "";

		for (index = 0; !status && index < fixed_fields.attribits.filename_size; index++)
		{
			status = decode_metadata_byte(data_byte);
			char c = data_byte;
			filename += c;
		}

		if (index < fixed_fields.attribits.filename_size)
		{
			cout << "Unexpected EOF" << endl;
			close_files();
			exit(-1);
		}

		return status;
	}

	template <class SAMPLE_TYPE_T>
	int whisper_engine <SAMPLE_TYPE_T>::decode_metadata_byte(uint8_t& data_byte)
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

	template <class SAMPLE_TYPE_T>
	ios_base::iostate whisper_engine <SAMPLE_TYPE_T>::decode_data_byte(uint8_t& data_byte)
	{
		uint8_t datamask = get_data_bitmask();
		uint8_t mask_bitsize = sample_bitmask_size;
		//uint8_t mask_shift_val = mask_bitsize;
		//uint8_t partial_datum = 0;
		const int16_t sample_mask = datamask;
		uint8_t fragment_elevator = 0;
		const uint8_t elevator_shift_size = data_mask_bit_count();
		uint8_t elevator_shift_position = 1;
		uint8_t elevator_shift_val = 0;
		uint8_t elevator_mask = datamask;

		//uint8_t data_bit_pos = 1;
		//uint8_t sample_bit_pos = 1;

		int16_t threshold = get_data_threshold();

		int16_t sample = 0;

		auto status = 0; // = infile.rdstate();

		//uint8_t bit_pos = 1;
		//uint8_t index = 0;

		data_byte = 0;

		while (elevator_mask)
		{
			infile.read((char*)&sample, sizeof(sample));
			status = infile.rdstate();
			if (status)
				break;
			int16_t absamp = abs(sample);
			if (absamp >= threshold)
			{
				fragment_elevator = absamp & sample_mask;
				elevator_shift_val = (elevator_shift_size * elevator_shift_position);
				if (elevator_shift_val)
				{
					fragment_elevator <<= elevator_shift_val;
				}
				data_byte |= fragment_elevator;
				elevator_shift_position++;
				elevator_shift_val = (elevator_shift_size * elevator_shift_position);
				elevator_mask <<= elevator_shift_val;
			}
		}

		if (elevator_mask)
		{
			cout << "Unexpected EOF" << endl;
			close_files();
			exit(-1);
		}

		return status;
	}

	template <class SAMPLE_TYPE_T>
	int whisper_engine <SAMPLE_TYPE_T>::decode_data_byteOLD(uint8_t& data_byte)
	{
		uint8_t datamask = get_data_bitmask();
		uint8_t mask_bitsize = sample_bitmask_size;
		uint8_t mask_shift_val = mask_bitsize;
		uint8_t partial_datum = 0;
		const int16_t sample_mask = datamask;

		uint8_t data_bit_pos = 1;
		uint8_t sample_bit_pos = 1;

		int16_t threshold = get_data_threshold();

		int16_t sample = 0;
		infile.read((char*)&sample, sizeof(sample));
		auto status = infile.rdstate();

		uint8_t bit_pos = 1;
		uint8_t index = 0;

		data_byte = 0;

		while (!status && datamask)
		{
			int16_t absamp = abs(sample);
			if (absamp >= threshold)
			{
				datamask = sample_mask;
				partial_datum = (absamp & datamask);
				partial_datum <<= mask_shift_val;
				datamask <<= mask_shift_val;
				data_byte |= partial_datum;
				mask_shift_val += mask_bitsize;
				if (!datamask)
				{
					return ios_base::goodbit;
				}
			}
			infile.read((char*)&sample, sizeof(sample));
			status = infile.rdstate();
		}

		if (datamask)
		{
			cout << "Unexpected EOF" << endl;
			close_files();
			exit(-1);
		}

		return status;
	}

	template <class SAMPLE_TYPE_T>
	int whisper_engine <SAMPLE_TYPE_T>::decode_data()
	{
		WavMetadata wav_metadata = { 0 };

		auto status = read_wav_metadata(wav_metadata);

		if (status)
		{
			cout << "WAV metadata read error" << endl;
			return status;
		}

		status = decode_whisper_metadata();
		set_default_metadata();

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

		cout << "Creating file " << filename << endl;

		datafile.open(datafilepath, std::fstream::binary | std::fstream::out | std::fstream::trunc);

		if (datafile.fail() || datafile.bad())
		{
			infile.close();
			datafile.close();
			exit(-1);
		}

		status = decode_hidden_data();

		if (status)
		{
			cout << "Hidden data decode error" << endl;
			exit(-1);
		}

		return status;
	}

	template <class SAMPLE_TYPE_T>
	int whisper_engine <SAMPLE_TYPE_T>::encode_data(fixed_metadata metadata)
	{
		int status = 0;
		uint8_t mask = get_data_bitmask();

		fixed_fields.data_byte_count = filesystem::file_size(datafilepath);
		cout << "Data byte count: " << fixed_fields.data_byte_count << endl;

		filename = datafilepath.filename().string();

		if (filename.length() >= 1023 || filename.length() < 1)
		{
			cout << "Datafile name must have fewer than 1023 characters" << endl;
			exit(-1);
		}

		copy_wav_metadata();

		//int16_t threshold_factor = 8;
		//cout << threshold_factor << endl;

		//whisper_metadata.attribits.mask_factor = 0;  // 1 bits per sample
		//whisper_metadata.attribits.threshold_factor = threshold_factor;

		//set_whisper_metadata(whisper_metadata);

		write_whisper_metadata();

		fixed_metadata whisper_metadata = get_whisper_metadata();
		whisper_metadata.attribits.threshold_factor = metadata.attribits.threshold_factor;
		whisper_metadata.attribits.mask_factor = metadata.attribits.mask_factor;
		set_whisper_metadata(whisper_metadata);

		write_whisper_embedded_filename();
		show_whisper_metadata();
		write_hidden_data();
		copy_remaining_samples();
		close_files();
		return 0;
	}

	template <class SAMPLE_TYPE_T>
	void whisper_engine <SAMPLE_TYPE_T>::close_files()
	{
		infile.close();
		outfile.close();
		datafile.close();
	}

	template <class SAMPLE_TYPE_T>
	int whisper_engine <SAMPLE_TYPE_T>::open_files_for_decoding()
	{
		set<path> unique_names;
		unique_names.insert(datafilepath);
		unique_names.insert(infilepath);

		if (unique_names.size() != 2)
		{
			cout << "Data and media files must be different" << endl;
			exit(-1);
		}

		infile.open(infilepath, std::fstream::binary | std::fstream::in);

		if (infile.eof() || infile.fail() || infile.bad())
		{
			infile.close();
			cout << "Could not open " << infilepath << endl;
			exit(-1);
		}

		return 0;
	}


	template <class SAMPLE_TYPE_T>
	int whisper_engine <SAMPLE_TYPE_T>::open_files_for_encoding()
	{
		set<path> unique_names;
		unique_names.insert(datafilepath);
		unique_names.insert(infilepath);
		unique_names.insert(outfilepath);

		if (unique_names.size() != 3)
		{
			cout << "Each filename must be unique. Try again. " << endl;
			exit(-1);
		}

		infile.open(infilepath, std::fstream::binary | std::fstream::in);

		if (infile.eof() || infile.fail() || infile.bad())
		{
			cout << "Failed to open media input file: " << infilepath.string() << endl;
			infile.close();
			return -1;
		}

		datafile.open(datafilepath, std::fstream::binary | std::fstream::in);

		if (datafile.eof() || datafile.fail() || datafile.bad())
		{
			cout << "Failed to open data file: " << datafilepath.string() << endl;
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
			cout << "Failed to create media output file: " << outfilepath.string() << endl;
			close_files();
			exit(-1);
		}

		return 0;
	}

	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::copy_remaining_samples()
	{
		int16_t sample = 0;
		infile.read((char*)&sample, sizeof(sample));
		auto state = infile.rdstate();

		while (!state)
		{
			outfile.write((const char*)&sample, sizeof(sample));
			if (outfile.bad() || outfile.fail())
			{
				cout << "File write error" << endl;
				close_files();
				return -1;
			}
			infile.read((char*)&sample, sizeof(sample));
			state = infile.rdstate();
			if (infile.eof())
			{
				close_files();
				return ios_base::goodbit;
			}
			if (infile.fail() || infile.bad())
			{
				cout << "File read error " << endl;
				close_files();
				exit(-1);
			}
		}

		close_files();
		return state;
	}

	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::read_wav_metadata(WavMetadata& wav_metadata)
	{
		wav_metadata = { 0 };
		infile.read((char*)&wav_metadata, sizeof(wav_metadata));
		auto state = infile.rdstate();
		if (state)
		{
			cout << "WAV metadata read error " << endl;
			close_files();
			exit(-1);
		}

		return state;
	}


	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::copy_wav_metadata()
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

		if (wav_metadata.format.numsamplebits % 8 || (wav_metadata.format.numsamplebits != 16))  // currently only 16-bit is supported
		{
			cout << "Unsupported bits-per-sample: " << wav_metadata.format.numsamplebits << endl;
			close_files();
			exit(-1);
		}

		if (wav_metadata.format.alignment != wav_metadata.format.numchannels * wav_metadata.format.numsamplebits / 8)
		{
			cout << "Incorrect sample alignmnet in WAV metadata" << endl;
			close_files();
			exit(-1);
		}

		//fixed_fields.attribits.sample_bits_select = (wav_metadata.format.numsamplebits / 8) - 1;  
		//fixed_fields.attribits.threshold_factor =    wav_metadata.format.numsamplebits / 2;  // default
		//fixed_fields.attribits.mask_factor = 0;  // default (this is the only supported value at this time
		//fixed_fields.attribits.skip_min_neg_sample_value = true;
		//fixed_fields.attribits.ignore_sign = 0;

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

	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::write_whisper_embedded_filename() // expects open files and does not close them
	{
		int16_t sample = 0;
		infile.read((char*)&sample, sizeof(sample));

		uint32_t str_size = datafilepath.filename().string().length();
		const string tmp_filename = datafilepath.filename().string();

		auto chrptr = tmp_filename.c_str();
		uint8_t bit_pos = 1;
		uint8_t index = 0;

		int16_t threshold = 0x800;

		while (!infile.rdstate())
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
			cout << "not enough space for filename" << endl;
			return -1;
		}

		return infile.rdstate() | outfile.rdstate();
	}

	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::write_whisper_metadata() // expects open files and does not close them
	{
		int16_t sample = 0;
		infile.read((char*)&sample, sizeof(sample));

		uint8_t* ptr_metadata = (uint8_t*)&fixed_fields;
		uint8_t bit_pos = 1;
		uint8_t md_index = 0;

		int16_t threshold = 0x800;

		while (!infile.rdstate())
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

	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::write_hidden_data() // expects open files and does not close them
	{
		uint8_t single_datum = 0;
		uint32_t count = 0;
		ios_base::iostate status = 0;
		datafile.read((char*)&single_datum, sizeof(single_datum));
		while (!(status = datafile.rdstate()))
		{
			status = write_single_hidden_datum(single_datum, count);
			//assemble_masked_data()
			if (status)
				break;
			datafile.read((char*)&single_datum, sizeof(single_datum));
		}
		if (count < fixed_fields.data_byte_count)
		{
			cout << "Insufficient space for sample data. " << fixed_fields.data_byte_count - count << "more needed. " << endl;
			return -1;
		}
		return status;
	}

	template <class SAMPLE_TYPE_T>
	void whisper_engine <SAMPLE_TYPE_T>::calc_mask_num_bits(uint8_t& num_bits)
	{
		uint8_t factor = fixed_fields.attribits.mask_factor;

		if (factor == 0)
		{
			num_bits = 1;
			return;
		}

		num_bits = 1 << factor;
	}


	template<class SAMPLE_TYPE_T>
	bool whisper_engine<SAMPLE_TYPE_T>::assemble_masked_dataOLD(SAMPLE_TYPE_T& sample, uint8_t& data_bit_pos, uint8_t& sample_bit_pos, uint8_t& data_mask, const SAMPLE_TYPE_T sample_mask, const uint8_t masked_data)
	{
		int16_t absamp = abs(sample);
		absamp &= ~sample_mask;

		if (absamp < sample_threshold)
		{
			return true;
		}
		uint8_t data_mask_bits = data_mask_bit_count();

		while (data_bit_pos & data_mask && sample_bit_pos & sample_mask)
		{
			if (masked_data & data_bit_pos)
			{
				absamp |= sample_bit_pos;
			}
			if (sample >= 0)
			{
				sample = absamp;
			}
			else
			{
				sample = -absamp;
			}
			sample_bit_pos <<= 1;
			data_bit_pos <<= 1;
		}
		data_mask <<= data_mask_bits;

		return true;
	}


	template<class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::assemble_masked_data(uint8_t& byte_data, SAMPLE_TYPE_T& sample, uint8_t& progress_mask)
	{
		int16_t absamp = abs(sample);
		const uint8_t sample_mask = get_data_bitmask();
		absamp &= ~sample_mask;

		if (!progress_mask)
			return true;
		if (absamp < sample_threshold)
		{
			return false;
		}

		uint8_t data_mask_bits = data_mask_bit_count();
		uint8_t masked_data = byte_data & sample_mask;

		absamp |= masked_data;

		byte_data >>= data_mask_bits;
		progress_mask >>= data_mask_bits;

		if (sample >= 0)
		{
			sample = absamp;
		}
		else
		{
			sample = -absamp;
		}

		return true;
	}

	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::write_single_hidden_datum(uint8_t& data, uint32_t& count) // expects open files and does not close them
	{
		int16_t sample = 0;
		uint8_t progress_mask = ~0;
		ios_base::fmtflags status = 0;

		infile.read((char*)&sample, sizeof(sample));

		int16_t threshold = get_data_threshold();

		while (!(status = infile.rdstate()) && progress_mask)
		{
			assemble_masked_data(data, sample, progress_mask);

			outfile.write((const char*)&sample, sizeof(sample));

			if ((status = outfile.rdstate()))
			{
				return status;
			}

			count++;

			infile.read((char*)&sample, sizeof(sample));
		}

		if (progress_mask)
			return status;
		else if (status && !(status & ios_base::eofbit))
			return status;
		else
			count++;
		return status;
	}

	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::write_single_hidden_datumOLD2(uint8_t& data, uint32_t& count) // expects open files and does not close them
	{
		int16_t sample = 0;
		uint8_t datamask = get_data_bitmask();
		uint8_t mask_bitsize = sample_bitmask_size;
		const int16_t sample_mask = datamask;

		infile.read((char*)&sample, sizeof(sample));

		uint8_t data_bit_pos = 1;
		uint8_t sample_bit_pos = 1;

		int16_t threshold = get_data_threshold();

		while (!infile.rdstate() && datamask)
		{
			assemble_masked_data(data, sample, datamask);

			outfile.write((const char*)&sample, sizeof(sample));

			count++;

			if (outfile.rdstate())
			{
				break;
			}
			if (!datamask)
			{
				return ios_base::goodbit;
			}

			infile.read((char*)&sample, sizeof(sample));
		}

		return infile.rdstate() | outfile.rdstate();
	}


	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::write_single_hidden_datumOLD(uint8_t* data, int32_t data_width) // expects open files and does not close them
	{
		int16_t sample = 0;
		uint8_t mask = get_data_bitmask();
		uint8_t mask_bitsize = sample_bitmask_size;
		//calc_m

		if (data_width < 1)
		{
			return 0;
		}
		infile.read((char*)&sample, sizeof(sample));

		uint8_t* ptr_data = data;
		uint8_t bit_pos = 1;
		uint8_t index = 0;

		int16_t threshold = get_data_threshold();

		while (!infile.rdstate())
		{
			int16_t absamp = abs(sample);
			if (absamp >= threshold)
			{
				absamp &= ~mask;
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


	template <class SAMPLE_TYPE_T>
	std::ios_base::fmtflags whisper_engine <SAMPLE_TYPE_T>::decode_hidden_data() // expects open files and does not close them
	{
		uint32_t index = 0;
		uint8_t data_byte = 0;
		ios_base::iostate state = 0;

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

	template<class SAMPLE_TYPE_T>
	void whisper_engine<SAMPLE_TYPE_T>::calc_max_data_bitmask(SAMPLE_TYPE_T &bitmask)
	{
		SAMPLE_TYPE_T threshold = 0;
		calc_threshold(threshold);
		bitmask = threshold - 1;
	}

	template <class SAMPLE_TYPE_T>
	int8_t whisper_engine <SAMPLE_TYPE_T>::get_data_bitmask()
	{
		return sample_bitmask;
	}

	template <class SAMPLE_TYPE_T>
	uint16_t whisper_engine <SAMPLE_TYPE_T>::get_data_threshold()
	{
		return sample_threshold;
	}

	template <class SAMPLE_TYPE_T>
	void whisper_engine <SAMPLE_TYPE_T>::calc_data_bitmask(SAMPLE_TYPE_T& bitmask)
	{
		uint8_t num_bits = 1;

		calc_mask_num_bits(num_bits);
		SAMPLE_TYPE_T threshold = 2;
		calc_threshold(threshold);
		SAMPLE_TYPE_T bitmask_tmp = (1L << num_bits) - 1;
	}

	template <class SAMPLE_TYPE_T>
	void whisper_engine <SAMPLE_TYPE_T>::calc_default_threshold(SAMPLE_TYPE_T& threshold)
	{
		uint8_t sizebytes = sizeof(threshold);
		uint8_t shift_factor = (min(sizebytes, sample_bytes()) * 4);

		if (shift_factor < 1)
			threshold = 2;
		else
			threshold = ((SAMPLE_TYPE_T)2) << shift_factor;

		return true;
	}

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::datafile_exists()
	{
		return filesystem::exists(datafilepath);
	}

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::create_datafile(filesystem::path filepath)
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

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::set_datafile_name(filesystem::path file_path, bool read_only)
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
		filename = "";
		datafilepath = file_path;
		return true;
	}

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::set_in_datafile_name(filesystem::path file_path)
	{
		return set_datafile_name(file_path, true);
	}

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::set_out_datapath(filesystem::path file_path)
	{
		return set_datafile_name(file_path, false);
	}

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::set_out_datadir(filesystem::path file_path)
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

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::set_in_musicpath(filesystem::path file_path)
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

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::set_out_musicpath(filesystem::path file_path)
	{
		if (filesystem::exists(file_path))
		{
			cout << "Output media file already exists at this path" << endl;
			exit(-1);
		}
		outfilepath = file_path;
		return true;
	}

	template <class SAMPLE_TYPE_T>
	uint8_t whisper_engine <SAMPLE_TYPE_T>::data_mask_bit_count()
	{
		uint8_t mask_factor = fixed_fields.attribits.mask_factor;
		if (!mask_factor)
			return 1;
		return 1 << mask_factor;
	}

	template <class SAMPLE_TYPE_T>
	int64_t whisper_engine <SAMPLE_TYPE_T>::precision_mask()
	{
		return (1i64 << (sample_bits() - 2)) - 1;
	}

	template <class SAMPLE_TYPE_T>
	bool whisper_engine <SAMPLE_TYPE_T>::magic_is_valid()
	{
		char whisper_str[] = "WHISPER";
		return !memcmp(whisper_str, fixed_fields.magic, 7);
	}

	template <class SAMPLE_TYPE_T>
	inline uint8_t whisper_engine <SAMPLE_TYPE_T>::sample_bytes()
	{
		return 1 + fixed_fields.attribits.sample_bits_select;
	}

	template <class SAMPLE_TYPE_T>
	inline uint8_t whisper_engine <SAMPLE_TYPE_T>::sample_bits()
	{
		return 8 * sample_bytes();
	}

	
	template <class SAMPLE_TYPE_T>
	void whisper_engine <SAMPLE_TYPE_T>::calc_max_threshold(SAMPLE_TYPE_T &threshold)
	{
		threshold = 2 << THRESHOLD_FACTOR_MAX;
	}

	template <class SAMPLE_TYPE_T>
	void whisper_engine <SAMPLE_TYPE_T>::calc_threshold(SAMPLE_TYPE_T &threshold)
	{
		int32_t t_factor = fixed_fields.attribits.threshold_factor;
		auto max_factor = t_factor;
		calc_absolute_max_threshold_factor(max_factor);

		if (t_factor < 1)
		{
			threshold = 2;
			return;
		}
		else if (t_factor > max_factor)
		{
			t_factor = max_factor;
			calc_max_threshold(threshold);
			return;
		}
		threshold = 2L << t_factor;
	}

