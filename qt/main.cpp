#include "MainWindow.h"

#include <QApplication>
#include <QFileInfo>
#include <QFile>
#include <QIcon>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  QApplication::setApplicationName("BestPractice");
  QApplication::setOrganizationName("BestPractice");
  QApplication::setWindowIcon(QIcon(":/app.ico"));
  QFile theme(":/theme.qss");
  if (theme.open(QIODevice::ReadOnly)) {
    app.setStyleSheet(QString::fromUtf8(theme.readAll()));
  }

  MainWindow window;
  window.show();

  const QStringList arguments = QApplication::arguments();
  if (arguments.size() > 1) {
    const QFileInfo audioFile(arguments.at(1));
    if (audioFile.isFile()) {
      window.openAudioFile(audioFile.absoluteFilePath());
    }
  }

  return QApplication::exec();
}
