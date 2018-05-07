/*
 * MacroStringReplace.h
 *
 *  Created on: 30.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef MACROSTRINGREPLACE_H_
#define MACROSTRINGREPLACE_H_

#include <pgmspace.h>
#include <Stream.h>
#include <functional>
#include <vector>
#include <map>

class FlashStream: public Stream
{
	public:
		FlashStream(PGM_P ptr, size_t size = 0);
		virtual ~FlashStream() = default;

		int available() override;
		int read() override;
		int peek() override;

		size_t write(uint8_t c) override;
		void flush() override {}

		void reset();

	private:
		PGM_P  ptr;
		size_t totalSize;
		size_t offset = 0;
};


class MemoryStream: public Stream
{
	public:
		MemoryStream(size_t initialSize = 128): buffer(initialSize) {}
		virtual ~MemoryStream() = default;

		int available() override {return 0;}
		int read() override {return 0;}
		int peek() override {return 0;}

		size_t write(uint8_t c) override {buffer.push_back(c); return 1;}
		void flush() override {}

		void reset() {buffer.clear();}
		std::vector<uint8_t> buffer;
};

class StringStream: public Stream
{
	public:
		StringStream(size_t initialSize = 128) {buffer.reserve(initialSize);}
		virtual ~StringStream() {}

		int available() override {return 0;}
		int read() override {return 0;}
		int peek() override {return 0;}

		size_t write(uint8_t c) override
		{
			buffer  += (char)c;
			return 1;
		}

		void flush() override {}

		void reset() {buffer = "";}

		String 	buffer;
};


using Lookup = std::function<String(const char*)>;

Lookup constString(const String& c);

void macroStringReplace(FlashStream& fs, Lookup lookup, Stream& outputStream);

Lookup mapLookup(const std::map<String, String>& m);

#endif /* MACROSTRINGREPLACE_H_ */
