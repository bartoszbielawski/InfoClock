/*
 * AJSP.cpp
 *
 *  Created on: Jan 2, 2017
 *      Author: Bartosz Bielawski
 */

#include "AJSP.hpp"
#include <ctype.h>

#ifdef USE_ARDUINO
#include <Arduino.h>
#else
#include <iostream>
#include <unistd.h>
#endif

using namespace AJSP;
using namespace std;

#ifndef USE_ARDUINO
const map<AJSP::Parser::Entity, std::string> AJSP::Parser::entityNames =
{
		{Parser::Entity::OBJECT, "Object"},
		{Parser::Entity::ARRAY,  "Array"},
		{Parser::Entity::VALUE,  "Value"},
		{Parser::Entity::KEY,	 "Key"},
		{Parser::Entity::STRING, "String"},
		{Parser::Entity::RAW,	 "Raw"}
};
#endif


static std::string localToString(uint32_t v)
{
#ifdef USE_ARDUINO
	char buffer[12];
	snprintf(buffer, 12,  "%d", v);
	return std::string(buffer);
#else
	return to_string(v);
#endif
}

AJSP::Parser::Parser()
{
	stack.emplace_back(Entity::VALUE, State::NONE);
	stack.reserve(6);
	localBuffer.reserve(32);
}

void AJSP::Parser::setListener(Listener* l)
{
	listener = l;
}

void AJSP::Parser::reset()
{
	localBuffer.clear();
	pathConstructor.clear();

	lastKey = rootElementName;
	offset = 0;

	stack.clear();
	stack.emplace_back(Entity::VALUE, State::NONE);
}

bool AJSP::Parser::skipWhitespace(char c) const
{
	auto& currentElement = stack.back();
	return isspace(c) && !(currentElement.entity == Entity::STRING && currentElement.state == State::STRING_BODY);
}


AJSP::Parser::Result AJSP::Parser::parse(char c)
{
	if (!c)
		return Result::OK;

	if (skipWhitespace(c))
	{
		offset++;
		return Result::OK;
	}

	bool consumed = false;

	while ((not consumed) and (result == Result::OK) and (not stack.empty()))
	{
		switch (stack.back().entity)
		{
			case Entity::OBJECT:
				consumed = parseObject(c);
				break;

			case Entity::ARRAY:
				consumed = parseArray(c);
				break;

			case Entity::VALUE:
				consumed = parseValue(c);
				break;

			case Entity::STRING:
			case Entity::KEY:
				consumed = parseString(c);
				break;

			case Entity::RAW:
				consumed = parseRaw(c);
				break;
		}
	};

	if (consumed)
		offset++;

	if (stack.empty() && result == Result::OK)
	{
		if (listener) listener->done();
		reset();
		return Result::DONE;
	}

	return result;
}


//changes the VALUE entity from the top of the stack to the proper entity
bool AJSP::Parser::parseValue(char c)
{
	//here we don't push anything on the stack to it's safe to take the reference here
	auto& currentElement = stack.back();

	if (currentElement.entity != Entity::VALUE)
	{
		result = Result::INVALID_INTERNAL_STATE;
		return false;
	}

	if (c == '{')	//object - consumes the element
	{
		//NOTE: exit point
		pathConstructor.push(lastKey);
		if (listener) listener->objectStart();
		
		currentElement = StackElement(Entity::OBJECT, State::OBJECT_KEY_OR_END);
		return true;
	}

	if (c == '[')	//array - consumes the char
	{
		//NOTE: exit point
		pathConstructor.push(lastKey);
		if (listener) listener->arrayStart();

		currentElement = StackElement(Entity::ARRAY, State::ARRAY_VALUE_OR_END);
		return true;
	}

	if ((c == 'u') or (c == '\"') or (c == '\''))	//string
	{
		currentElement = StackElement(Entity::STRING, State::STRING_START);
		return parseString(c);
	}

	if (checkRawChar(c))
	{
		//let's see if we can handle this one with RAW entity (bool, null, numbers)
		currentElement = StackElement(Entity::RAW, State::NONE);
		localBuffer.clear();
		return parseRaw(c);
	}

	//failed to recognize character
	stack.pop_back();
	return false;
}

