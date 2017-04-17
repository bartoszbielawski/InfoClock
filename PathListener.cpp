/*
 * PathListener.cpp
 *
 *  Created on: 04.01.2017
 *      Author: caladan
 */

#include "PathListener.h"

using namespace AJSP;


void PathListener::value(const std::string& value, AJSP::Parser::Entity)
{
	const std::string& currentPath = parser->getCurrentPath();
	for (const char* p: _monitoredPaths)
	{
		
		if (currentPath == p)
			callback(currentPath, value);
	}
}

