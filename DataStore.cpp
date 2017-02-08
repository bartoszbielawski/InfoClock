/*
 * DataStore.cpp
 *
 *  Created on: 05.02.2017
 *      Author: Bartosz Bielawski
 */

#include "DataStore.h"

static DataStore<String, String>* ds = nullptr;

DataStore<String, String>& dataStore()
{
	if (ds == nullptr)
		ds = new DataStore<String, String>;

	return *ds;
}
