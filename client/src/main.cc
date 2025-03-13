#include <QApplication>
#include "mainwindow.hpp"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
int main(int argc, char* argv[]){
    QApplication app(argc, argv);
    QString qss_file_path = QCoreApplication::applicationDirPath() + "/stylesheet.qss";
    QFile qss_file(qss_file_path);
    qDebug() << qss_file_path;
    if (qss_file.open(QFile::ReadOnly)) {
       qDebug() << "open qss file sucess";
        QString style  = QLatin1String(qss_file.readAll());
        app.setStyleSheet(style);
        qss_file.close();
    }else {
        qDebug() << "oepn qss file failed";
    }
    MainWindow w;
    w.show();
    return app.exec();
}