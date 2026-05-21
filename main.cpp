#include <QApplication>
#include "SudokuWindow.h"

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
    QApplication app(argc, argv);

    SudokuWindow window;
    window.show();

    return app.exec();
}
