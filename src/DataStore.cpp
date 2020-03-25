/*
 * DataStore.cpp
 *
 *  Created on: 05.02.2017
 *      Author: Bartosz Bielawski
 */

#include "DataStore.h"

static DataStore::DataStore<String, String>& dataStore()
{
	static DataStore::DataStore<String, String> ds;
	return ds;
}

bool DataStore::hasValue(const String& key)
{
	return dataStore().hasKey(key);
}

String& DataStore::value(const String& key)
{
	return dataStore().value(key);
}

const String& DataStore::valueOrDefault(const String& key, const String& def)
{
	return dataStore().valueOrDefault(key, def);
}

std::vector<String> DataStore::availableKeys()
{
	return dataStore().getKeys();
}

void DataStore::erase(const String& key)
{
	dataStore().erase(key);
}

void DataStore::clear()
{
	dataStore().clear();
}

