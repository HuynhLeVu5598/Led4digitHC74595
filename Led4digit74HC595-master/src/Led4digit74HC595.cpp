/*
  Led4digit74HC595.cpp - Library
*/

#include "Led4digit74HC595.h"


unsigned char _LED_CHAR[14];

Led4digit74HC595::Led4digit74HC595(int SCLK, int RCLK, int DIO)
{
	pinMode(SCLK, OUTPUT);
	pinMode(RCLK, OUTPUT);
	pinMode(DIO, OUTPUT);

	_SCLK = SCLK;
	_RCLK = RCLK;
	_DIO = DIO;

	_dp = 0;
	_char[0] = 10;
	_char[1] = 10;
	_char[2] = 10;
	_char[3] = 10;

	// Online generator for example: http://www.uize.com/examples/seven-segment-display.html
	_LED_CHAR[0] = 0x3F; //0
	_LED_CHAR[1] = 0x06; //1
	_LED_CHAR[2] = 0x5B; //2
	_LED_CHAR[3] = 0x4F; //3
	_LED_CHAR[4] = 0x66; //4
	_LED_CHAR[5] = 0x6D; //5
	_LED_CHAR[6] = 0x7D; //6
	_LED_CHAR[7] = 0x07; //7
	_LED_CHAR[8] = 0x7F; //8
	_LED_CHAR[9] = 0x6F; //9
	_LED_CHAR[10] = 0x00; // nothing
	_LED_CHAR[11] = 0x40; //-
	_LED_CHAR[12] = 0x08; //_
	_LED_CHAR[13] = 0x01; // upper -

	sleep();	//display dark
}



// Set number to be show in next step
void Led4digit74HC595::setNumber(int _n)
{
	if (_n > 9999) {
		for (char c = 0; c < 4; c++) {
			_char[c] = 13;	//13=index for upper - char
		}
		return;
	}
	if (_n < -999) {
		for (char c = 0; c < 4; c++) {
			_char[c] = 12;	//12=index for _ char
		}
		return;
	}

	int absNumber = abs(_n);
	int one, ten, hundred, thousand;
	one = (int)absNumber % 10;
	ten = (int)((absNumber % 100) - one) / 10;
	hundred = (int)((absNumber % 1000) - ten - one) / 100;
	thousand = (int)((absNumber % 10000) - hundred - ten - one) / 1000;

	_char[0] = _LED_CHAR[thousand];
	_char[1] = _LED_CHAR[hundred];
	_char[2] = _LED_CHAR[ten];
	_char[3] = _LED_CHAR[one];

	//show "nothing" on left leading positions when needed
	if (thousand == 0) {
		_char[0] = _LED_CHAR[10];			//10=index for nothing led segments
		if (hundred == 0) {
			_char[1] = _LED_CHAR[10];
			if (ten == 0) {
				_char[1] = _LED_CHAR[10];
			}
		}
	}

	//if negative number = show minus 
	if (_n < 0) {
		if (absNumber > 99) {
			_char[0] = _LED_CHAR[11];	//11=index for minus char -
		}
		else if (absNumber > 9) {
			_char[1] = _LED_CHAR[11];
		}
		else if (absNumber > 0) {
			_char[2] = _LED_CHAR[11];
		}
	}


	if (_dp == 1) _char[3] = _char[3] | B10000000;	//add decimal point
	if (_dp == 2) _char[2] = _char[2] | B10000000;
	if (_dp == 3) _char[1] = _char[1] | B10000000;
	if (_dp == 4) _char[0] = _char[0] | B10000000;
}



// Periodic call action in main loop of program
void Led4digit74HC595::loopShow()
{

	byte port = B0001 << _showPos;
	//writeByte(~_LED_CHAR[_char[_showPos]]);
	writeByte(~_char[_showPos]);	//Led bits for module must be negated (~)
	writeByte(port);
	digitalWrite(_RCLK, HIGH);
	digitalWrite(_RCLK, LOW);


	//move actual digit position
	if (_showPos < 3)
		_showPos++;
	else
		_showPos = 0;
}


