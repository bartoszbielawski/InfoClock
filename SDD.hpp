/*
 * Author: Bartosz Bielawski
 */

#include <vector>
#include <string>
#include "pyfont.h"
// Scrolling Display Driver (SDD)
// Class for the state machine that handles the scrolling of the
// text on the screens

class LEDMatrixDriver;

class SDD
{
	public:
		SDD(LEDMatrixDriver &ledMatrixDriver);
		~SDD() {}

		bool tick();
		void renderString(const String &message, const PyFont& font);
		void refreshDisplay();

	private:
		std::vector<uint8_t> buffer;
		enum class STATE
		{
				START,
				MIDDLE,
				END
		};

		STATE state = STATE::START;

		LEDMatrixDriver &ledMatrixDriver;
		const static int columnIncrement = 1;
		size_t           startColumn = 0;

		const static int endDelay = 10;
		const static uint8_t intensity = 7;
		int              delayCounter = 0;
		int              physicalDisplayLen;
};
