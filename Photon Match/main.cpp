#include "PhotonMatch.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	PhotonMatch w;
	w.show();
	return a.exec();
}
