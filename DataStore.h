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

template <class S, class T>
class DataStore
{
	public:
		DataStore() {}
		virtual ~DataStore() {}

		T& value(const S& key)
		{
			for (auto& e: data)
			{
				if (e.key == key)
				{
					return e.value;
				}
			}

			data.emplace_back(key, String());
			return data.back().value;
		}

		std::vector<S> getKeys()
		{
			std::vector<S> keys;
			for (auto& e: data)
			{
				keys.emplace_back(e.key);
			}
			return keys;
		}

	private:
		struct Element
		{
			Element(const S& key, const T& value): key(key), value(value) {}
			S key;
			T value;
		};

		std::vector<Element> data;
};

DataStore<String,String>& dataStore();

#endif /* DATASTORE_H_ */
