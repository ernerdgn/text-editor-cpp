#include "mainwindow.h"

#include <QApplication>
//
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //--//
    QStringList args = QApplication::arguments();
    if (args.size() > 1) w.loadFile(args[1]);
    else w.loadFile("");
    //--//

    return a.exec();
}
