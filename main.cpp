#include <QApplication>
#include <QFontDatabase>
#include "SudokuWindow.h"

int main(int argc, char *argv[]) {


    QApplication app(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/Outfit-Light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Outfit-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Outfit-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Outfit-SemiBold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Outfit-Bold.ttf");

    QFont appFont("Outfit", 11);
    QApplication::setFont(appFont);

    SudokuWindow window;
    window.show();

    return app.exec();
}