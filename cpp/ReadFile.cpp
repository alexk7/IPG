//
//  ReadFile.cpp
//
//  Created by Alexandre Cossette on 10-12-14.
//  Copyright 2010 Alexandre Cossette. All rights reserved.
//

#include "Common.h"
#include "ReadFile.h"

bool ReadFile(std::vector<char>& _text, const char* _filename)
{
	std::ifstream file(_filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		// If the file offset at the end of the file is bigger than the maximum
		// size_t value in bytes (usually 4GB on 32-bit machines), it means the file
		// won't fit in memory even if the file system supports such files!
		std::ifstream::pos_type lastpos = file.tellg();
		file.seekg(0, std::ios::beg);
		std::ifstream::pos_type firstpos = file.tellg();
		std::size_t textsize = static_cast<std::size_t>(lastpos - firstpos);
		
		if (firstpos + std::ifstream::off_type(textsize) != lastpos)
			return false;
		
		_text.resize(textsize);
		file.read(&_text[0], textsize);
		file.close();
		
		return true;
	}
	
	return false;
}
