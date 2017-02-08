/*
 * PathListener.cpp
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */

#include "PathListener.h"

#include "Arduino.h"

using namespace AJSP;

PathListener::PathListener(Parser* p): Listener(p)
{
}

PathListener::~PathListener()
{
}


void PathListener::arrayStart()
{
	pathConstructor.push(parser->getLastKey());
}

void PathListener::arrayEnd()
{
	pathConstructor.pop();
}

void PathListener::objectStart()
{
	pathConstructor.push(parser->getLastKey());
}

void PathListener::objectEnd()
{
	pathConstructor.pop();
}

void PathListener::key(const std::string& key)
{

}

void PathListener::value(const std::string& value, AJSP::Parser::Entity entity)
{
	pathConstructor.push(parser->getLastKey());
	for (const char* p: _monitoredPaths)
	{
		if (pathConstructor.getPath() == p)
			callback(pathConstructor.getPath(), value);
	}
	pathConstructor.pop();
}

void PathListener::done()
{

}

void PathListener::clear()
{
	pathConstructor.clear();
}
