#include "PhotonMatch.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/PhotonMatch/Icon/photon-match-program-icon.ico"));
	PhotonMatch w;
	w.show();
	return a.exec();
}
