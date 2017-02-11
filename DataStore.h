/*
 *
 *  Created on: 04.02.2017
 *      Author: Bartosz Bielawski
 */


#ifndef DATASTORE_H_
#define DATASTORE_H_

#include <Arduino.h>
#include <vector>
#include <utility>
#include <map>

namespace DataStore
{
	String&	value(const String& key);
	bool	hasValue(const String& key);
	std::vector<String> availableKeys();

	template <class S, class T>
	class DataStore
	{
		public:
			DataStore() {}
			virtual ~DataStore() {}

			bool hasKey(const S& key)
			{
				return data.count(key) == 1;
			}

			T& value(const S& key)
			{
				return data[key];
			}

			std::vector<S> getKeys()
			{

				std::vector<S> keys;
				for (const auto& p: data)
				{
					keys.emplace_back(p.first);
				}

				return keys;
			}

		private:

			std::map<S, T> data;
	};
}

#endif /* DATASTORE_H_ */
