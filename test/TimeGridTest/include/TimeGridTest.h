#include <QtTest>
#include "iscpxTimeGrid.h"

class TimeGridTest : public QObject
{
	Q_OBJECT
private slots:

	//! Tests valid usage of constructor.
	void testConstructor();

	//! Tests getting the start time.
	void testGetStart();

	//! Test getting the length of the samples.
	void testGetLength();
};

