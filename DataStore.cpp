/*
 * DataStore.cpp
 *
 *  Created on: 05.02.2017
 *      Author: Bartosz Bielawski
 */

#include "DataStore.h"

DataStore<String, String>& dataStore()
{
	static DataStore<String, String> ds;
	return ds;
}
