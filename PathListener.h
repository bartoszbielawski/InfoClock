/*
 * PathListener.h
 *
 *  Created on: 04.01.2017
 *      Author: caladan
 */

#ifndef PATHLISTENER_H_
#define PATHLISTENER_H_

#include "AJSP.hpp"
#include "PathConstructor.hpp"

class PathListener: public AJSP::Listener {
	public:
		using Callback = void (*)(const std::string& path, const std::string& value);

		PathListener(AJSP::Parser* p): parser(p) {}
		virtual ~PathListener() = default;
		
		virtual void value(const std::string& value, AJSP::Parser::Entity entity);

		void setCallback(Callback callback) {this->callback = callback;}

		std::vector<const char*> &monitoredPaths() {return _monitoredPaths;}
	private:
		std::vector<const char*> _monitoredPaths;
		Callback callback = nullptr;
		AJSP::Parser* parser;
};

#endif /* PATHLISTENER_H_ */
