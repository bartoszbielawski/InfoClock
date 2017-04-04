/*
 * LEDMatrixDriver.h
 *
 *  Created on: 30.03.2017
 *      Author: caladan
 */

#ifndef LEDMATRIXDRIVER_H_
#define LEDMATRIXDRIVER_H_

#include <SPI.h>
#include <vector>

class LEDMatrixDriver
{
	const static uint16_t ENABLE =		0x0C00;
	const static uint16_t TEST =	 	0x0F00;
	const static uint16_t INTENSITY =	0x0A00;
	const static uint16_t SCAN_LIMIT =	0x0B00;
	const static uint16_t DECODE =		0x0900;

	public:
		LEDMatrixDriver(uint8_t N, uint8_t ssPin);
		virtual ~LEDMatrixDriver();

		void setEnabled(bool enabled);
		void setIntensity(uint8_t level);
		void setPixel(uint16_t x, uint16_t y, bool enabled);
		void setColumn(uint16_t x, uint8_t value);
		uint8_t getSegments() const {return N;}

		void display();
		void displayRow(uint8_t row) {_displayRow(row);}
	private:
		void _sendCommand(uint16_t command);
		void _displayRow(uint8_t row);

		const uint8_t N;
		SPISettings spiSettings;
		std::vector<uint8_t> frameBuffer;
		std::vector<uint16_t> commandBuffer;
		uint8_t ssPin;
};

#endif /* LEDMATRIXDRIVER_H_ */
