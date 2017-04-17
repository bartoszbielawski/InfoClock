/*
 * PathConstructor.h
 *
 *  Created on: 04.01.2017
 *      Author: caladan
 */

#ifndef PATHCONSTRUCTOR_H_
#define PATHCONSTRUCTOR_H_

#include <string>
#include <vector>

class PathConstructor {
	public:
		PathConstructor(uint32_t reserve = 64, char separator = '/');
		virtual ~PathConstructor();

		void push(const std::string& s);
		void push(const char* s);

		void pop();

		void clear();

		const std::string& getPath() const {return buffer;}
	private:
		std::string buffer;
		size_t size;
		std::vector<uint16_t> sizes;
		char separator;
};

#endif /* PATHCONSTRUCTOR_H_ */
