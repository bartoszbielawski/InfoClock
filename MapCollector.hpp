#ifndef MAPCOLLECTOR_H
#define MAPCOLLECTOR_H

/*
 * MapCollector.hpp
 *
 *  Created on: 18.04.2017
 *      Author: Bartosz Bielawski
 */

#include "AJSP.hpp"
#include "PathConstructor.hpp"
#include <functional>
#include <map>

template <class ... Types>
bool True(Types ... args)
{
	return true;
}

class MapCollector: private AJSP::Listener
{
    public:
		using Predicate = std::function<bool(const std::string& path, const std::string& value)>;

        MapCollector(Predicate p = True<std::string, std::string>);
        ~MapCollector();

        void reset();
        AJSP::Parser::Result parse(char c) {return _parser.parse(c);}
        bool done() const {return _parser.done();}

        std::map<std::string, std::string>& getValues() {return _values;}

    private:
        virtual void value(const std::string& value, AJSP::Parser::Entity entity);

        AJSP::Parser                		 _parser;
        std::map<std::string, std::string>   _values;
        Predicate							 _predicate;
};

#endif //MAPCOLLECTOR_H
