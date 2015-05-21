#include <QtTest>
#include "TimeGrid.h"

class TimeGridTest : public QObject
{
	Q_OBJECT
private slots:
	/*!
	 * Tests valid usage of constructor.
	 */
	void testConstructor();

	void testFailure();
};

