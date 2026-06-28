#ifndef BESTPRACTICE_QT_MAINWINDOW_H
#define BESTPRACTICE_QT_MAINWINDOW_H

#include <QMainWindow>
#include <QStyle>

class QCheckBox;
class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QSlider;
class QSpinBox;
class QTableWidget;
class QToolButton;
class QString;
class QWidget;
class AudioEngine;

class MainWindow : public QMainWindow
{
public:
  explicit MainWindow(QWidget* parent = nullptr);
  void openAudioFile(const QString& path);

private:
  QWidget* createDiscPanel();
  QWidget* createTransportPanel();
  QWidget* createLoopPanel();
  QWidget* createProcessingTabs();
  QWidget* createMessagesPanel();
  QToolButton* makeToolButton(const QString& tooltip, QStyle::StandardPixmap icon);

  void connectUi();
  void addMessage(const QString& message);
  void updateSpeedLabel(int value);
  void updatePitchLabel(int value);
  void updateFinePitchLabel(int value);
  void updateBassLabel(int value);
  void updateHighLabel(int value);
  void updatePlaybackPosition(qint64 position);
  void updatePlaybackDuration(qint64 duration);
  void updatePlaybackState();
  void setLoopTime(bool start, qint64 milliseconds);
  qint64 loopTime(bool start) const;
  static QString formatTime(qint64 milliseconds);
  void showBackendPlaceholder(const QString& action);

  QComboBox* driveList_ = nullptr;
  QLabel* discTitleLabel_ = nullptr;
  QTableWidget* trackList_ = nullptr;
  QTableWidget* messages_ = nullptr;

  QPushButton* refreshCdButton_ = nullptr;
  QPushButton* openFileButton_ = nullptr;
  QPushButton* saveFileButton_ = nullptr;
  QPushButton* aboutButton_ = nullptr;
  QPushButton* helpButton_ = nullptr;
  QPushButton* languageButton_ = nullptr;
  QCheckBox* checkCddb_ = nullptr;

  QToolButton* selectTrackButton_ = nullptr;
  QToolButton* pauseButton_ = nullptr;
  QToolButton* previousButton_ = nullptr;
  QToolButton* nextButton_ = nullptr;

  AudioEngine* audioEngine_ = nullptr;
  QLabel* timeLabel_ = nullptr;
  QSlider* cueSlider_ = nullptr;
  QSlider* volumeSlider_ = nullptr;

  QWidget* loopPanel_ = nullptr;
  QCheckBox* loopCheck_ = nullptr;
  QPushButton* loopStartButton_ = nullptr;
  QPushButton* loopEndButton_ = nullptr;
  QSpinBox* loopStartMin_ = nullptr;
  QSpinBox* loopStartSec_ = nullptr;
  QSpinBox* loopStartFrame_ = nullptr;
  QSpinBox* loopEndMin_ = nullptr;
  QSpinBox* loopEndSec_ = nullptr;
  QSpinBox* loopEndFrame_ = nullptr;

  QLabel* speedLabel_ = nullptr;
  QLabel* pitchLabel_ = nullptr;
  QLabel* finePitchLabel_ = nullptr;
  QSlider* speedSlider_ = nullptr;
  QSlider* pitchSlider_ = nullptr;
  QSlider* finePitchSlider_ = nullptr;
  QCheckBox* filterCheck_ = nullptr;
  QComboBox* qualityCombo_ = nullptr;

  QCheckBox* karaokeCheck_ = nullptr;
  QSlider* vocalPositionSlider_ = nullptr;
  QSlider* bassSlider_ = nullptr;
  QSlider* highSlider_ = nullptr;
  QLabel* bassLabel_ = nullptr;
  QLabel* highLabel_ = nullptr;
};

#endif
