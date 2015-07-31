#ifndef TIMETEST_H
#define TIMETEST_H

#include "iscpxTest.h"

class TimeTest : public iscpx::Test {
	Q_OBJECT

private slots:

	//! Tests valid usage of constructor.
	void testConstructor();

	//! Tests constructing a time with a string.
	void testConstructorWithString();

	//! Tests adding zero milliseconds to a time.
	void testAddMSecsZero();

	//! Tests adding integral milliseconds to a time.
	void testAddMSecsInt();

	//! Tests adding floating point milliseconds to a time.
	void testAddMSecsFloat();
};

#endif // TIMETEST_H