bool AJSP::Parser::parseString(char c)
{
	auto& currentElement = stack.back();	//no stack allocation

	switch (currentElement.state)
	{
		case State::STRING_START:
			//we should skip 'u' that is at the beginning - u for unicode
			if (c == 'u')
				return true;
			if ((c == '\"') or (c == '\''))
			{
				currentElement.state = State::STRING_BODY;	//we're in the string
				localBuffer.clear();
				return true;
			}

			result = Result::IC_STRING_START_EXPECTED;
			return false;

		case State::STRING_BODY:
			if ((c == '\"') or (c == '\''))		//end of string
			{
				//NOTE: exit point
				bool isKey = currentElement.entity == Entity::KEY;

				if (isKey)
				{
					lastKey = localBuffer;
					if (listener) listener->key(localBuffer);
				}
				else
				{	
					if (listener) 
					{
						pathConstructor.push(lastKey);
						listener->value(localBuffer, Entity::STRING);
						pathConstructor.pop();
					}
				}

				stack.pop_back();
				return true;
			}

			if (c == '\\')
			{
				currentElement.state = State::STRING_ESCAPE;
				return true;
			}

			localBuffer += c;
			return true;

		case State::STRING_ESCAPE:
			switch (c)
			{
				case 'n': localBuffer += '\n'; break;
				case 'r': localBuffer += '\r'; break;
				case 't': localBuffer += '\t'; break;
				case '\\': localBuffer += '\\'; break;

				default:
					localBuffer += c;		//just put the raw value
			}

			currentElement.state = State::STRING_BODY;
			return true;

				default:;
	}

	result = Result::INVALID_INTERNAL_STATE;
	return false;
}

bool AJSP::Parser::parseArray(char c)
{
	switch(stack.back().state)
	{
		case State::ARRAY_VALUE_OR_END:
			lastKey = "0";
			if (c == ']')
			{
				//NOTE: exit point
				if (listener)
					listener->arrayEnd();
				
				pathConstructor.pop();
				stack.pop_back();
				return true;
			}

			stack.back().state = State::ARRAY_SEPARATOR_OR_END;
			stack.emplace_back(Entity::VALUE, State::NONE);
			if (parseValue(c))
				return true;

			result = Result::IC_ARRAY_VALUE_OR_END_EXPECTED;
			return false;

		case State::ARRAY_VALUE:
			stack.back().state = State::ARRAY_SEPARATOR_OR_END;
			stack.emplace_back(Entity::VALUE, State::NONE);
			if (parseValue(c))
				return true;

			result = Result::IC_ARRAY_VALUE_EXPECTED;
			return false;

		case State::ARRAY_SEPARATOR_OR_END:
			if (c == ']')
			{
				//NOTE: exit point
				if (listener)
					listener->arrayEnd();

				pathConstructor.pop();
				stack.pop_back();
				return true;
			}

			if (c == ',')
			{
				stack.back().state = State::ARRAY_VALUE;
				lastKey = localToString(++stack.back().counter);
				return true;
			}

			result = Result::IC_ARRAY_COMMA_OR_END_EXPECTED;
			return false;
		default:;
	}

	result = Result::INVALID_INTERNAL_STATE;
	return false;
}


bool		AJSP::Parser::parseObject(char c)
{
	switch (stack.back().state)
	{
		case State::OBJECT_KEY_OR_END:
			if (c == '}')
			{
				//NOTE: exit point
				if (listener)
					listener->objectEnd();

				pathConstructor.pop();
				stack.pop_back();
				return true;
			}

			//the next thing we're expecting on this stack level
			//is a colon (after the string is done)
			stack.back().state = State::OBJECT_COLON;

			//try parsing it as a string
			{
				stack.emplace_back(Entity::KEY, State::STRING_START);
				bool consumed = parseString(c);

				if (!consumed and result == Result::IC_STRING_START_EXPECTED)
				{
					result = Result::IC_OBJECT_KEY_OR_END_EXPECTED;
				}
				return consumed;
			}

		case State::OBJECT_COLON:
			//here we only expect K and V separator
			if (c == ':')
			{
				stack.back().state = State::OBJECT_VALUE;
				return true;
			}

			result = Result::IC_OBJECT_COLON_EXPECTED;
			return false;

		case State::OBJECT_VALUE:
			stack.back().state = State::OBJECT_SEPARATOR_OR_END;
			stack.emplace_back(Entity::VALUE, State::NONE);
			if (parseValue(c))
			{
				return true;
			}

			result = Result::IC_OBJECT_VALUE_EXPECTED;
			return false;


		case State::OBJECT_SEPARATOR_OR_END:
			if (c == '}')
			{
				//NOTE: exit point
				if (listener)
					listener->objectEnd();

				pathConstructor.pop();
				stack.pop_back();
				return true;
			}

			if (c == ',')
			{
				stack.back().state = State::OBJECT_COLON;
				stack.emplace_back(Entity::KEY, State::STRING_START);
				return true;
			}

			result = Result::IC_OBJECT_SEPARATOR_OR_END_EXPECTED;
			return false;

		default:;
	}

	result = Result::INVALID_CHARACTER;
	return false;
}

