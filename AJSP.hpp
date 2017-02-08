/*
 * AJSP.hpp
 *
 *  Created on: Jan 2, 2017
 *      Author: Bartosz Bielawski
 *
 */

#ifndef AJSP_HPP_
#define AJSP_HPP_

#include <string>
#include <stack>
#include <utility>
#include <vector>

#define USE_ARDUINO

#ifndef USE_ARDUINO
#include <map>
#endif

namespace AJSP
{
	class Listener;

	class Parser
	{
		public:
			Parser();
			~Parser() {}

			void reset();

			enum class Result: uint8_t;

			Result parse(char c);		//returns true when it's done
			void setListener(Listener* l);
			uint32_t getCurrentOffset() const {return offset;}
			const std::string& getLastKey() const {return lastKey;}

			bool done() {return stack.empty();}

			enum class Result: uint8_t
			{
				OK,
				DONE,
				INVALID_CHARACTER = 0x10,		//generic
				IC_STRING_START_EXPECTED,
				IC_ARRAY_COMMA_OR_END_EXPECTED,
				IC_ARRAY_VALUE_OR_END_EXPECTED,
				IC_ARRAY_VALUE_EXPECTED,
				IC_OBJECT_COLON_EXPECTED,
				IC_OBJECT_VALUE_EXPECTED,
				IC_OBJECT_KEY_OR_END_EXPECTED,
				IC_OBJECT_SEPARATOR_OR_END_EXPECTED,
				INVALID_INTERNAL_STATE = 0x80
			};

			static const char* getResultDescription(Result r);

			enum class Entity: uint8_t
			{
				VALUE,
				OBJECT,
				ARRAY,
				STRING,
				KEY,
				RAW,
			};
			Result getLastResult() const {return result;}

		private:
			enum class State: uint8_t
			{
				NONE = 0,		//for anything that doesn't need state

				OBJECT_KEY_OR_END = 0x10,
				OBJECT_COLON,
				OBJECT_VALUE,
				OBJECT_SEPARATOR_OR_END,

				ARRAY_VALUE_OR_END = 0x20,
				ARRAY_SEPARATOR_OR_END,
				ARRAY_VALUE,

				STRING_START = 0x30,	//for strings and keys
				STRING_BODY,
				STRING_ESCAPE,

				INVALID = 0xFF
			};

			struct StackElement
			{
					StackElement(Entity e, State s): entity(e), state(s) {}
					Entity entity;
					State state;
					uint16_t counter = 0;
			};

			bool 		skipWhitespace(char c) const;

			bool 		parseValue(char c);
			bool		parseString(char c);
			bool		parseObject(char c);
			bool		parseArray(char c);
			bool		parseRaw(char c);

			bool 		checkRawChar(char c);

			Listener* 	listener = nullptr;

			std::string localBuffer;

			constexpr static const char* rootElementName = "root";

			std::string lastKey = rootElementName;
			uint32_t	offset = 0;
			Result   	result = Result::OK;

			std::vector<StackElement> stack;

			static const char* getStateDescription(State s);

#ifndef USE_ARDUINO
			void 	  printState(const std::string& msg) const;
			void 	  printStack() const;
			static const std::map<Entity, std::string> entityNames;
#endif
	};

	class Listener
	{
		public:
			Listener(Parser* p): parser(p) {}
			virtual ~Listener() {}

			virtual void arrayStart() = 0;
			virtual void arrayEnd() = 0;

			virtual void objectStart() = 0;
			virtual void objectEnd() = 0;

			virtual void key(const std::string& key) = 0;
			virtual void value(const std::string& value, Parser::Entity entity) = 0;

			virtual void done() = 0;
		protected:
			Parser* parser;
	};


}


#endif /* AJSP_HPP_ */
