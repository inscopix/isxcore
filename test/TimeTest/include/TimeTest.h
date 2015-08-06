#ifndef TIMETEST_H
#define TIMETEST_H

#include "isxTest.h"

class TimeTest : public isx::Test {
	Q_OBJECT

private slots:

	//! Tests valid usage of constructor.
	void testConstructor();

	//! Tests constructing a time with a string.
	void testConstructorWithString();

	//! Tests adding zero seconds to a time.
	void testAddSecsZero();

	//! Tests adding integral seconds to a time.
	void testAddSecsInt();

	//! Tests adding floating point seconds to a time.
	void testAddSecsFloat();
};

#endif // TIMETEST_H
