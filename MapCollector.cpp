/*
 * MapCollector.cpp
 *
 *  Created on: 18.04.2017
 *      Author: Bartosz Bielawski
 */

#include "MapCollector.hpp"

using namespace AJSP;

MapCollector::MapCollector(Predicate pred):
		_predicate(pred)
{  
    _parser.setListener(this);
}

MapCollector::~MapCollector()
{
}

void MapCollector::reset()
{
    _values.clear();
    _parser.reset();
}
     
void MapCollector::value(const std::string& value, Parser::Entity entity)
{
	const std::string& path = _parser.getCurrentPath();
	if (!_predicate(path))
		return;

	_values[path] = value;
}

    
