/*
 * MacroStringReplace.cpp
 *
 *  Created on: 30.01.2017
 *      Author: Bartosz Bielawski
 */

#include "MacroStringReplace.h"
#include "utils.h"

FlashStream::FlashStream(PGM_P ptr, size_t size): ptr(ptr), totalSize(size)
{
	if (size == 0)
		totalSize = strlen_P(ptr);
}

int FlashStream::available()
{
	return totalSize - offset;
}

int FlashStream::read()
{
	if (offset >=  totalSize)
		return 0;

	return pgm_read_byte(ptr+offset++);
}

int FlashStream::peek()
{
	if (offset >=  totalSize)
		return 0;

	return pgm_read_byte(ptr+offset);
}

size_t FlashStream::write(uint8_t c)
{
	return 0;
}

void FlashStream::reset()
{
	offset = 0;
}


void macroStringReplace(FlashStream& fs, Lookup lookup, Stream& outputStream)
{
	char localBuffer[128];
	bool inKey = false;

	fs.reset();

	while (uint32_t available = fs.available())
	{

		int toRead = min(available, sizeof(localBuffer));

		int read = fs.readBytesUntil('$', localBuffer, toRead);
		if (read == toRead)
		{
			outputStream.write(localBuffer, toRead);
			continue;		//otherwise...
		}

		inKey = not inKey;

		if (not inKey)
		{
			if (read == 0)
			{
				outputStream.write('$');
				continue;
			}

			localBuffer[read]= '\0';
			outputStream.write(lookup(localBuffer).c_str());
			continue;
		}

		outputStream.write(localBuffer, read);
	}
}

Lookup constString(const String& c)
{
	return [c](const char* argument) {return c;};
}

Lookup mapLookup(const std::map<String, String>& m)
{
	return [m](const char* argument)
	{
		auto f = m.find(argument);
		if (f == m.end())
			return String();

		return f->second;
	};
}

