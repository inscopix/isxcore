#include "iscpxTest.h"

class TimeGridTest : public iscpx::Test {
	Q_OBJECT

private slots:

	//! Tests valid usage of constructor.
	void testConstructor();

	//! Tests getting the start time.
	void testGetStart();

	//! Test getting the length of the samples.
	void testGetLength();
};

