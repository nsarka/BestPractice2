#include "MainWindow.h"

#include <QApplication>
#include <QFileInfo>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  QApplication::setApplicationName("BestPractice");
  QApplication::setOrganizationName("BestPractice");

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
