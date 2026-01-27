/*
 * Author: Bartosz Bielawski
 */


#include <LEDMatrixDriver.hpp>
#include "SDD.hpp"
#include "config.h"
#include "utils.h"
using namespace std;

SDD::SDD(LEDMatrixDriver &ledMatrixDriver):
						buffer(ledMatrixDriver.getSegments() * 8),
						ledMatrixDriver(ledMatrixDriver),  
						physicalDisplayLen(ledMatrixDriver.getSegments() * 8)
{
	ledMatrixDriver.setEnabled(true);
}

bool SDD::tick()
{
	switch (state)
	{
		case STATE::START:
			if (!--delayCounter)
			{
				state = STATE::MIDDLE;
			}
			return false;

		case STATE::MIDDLE:
		{
			startColumn += columnIncrement;

			bool done = (buffer.size() - startColumn) < physicalDisplayLen;
			if (done)
			{
				startColumn = buffer.size() - physicalDisplayLen;
				state = STATE::END;
				delayCounter = endDelay;
			}
			refreshDisplay();
			return false;
		}

		case STATE::END:
			delayCounter--;
			if (delayCounter == 0)
			{
				state = STATE::START;
				delayCounter = endDelay;
				startColumn = 0;
				ledMatrixDriver.setEnabled(true);
				return true;
			}
			return false;
	}

	return true;
}


void SDD::renderString(const String &message, const PyFont& font)
{
	const char* cleanmessage = utf8ToLatin1(message.c_str());
	size_t len = calculateRenderedLength(font, cleanmessage);
	startColumn = 0;

	//here we make difference between a text that fits in the display and a text
	//that has to be scrolled
	if (len <= physicalDisplayLen)
	{
		buffer.resize(physicalDisplayLen);               //resize and zero

		for (auto& e: buffer)
			e = 0;

		int margin = (physicalDisplayLen - len + 1) / 2;    //calculate margin with rounding

		renderText(font, cleanmessage, buffer.data() + margin, len);

		state = STATE::END;
		delayCounter = endDelay;

		refreshDisplay();
		return;
	}

	//just change the size, the values will be initialized anyway when rendering
	buffer.resize(len);
	renderText(font, cleanmessage, buffer.data(), len);

	state = STATE::START;
	delayCounter = endDelay;

	refreshDisplay();
}

void SDD::refreshDisplay()
{
	for (uint32_t  i = 0; i < physicalDisplayLen; ++i)
	{
		ledMatrixDriver.setColumn(i, buffer[i+startColumn]);
	}

	for (int i = 0; i < LED_DISPLAYS; i++)
	{
		ledMatrixDriver.display();
	}
}
