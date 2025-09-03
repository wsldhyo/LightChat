#include "client_globalvar.hpp"
#include "mainwindow.hpp"
#include <QApplication>
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
  g_gate_url_prefix = "http://" + gate_host + ":" + gate_port;
  qDebug() << "gate url prefix:" << g_gate_url_prefix;
}

int main(int argc, char *argv[]) {
  qDebug() << "All resources:" << QDir(":/").entryList();
  QApplication a(argc, argv);

  // 加载qss文件
  QFile qss(":/style/stylesheet.qss");
  if (qss.open(QFile::ReadOnly)) {
    qDebug("open qss success");
    QString style = QLatin1String(qss.readAll());
    a.setStyleSheet(style);
    qss.close();
  } else {
    qDebug("Open failed");
  }

  read_config();
  MainWindow w;
  w.show();

  return a.exec();
}