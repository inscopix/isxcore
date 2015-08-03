#ifndef POINTTEST_H
#define POINTTEST_H

#include "isxTest.h"

class PointTest : public isx::Test {
	Q_OBJECT

private slots:

	//! Tests valid usage of constructor.
	void testConstructor();

	//! Tests constructing a time with a string.
	void testConstructorWithCoords();

	//! Tests adding another point.
	void testPlusPoint();

	//! Tests subtracting another point.
	void testMinusPoint();
};

#endif // POINTTEST_H
