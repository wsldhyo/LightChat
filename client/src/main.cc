#include "../common/constant.hpp"
#include "mainwindow.hpp"
#include "global.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
void read_config() {
  QString file = "config.ini";
  QString config_path = QDir::toNativeSeparators(
      QCoreApplication::applicationDirPath() + QDir::separator() + file);
  QSettings settings(config_path, QSettings::IniFormat);
  QString gate_host = settings.value("GateServer/host").toString();
  QString gate_port = settings.value("GateServer/port").toString();
  gate_url_prefix = "http://" + gate_host + ":" + gate_port;
  qDebug() << "gate url prefix:" << gate_url_prefix;
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  qRegisterMetaType<RequestID>("RequestID");
  qRegisterMetaType<Modules>("Modules");
  qRegisterMetaType<ErrorCode>("ErrorCode");
  QString qss_file_path =
      QDir::toNativeSeparators(QCoreApplication::applicationDirPath() +
                               QDir::separator() + "stylesheet.qss");
  QFile qss_file(qss_file_path);
  qDebug() << qss_file_path;
  if (qss_file.open(QFile::ReadOnly)) {
    qDebug() << "open qss file sucess";
    QString style = QLatin1String(qss_file.readAll());
    app.setStyleSheet(style);
    qss_file.close();
  } else {
    qDebug() << "oepn qss file failed";
  }
  read_config();
  MainWindow w;
  w.show();
  return app.exec();
}