bool 		AJSP::Parser::checkRawChar(char c)
{
	return isalnum(c) or c == '+' or c == '-' or c == '.';
}

bool		AJSP::Parser::parseRaw(char c)
{
	/*
	 * FIXME:
	 * currently the code will accept any input that consists of these characters
	 * it could be fixed to be able to invalid sequences:
	 * -multiple dots
	 * -multiple exponents
	 * -+- signs not at the beginning or not after e/E
	 * -... and probably many more
	 *
	 * And it doesn't support unicode escapes...
	 */

	if (checkRawChar(c))
	{
		localBuffer += c;
		return true;
	}

	//if we already had something in the buffer then it's the end of the token
	if (localBuffer.length())
	{
		//NOTE: exit point
		if (listener)
		{
			pathConstructor.push(lastKey);
			listener->value(localBuffer, Entity::RAW);
			pathConstructor.pop();
		}
		
		localBuffer.clear();
	}

	stack.pop_back();
	return false;
}


#ifndef USE_ARDUINO
void 	  AJSP::Parser::printState(const std::string& msg) const
{
	cout << "=================  " << msg << "  ==============" << endl;
	cout << "StackSize:   " << stack.size() << endl;
	cout << "Top element: " << entityNames.at(stack.back().entity) << endl;
	cout << "Offset:      " << offset << endl;
	cout << "Result:      " << getResultDescription(result) << endl;
	cout << "State:       " << int(result) << endl;
	printStack();
}

void AJSP::Parser::printStack() const
{
	for (const auto& se: stack)
	{
		cout << entityNames.at(se.entity) << "\t\t, " << getStateDescription(se.state) << endl;
	}
}
#endif

const char* AJSP::Parser::getStateDescription(State s)
{
	switch(s)
	{
		case State::NONE: return "NONE";
		case State::OBJECT_KEY_OR_END: return "OBJECT_KEY_OR_END";
		case State::OBJECT_COLON: return "OBJECT_COLON";
		case State::OBJECT_VALUE: return "OBJECT_VALUE";
		case State::OBJECT_SEPARATOR_OR_END: return "OBJECT_SEPARATOR_OR_END";
		case State::ARRAY_VALUE_OR_END: return "ARRAY_VALUE_OR_END";
		case State::ARRAY_SEPARATOR_OR_END: return "ARRAY_SEPARATOR_OR_END";
		case State::ARRAY_VALUE: return "ARRAY_VALUE";
		case State::STRING_START: return "STRING_START";
		case State::STRING_BODY: return "STRING_BODY";
		case State::STRING_ESCAPE: return "STRING_ESCAPE";
		case State::INVALID: return "INVALID";
		default:;
	}
	return "Unknown";
}

const char* AJSP::Parser::getResultDescription(Result r)
{
	switch (r)
	{
		case Result::OK: 	return "OK";
		case Result::DONE: 	return "Done";
		case Result::INVALID_CHARACTER: 			return "Invalid character";
		case Result::IC_STRING_START_EXPECTED:		return "String start expected";
		case Result::IC_ARRAY_COMMA_OR_END_EXPECTED:return "Array separator or end brace expected";
		case Result::IC_ARRAY_VALUE_OR_END_EXPECTED:return "Value or end brace expected";
		case Result::IC_ARRAY_VALUE_EXPECTED:		return "Value expected";
		case Result::IC_OBJECT_COLON_EXPECTED:		return "Colon expected";
		case Result::IC_OBJECT_VALUE_EXPECTED:		return "Value expected";
		case Result::IC_OBJECT_KEY_OR_END_EXPECTED:	return "Key or end brace expected";
		case Result::IC_OBJECT_SEPARATOR_OR_END_EXPECTED: 			return "Comma or end brace expected";
		case Result::INVALID_INTERNAL_STATE:		return "Invalid internal state";
	}

	return "Unknown";
}
