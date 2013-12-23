#include <QtGui/QApplication>
#include "view.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Game game;
    View w;
    w.setGame(&game);
    w.show();
    
    return a.exec();
}