void Led4digit74HC595::writeByte(unsigned char X)
{
	for (char i = 0; i < 8; i++)
	{
		//select left one bit and check if is 1 or 0
		if (X & B10000000) {
			digitalWrite(_DIO, LOW);
		}
		else {
			digitalWrite(_DIO, HIGH);
		}
		X <<= 1;
		digitalWrite(_SCLK, HIGH);
		digitalWrite(_SCLK, LOW);
	}
}


// Position of decimal point (1 - 4) or disable (0)
void Led4digit74HC595::setDecimalPoint(unsigned char position)
{
	if (position > 4) position = 4;
	_dp = position;
	for (char c = 0; c < 4; c++) {
		_char[c] = _char[c] & B01111111;	//remove decimal point
	}
	if (_dp == 1) _char[3] = _char[3] | B10000000;	//add decimal point
	if (_dp == 2) _char[2] = _char[2] | B10000000;
	if (_dp == 3) _char[1] = _char[1] | B10000000;
	if (_dp == 4) _char[0] = _char[0] | B10000000;
}


// All characters temporary off when program processing another long time function in multiplexing display characters
void Led4digit74HC595::sleep()
{
	char c;

	digitalWrite(_DIO, LOW);	//segments off
	for (c = 0; c < 8; c++)
	{
		digitalWrite(_SCLK, HIGH);
		digitalWrite(_SCLK, LOW);
	}

	digitalWrite(_DIO, HIGH);	//positions
	for (c = 0; c < 8; c++)
	{
		digitalWrite(_SCLK, HIGH);
		digitalWrite(_SCLK, LOW);
	}

	digitalWrite(_RCLK, HIGH);	//write and active
	digitalWrite(_RCLK, LOW);


	// temporary shot decimal point while display is darking
	digitalWrite(_DIO, HIGH);	// DP segment, active = HIGH
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);
	for (char i = 0; i < 7; i++) {
		digitalWrite(_DIO, LOW);
		digitalWrite(_SCLK, HIGH);
		digitalWrite(_SCLK, LOW);
	}
	for (char i = 0; i < 4; i++) {	// non exist first 4 positions
		digitalWrite(_DIO, HIGH);
		digitalWrite(_SCLK, HIGH);
		digitalWrite(_SCLK, LOW);
	}

	if (_dp == 4) {
		digitalWrite(_DIO, LOW);	// 1 position = most left, active = LOW
	}
	else {
		digitalWrite(_DIO, HIGH);
	}
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	if (_dp == 3) {
		digitalWrite(_DIO, LOW);	// 1 position = most left, active = LOW
	}
	else {
		digitalWrite(_DIO, HIGH);
	}
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	if (_dp == 2) {
		digitalWrite(_DIO, LOW);	// 1 position = most left, active = LOW
	}
	else {
		digitalWrite(_DIO, HIGH);
	}
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	if (_dp < 2) {
		digitalWrite(_DIO, LOW);	// 1 position = most left, active = LOW
	}
	else {
		digitalWrite(_DIO, HIGH);
	}
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	//--------------- write and show
	digitalWrite(_RCLK, HIGH);
	digitalWrite(_RCLK, LOW);


}


/* Test function for basic education
void Led4digit74HC595::test()
{
	// first byte (bit 1 - 8) = active segments when HIGH
	// second byte (bit 9 -16) = active position when LOW

	digitalWrite(_DIO, HIGH);	// DP segment, active = HIGH
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	// G segment
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	// F segment
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, LOW);	// E segment
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, LOW);	// D segment
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, LOW);	// C segment
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	// B segment
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	// A segment
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	//-------
	digitalWrite(_DIO, HIGH);	//
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	//
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	//
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	//
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, LOW);	// 1 position = most left, active = LOW
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	// 2 position
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, HIGH);	// 3 position
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	digitalWrite(_DIO, LOW);	// 4 position = most right
	digitalWrite(_SCLK, HIGH);
	digitalWrite(_SCLK, LOW);

	//--------------- write and show
	digitalWrite(_RCLK, HIGH);
	digitalWrite(_RCLK, LOW);
}
*/