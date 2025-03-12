#include "test.hpp"
#include <qt5/QtWidgets/QApplication>
int main(int argc, char* argv[]){
    QApplication app(argc, argv);
    TestW w;
    w.show();
    return app.exec();
}