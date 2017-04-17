/*
 * PathConstructor.cpp
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */

#include "PathConstructor.hpp"
#include <string.h>

PathConstructor::PathConstructor(uint32_t reserve, char separator): size(0), separator(separator)
{
	buffer.reserve(reserve);
	sizes.reserve(reserve / 8);
}

PathConstructor::~PathConstructor()
{
}

void PathConstructor::push(const std::string& s)
{
	sizes.push_back(s.size()+1);	//this part + separator
	size += s.size() + 1;

	buffer += separator;
	buffer.append(s);

}

void PathConstructor::push(const char* s)
{
	int len = strlen(s) + 1;
	sizes.push_back(len);
	size += len;

	buffer += separator;
	buffer.append(s);
}

void PathConstructor::pop()
{
	if (sizes.size() == 0)
		return;			//nothing to pop

	int sizeToRemove = sizes.back();
	sizes.pop_back();
	int startPos = buffer.size() - sizeToRemove;
	buffer.erase(startPos, sizeToRemove);
	size -= sizeToRemove;
}

void PathConstructor::clear()
{
	sizes.clear();
	buffer.clear();
}
