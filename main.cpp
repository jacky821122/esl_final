#include "widget2.h"
#include "detectcolor.h"
#include <iostream>
#include <cstdio>
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	widget2 *widget = new widget2();
	widget->show();
	return app.exec();
}
