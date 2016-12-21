#include "computervisionsystem.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB18030"));

	ComputerVisionSystem w;
	w.show();
	return a.exec();
}
