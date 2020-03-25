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
	const String& valueOrDefault(const String& key, const String& def);
	std::vector<String> availableKeys();
	void erase(const String& key);
	void clear();

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

			const T& valueOrDefault(const S& key, const T& def)
			{
				if (not hasKey(key))
					return def;

				return value(key);
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

			void erase(const S& key)
			{
				data.erase(key);
			}

			void clear()
			{
				data.clear();
			}

		private:

			std::map<S, T> data;
	};
}

#endif /* DATASTORE_H_ */
