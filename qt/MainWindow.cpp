#include "MainWindow.h"

#include "AboutDialog.h"
#include "AudioEngine.h"
#include "LanguageDialog.h"
#include "ProgressDialog.h"
#include "version.h"

#include <QAbstractItemView>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QTabWidget>
#include <QTableWidget>
#include <QTime>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <array>

namespace
{
class SeekSlider final : public QSlider
{
public:
  explicit SeekSlider(QWidget* parent = nullptr)
    : QSlider(Qt::Horizontal, parent)
  {
  }

protected:
  void mousePressEvent(QMouseEvent* event) override
  {
    if (event->button() == Qt::LeftButton) {
      QStyleOptionSlider option;
      initStyleOption(&option);
      const QRect groove = style()->subControlRect(
        QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, this);
      const QRect handle = style()->subControlRect(
        QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, this);
      const int sliderMin = groove.left();
      const int sliderMax = groove.right() - handle.width() + 1;
      const int clickPosition = static_cast<int>(event->position().x()) - handle.width() / 2;
      const int clickedValue = QStyle::sliderValueFromPosition(
        minimum(), maximum(), clickPosition - sliderMin,
        std::max(1, sliderMax - sliderMin), option.upsideDown);
      setValue(clickedValue);
      emit sliderMoved(clickedValue);
    }
    QSlider::mousePressEvent(event);
  }
};

QLabel* sectionLabel(const QString& text)
{
  auto* label = new QLabel(text);
  QFont font = label->font();
  font.setBold(true);
  label->setFont(font);
  return label;
}

void setTableHeaders(QTableWidget* table, const QStringList& headers)
{
  table->setColumnCount(headers.size());
  table->setHorizontalHeaderLabels(headers);
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->hide();
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QSlider* horizontalSlider(int minimum, int maximum, int value)
{
  auto* slider = new QSlider(Qt::Horizontal);
  slider->setRange(minimum, maximum);
  slider->setValue(value);
  slider->setTracking(true);
  return slider;
}
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setWindowTitle(QStringLiteral("BestPractice ") + BP_VERSION);
  resize(940, 620);

  audioEngine_ = new AudioEngine(this);

  auto* root = new QWidget(this);
  auto* layout = new QGridLayout(root);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setHorizontalSpacing(12);
  layout->setVerticalSpacing(10);

  layout->addWidget(createDiscPanel(), 0, 0, 4, 1);
  layout->addWidget(createTransportPanel(), 4, 0);
  layout->addWidget(createMessagesPanel(), 5, 0);
  layout->addWidget(createLoopPanel(), 0, 1);
  layout->addWidget(createProcessingTabs(), 2, 1, 4, 1);

  layout->setColumnStretch(0, 5);
  layout->setColumnStretch(1, 4);
  layout->setRowStretch(3, 1);
  layout->setRowStretch(5, 1);

  setCentralWidget(root);
  statusBar()->showMessage("Ready");

  addMessage("Qt DSP playback engine initialized.");
  connectUi();
}

QWidget* MainWindow::createDiscPanel()
{
  auto* panel = new QWidget;
  auto* layout = new QVBoxLayout(panel);
  layout->setContentsMargins(0, 0, 0, 0);

  layout->addWidget(sectionLabel("Disc"));

  auto* driveRow = new QHBoxLayout;
  driveList_ = new QComboBox;
  driveList_->addItem("No CD drive detected");
  driveRow->addWidget(new QLabel("Drive"));
  driveRow->addWidget(driveList_, 1);
  layout->addLayout(driveRow);

  discTitleLabel_ = new QLabel("Disc title: ");
  layout->addWidget(discTitleLabel_);

  trackList_ = new QTableWidget(0, 2);
  setTableHeaders(trackList_, {"Track #", "Track Title"});
  trackList_->setContextMenuPolicy(Qt::ActionsContextMenu);
  auto* playTrackAction = trackList_->addAction("Play CD track");
  auto* refreshAction = trackList_->addAction("Refresh CD");
  connect(playTrackAction, &QAction::triggered, this, [this] { showBackendPlaceholder("Play CD track"); });
  connect(refreshAction, &QAction::triggered, this, [this] { showBackendPlaceholder("Refresh CD"); });
  layout->addWidget(trackList_, 1);

  auto* cdOptions = new QHBoxLayout;
  refreshCdButton_ = new QPushButton("Refresh CD");
  checkCddb_ = new QCheckBox("Check CDDB");
  checkCddb_->setChecked(true);
  cdOptions->addWidget(refreshCdButton_);
  cdOptions->addWidget(checkCddb_);
  layout->addLayout(cdOptions);

  return panel;
}

QWidget* MainWindow::createTransportPanel()
{
  auto* panel = new QWidget;
  auto* layout = new QVBoxLayout(panel);
  layout->setContentsMargins(0, 0, 0, 0);

  auto* transportRow = new QHBoxLayout;
  selectTrackButton_ = makeToolButton("Play selected CD track", QStyle::SP_DriveCDIcon);
  pauseButton_ = makeToolButton("Play or pause", QStyle::SP_MediaPause);
  previousButton_ = makeToolButton("Previous track", QStyle::SP_MediaSkipBackward);
  nextButton_ = makeToolButton("Next track", QStyle::SP_MediaSkipForward);
  pauseButton_->setEnabled(false);
  previousButton_->setEnabled(false);
  nextButton_->setEnabled(false);
  selectTrackButton_->setEnabled(false);
  transportRow->addWidget(selectTrackButton_);
  transportRow->addWidget(pauseButton_);
  transportRow->addWidget(previousButton_);
  transportRow->addWidget(nextButton_);
  transportRow->addStretch();

  openFileButton_ = new QPushButton("Open File...");
  saveFileButton_ = new QPushButton("Save to file...");
  saveFileButton_->setEnabled(false);
  transportRow->addWidget(openFileButton_);
  transportRow->addWidget(saveFileButton_);
  layout->addLayout(transportRow);

  auto* cueRow = new QHBoxLayout;
  timeLabel_ = new QLabel("0:00:00");
  timeLabel_->setAlignment(Qt::AlignCenter);
  timeLabel_->setMinimumWidth(112);
  timeLabel_->setStyleSheet("QLabel { background: #101214; color: #eef2f5; border-radius: 4px; padding: 6px; font-size: 22px; }");
  cueSlider_ = new SeekSlider;
  cueSlider_->setRange(0, 1000);
  cueSlider_->setValue(0);
  cueSlider_->setTracking(true);
  cueSlider_->setEnabled(false);
  cueRow->addWidget(timeLabel_);
  cueRow->addWidget(cueSlider_, 1);
  layout->addLayout(cueRow);

  auto* footer = new QHBoxLayout;
  footer->addWidget(new QLabel("Volume"));
  volumeSlider_ = horizontalSlider(0, 255, 192);
  footer->addWidget(volumeSlider_, 1);
  languageButton_ = new QPushButton("Language");
  aboutButton_ = new QPushButton("About");
  helpButton_ = new QPushButton("Help");
  footer->addWidget(languageButton_);
  footer->addWidget(aboutButton_);
  footer->addWidget(helpButton_);
  layout->addLayout(footer);

  return panel;
}

QWidget* MainWindow::createLoopPanel()
{
  auto* group = new QGroupBox("Loop Controls");
  loopPanel_ = group;
  group->setEnabled(false);

  auto* layout = new QGridLayout(group);
  layout->addWidget(new QLabel("min"), 0, 1);
  layout->addWidget(new QLabel("sec"), 0, 2);
  layout->addWidget(new QLabel("1/100 s"), 0, 3);

  auto addTimeRow = [layout](int row, const QString& label, QPushButton*& nowButton) {
    QSpinBox* minute = nullptr;
    QSpinBox* second = nullptr;
    QSpinBox* frame = nullptr;
    layout->addWidget(new QLabel(label), row, 0);
    for (int column = 1; column <= 3; ++column) {
      auto* box = new QSpinBox;
      box->setButtonSymbols(QAbstractSpinBox::PlusMinus);
      box->setMaximum(column == 1 ? 32767 : (column == 2 ? 59 : 99));
      box->setWrapping(column != 1);
      box->setKeyboardTracking(false);
      layout->addWidget(box, row, column);
      if (column == 1) {
        minute = box;
      } else if (column == 2) {
        second = box;
      } else {
        frame = box;
      }
    }
    nowButton = new QPushButton("Now");
    nowButton->setEnabled(false);
    layout->addWidget(nowButton, row, 4);
    return std::array<QSpinBox*, 3>{minute, second, frame};
  };

  const auto startBoxes = addTimeRow(1, "Start", loopStartButton_);
  loopStartMin_ = startBoxes[0];
  loopStartSec_ = startBoxes[1];
  loopStartFrame_ = startBoxes[2];
  const auto endBoxes = addTimeRow(2, "End", loopEndButton_);
  loopEndMin_ = endBoxes[0];
  loopEndSec_ = endBoxes[1];
  loopEndFrame_ = endBoxes[2];
  loopCheck_ = new QCheckBox("Loop");
  layout->addWidget(loopCheck_, 3, 0, 1, 2);

  return group;
}

QWidget* MainWindow::createProcessingTabs()
{
  auto* tabs = new QTabWidget;

  auto* timePitch = new QWidget;
  auto* tp = new QGridLayout(timePitch);

  pitchLabel_ = new QLabel("0 semitone(s)");
  pitchLabel_->setAlignment(Qt::AlignRight);
  pitchSlider_ = horizontalSlider(-12, 12, 0);
  auto* pitchReset = makeToolButton("Reset pitch", QStyle::SP_BrowserReload);
  tp->addWidget(new QLabel("Playing pitch"), 0, 0);
  tp->addWidget(pitchLabel_, 0, 1);
  tp->addWidget(pitchSlider_, 1, 0, 1, 2);
  tp->addWidget(pitchReset, 1, 2);

  finePitchLabel_ = new QLabel("0 cents");
  finePitchLabel_->setAlignment(Qt::AlignRight);
  finePitchSlider_ = horizontalSlider(-50, 50, 0);
  auto* fineReset = makeToolButton("Reset fine pitch", QStyle::SP_BrowserReload);
  tp->addWidget(new QLabel("Pitch fine adjust"), 2, 0);
  tp->addWidget(finePitchLabel_, 2, 1);
  tp->addWidget(finePitchSlider_, 3, 0, 1, 2);
  tp->addWidget(fineReset, 3, 2);

  speedLabel_ = new QLabel("100.0%");
  speedLabel_->setAlignment(Qt::AlignRight);
  speedSlider_ = horizontalSlider(200, 2000, 1000);
  auto* speedReset = makeToolButton("Reset speed", QStyle::SP_BrowserReload);
  tp->addWidget(new QLabel("Playing speed"), 4, 0);
  tp->addWidget(speedLabel_, 4, 1);
  tp->addWidget(speedSlider_, 5, 0, 1, 2);
  tp->addWidget(speedReset, 5, 2);

  filterCheck_ = new QCheckBox("Anti-aliasing filter");
  filterCheck_->setChecked(true);
  qualityCombo_ = new QComboBox;
  qualityCombo_->addItems({"Low", "Intermediate", "Best"});
  qualityCombo_->setCurrentIndex(2);
  tp->addWidget(filterCheck_, 6, 0, 1, 3);
  tp->addWidget(new QLabel("Time-stretch quality"), 7, 0);
  tp->addWidget(qualityCombo_, 7, 1, 1, 2);
  tp->setRowStretch(8, 1);

  connect(pitchReset, &QToolButton::clicked, pitchSlider_, [this] { pitchSlider_->setValue(0); });
  connect(fineReset, &QToolButton::clicked, finePitchSlider_, [this] { finePitchSlider_->setValue(0); });
  connect(speedReset, &QToolButton::clicked, speedSlider_, [this] { speedSlider_->setValue(1000); });
  tabs->addTab(timePitch, "Time and Pitch");

  auto* karaoke = new QWidget;
  auto* kp = new QGridLayout(karaoke);
  karaokeCheck_ = new QCheckBox("Karaoke mode");
  kp->addWidget(karaokeCheck_, 0, 0, 1, 3);
  kp->addWidget(new QLabel("Slide for optimal vocal suppression"), 1, 0, 1, 3);
  kp->addWidget(new QLabel("Left"), 2, 0);
  vocalPositionSlider_ = horizontalSlider(1, 256, 128);
  vocalPositionSlider_->setEnabled(false);
  kp->addWidget(vocalPositionSlider_, 2, 1);
  kp->addWidget(new QLabel("Right"), 2, 2);

  bassLabel_ = new QLabel("0 Hz");
  bassSlider_ = horizontalSlider(0, 500, 0);
  bassSlider_->setEnabled(false);
  kp->addWidget(new QLabel("Bass pass-through frequency range"), 3, 0, 1, 2);
  kp->addWidget(bassLabel_, 3, 2);
  kp->addWidget(bassSlider_, 4, 0, 1, 3);

  highLabel_ = new QLabel("0 Hz");
  highSlider_ = horizontalSlider(0, 2000, 0);
  highSlider_->setEnabled(false);
  kp->addWidget(new QLabel("Treble pass-through frequency range"), 5, 0, 1, 2);
  kp->addWidget(highLabel_, 5, 2);
  kp->addWidget(highSlider_, 6, 0, 1, 3);
  kp->setRowStretch(7, 1);

  tabs->addTab(karaoke, "Karaoke");
  return tabs;
}

QWidget* MainWindow::createMessagesPanel()
{
  auto* panel = new QWidget;
  auto* layout = new QVBoxLayout(panel);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sectionLabel("Messages"));
  messages_ = new QTableWidget(0, 2);
  setTableHeaders(messages_, {"Time", "Message"});
  layout->addWidget(messages_, 1);
  return panel;
}

QToolButton* MainWindow::makeToolButton(const QString& tooltip, QStyle::StandardPixmap icon)
{
  auto* button = new QToolButton;
  button->setIcon(style()->standardIcon(icon));
  button->setToolTip(tooltip);
  button->setAccessibleName(tooltip);
  button->setMinimumSize(44, 32);
  return button;
}

void MainWindow::connectUi()
{
  connect(refreshCdButton_, &QPushButton::clicked, this, [this] { showBackendPlaceholder("Refresh CD"); });
  connect(selectTrackButton_, &QToolButton::clicked, this, [this] { showBackendPlaceholder("Select CD track"); });
  connect(pauseButton_, &QToolButton::clicked, this, [this] {
    audioEngine_->togglePlayback();
  });
  connect(previousButton_, &QToolButton::clicked, this, [this] { audioEngine_->seek(0); });
  connect(nextButton_, &QToolButton::clicked, this, [this] { showBackendPlaceholder("Next track"); });
  connect(saveFileButton_, &QPushButton::clicked, this, [this] {
    QString path = QFileDialog::getSaveFileName(this, "Save processed audio", QString(),
                                                "Wave audio (*.wav)");
    if (path.isEmpty()) {
      return;
    }
    if (!path.endsWith(".wav", Qt::CaseInsensitive)) {
      path += ".wav";
    }
    QString error;
    if (audioEngine_->saveProcessedWav(path, &error)) {
      addMessage("Saved processed audio: " + path);
      statusBar()->showMessage("Saved " + path, 5000);
    } else {
      addMessage("Save error: " + error);
      statusBar()->showMessage(error, 5000);
    }
  });
  connect(helpButton_, &QPushButton::clicked, this, [this] { showBackendPlaceholder("Open help"); });
  connect(loopStartButton_, &QPushButton::clicked, this, [this] {
    setLoopTime(true, audioEngine_->position());
    if (loopTime(false) < loopTime(true)) {
      setLoopTime(false, loopTime(true));
    }
  });
  connect(loopEndButton_, &QPushButton::clicked, this, [this] {
    setLoopTime(false, audioEngine_->position());
    if (loopTime(false) < loopTime(true)) {
      setLoopTime(true, loopTime(false));
    }
  });

  connect(openFileButton_, &QPushButton::clicked, this, [this] {
    const QString path = QFileDialog::getOpenFileName(this, "Open audio file", QString(), "Audio files (*.wav *.mp3)");
    if (!path.isEmpty()) {
      openAudioFile(path);
    }
  });

  connect(aboutButton_, &QPushButton::clicked, this, [this] {
    AboutDialog dialog(this);
    dialog.exec();
  });

  connect(languageButton_, &QPushButton::clicked, this, [this] {
    LanguageDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
      addMessage("Selected language: " + dialog.selectedLanguageCode());
    }
  });

  connect(speedSlider_, &QSlider::valueChanged, this, [this](int value) {
    updateSpeedLabel(value);
    audioEngine_->setSpeed(value);
  });
  connect(pitchSlider_, &QSlider::valueChanged, this, [this](int value) {
    updatePitchLabel(value);
    audioEngine_->setPitch(value, finePitchSlider_->value());
  });
  connect(finePitchSlider_, &QSlider::valueChanged, this, [this](int value) {
    updateFinePitchLabel(value);
    audioEngine_->setPitch(pitchSlider_->value(), value);
  });
  connect(filterCheck_, &QCheckBox::toggled, audioEngine_, &AudioEngine::setAntiAliasEnabled);
  connect(qualityCombo_, &QComboBox::currentIndexChanged, audioEngine_, &AudioEngine::setQuality);
  connect(karaokeCheck_, &QCheckBox::toggled, this, [this](bool enabled) {
    vocalPositionSlider_->setEnabled(enabled);
    bassSlider_->setEnabled(enabled);
    highSlider_->setEnabled(enabled);
    audioEngine_->setKaraokeEnabled(enabled);
  });
  const auto updateKaraoke = [this] {
    audioEngine_->setKaraokeSettings(vocalPositionSlider_->value(),
                                     bassSlider_->value() * 10,
                                     highSlider_->value() * 10);
  };
  connect(vocalPositionSlider_, &QSlider::valueChanged, this, updateKaraoke);
  connect(bassSlider_, &QSlider::valueChanged, this, [this, updateKaraoke](int value) {
    updateBassLabel(value);
    updateKaraoke();
  });
  connect(highSlider_, &QSlider::valueChanged, this, [this, updateKaraoke](int value) {
    updateHighLabel(value);
    updateKaraoke();
  });
  connect(volumeSlider_, &QSlider::valueChanged, this, [this](int value) {
    audioEngine_->setVolume(value);
  });
  audioEngine_->setVolume(volumeSlider_->value());

  connect(cueSlider_, &QSlider::sliderMoved, audioEngine_, &AudioEngine::seek);
  connect(audioEngine_, &AudioEngine::positionChanged, this, [this](qint64 position) {
    updatePlaybackPosition(position);
  });
  connect(audioEngine_, &AudioEngine::durationChanged, this, [this](qint64 duration) {
    updatePlaybackDuration(duration);
  });
  connect(audioEngine_, &AudioEngine::playbackStateChanged, this, [this] {
    updatePlaybackState();
  });
  connect(audioEngine_, &AudioEngine::processingChanged, this, [this](bool processing) {
    openFileButton_->setEnabled(!processing);
    saveFileButton_->setEnabled(!processing && audioEngine_->hasAudio());
    pauseButton_->setEnabled(!processing && audioEngine_->hasAudio());
    statusBar()->showMessage(processing ? "Processing audio..." : "Ready");
  });
  connect(audioEngine_, &AudioEngine::ready, this, [this](const QString& path) {
    saveFileButton_->setEnabled(true);
    pauseButton_->setEnabled(true);
    cueSlider_->setEnabled(true);
    loopPanel_->setEnabled(true);
    loopStartButton_->setEnabled(true);
    loopEndButton_->setEnabled(true);
    previousButton_->setEnabled(true);
    statusBar()->showMessage("Playing " + QFileInfo(path).fileName());
  });
  connect(audioEngine_, &AudioEngine::message, this, &MainWindow::addMessage);
  connect(audioEngine_, &AudioEngine::errorOccurred, this, [this](const QString& error) {
    addMessage(error);
    statusBar()->showMessage(error);
  });
}

void MainWindow::openAudioFile(const QString& path)
{
  saveFileButton_->setEnabled(false);
  pauseButton_->setEnabled(false);
  cueSlider_->setEnabled(false);
  loopPanel_->setEnabled(false);
  setLoopTime(true, 0);
  setLoopTime(false, 0);
  loopCheck_->setChecked(false);
  audioEngine_->loadFile(path);
  addMessage("Selected file: " + path);
}

void MainWindow::addMessage(const QString& message)
{
  const int row = messages_->rowCount();
  messages_->insertRow(row);
  messages_->setItem(row, 0, new QTableWidgetItem(QTime::currentTime().toString("HH:mm:ss")));
  messages_->setItem(row, 1, new QTableWidgetItem(message));
  messages_->scrollToBottom();
}

void MainWindow::updateSpeedLabel(int value)
{
  speedLabel_->setText(QString::number(value / 10.0, 'f', 1) + "%");
}

void MainWindow::updatePitchLabel(int value)
{
  pitchLabel_->setText(QString("%1 semitone(s)").arg(value));
}

void MainWindow::updateFinePitchLabel(int value)
{
  finePitchLabel_->setText(QString("%1 cents").arg(value));
}

void MainWindow::updateBassLabel(int value)
{
  bassLabel_->setText(QString("%1 Hz").arg(value * 10));
}

void MainWindow::updateHighLabel(int value)
{
  highLabel_->setText(QString("%1 Hz").arg(value * 10));
}

void MainWindow::updatePlaybackPosition(qint64 position)
{
  if (!cueSlider_->isSliderDown()) {
    cueSlider_->setValue(static_cast<int>(position));
  }
  timeLabel_->setText(formatTime(position));

  if (loopCheck_->isChecked()) {
    const qint64 start = loopTime(true);
    const qint64 end = loopTime(false);
    if (end > start && position >= end) {
      audioEngine_->seek(start);
    }
  }
}

void MainWindow::updatePlaybackDuration(qint64 duration)
{
  cueSlider_->setRange(0, static_cast<int>(duration));
  cueSlider_->setEnabled(duration > 0);
  if (loopTime(false) == 0) {
    setLoopTime(false, duration);
  }
}

void MainWindow::updatePlaybackState()
{
  const bool playing = audioEngine_->isPlaying();
  pauseButton_->setIcon(style()->standardIcon(playing ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
  pauseButton_->setToolTip(playing ? "Pause" : "Play");
}

void MainWindow::setLoopTime(bool start, qint64 milliseconds)
{
  milliseconds = std::max<qint64>(0, milliseconds);
  const qint64 totalSeconds = milliseconds / 1000;
  const int minutes = static_cast<int>(totalSeconds / 60);
  const int seconds = static_cast<int>(totalSeconds % 60);
  const int frames = static_cast<int>((milliseconds % 1000) / 10);

  QSpinBox* minuteBox = start ? loopStartMin_ : loopEndMin_;
  QSpinBox* secondBox = start ? loopStartSec_ : loopEndSec_;
  QSpinBox* frameBox = start ? loopStartFrame_ : loopEndFrame_;
  minuteBox->setValue(minutes);
  secondBox->setValue(seconds);
  frameBox->setValue(frames);
}

qint64 MainWindow::loopTime(bool start) const
{
  const QSpinBox* minuteBox = start ? loopStartMin_ : loopEndMin_;
  const QSpinBox* secondBox = start ? loopStartSec_ : loopEndSec_;
  const QSpinBox* frameBox = start ? loopStartFrame_ : loopEndFrame_;
  return (static_cast<qint64>(minuteBox->value()) * 60 + secondBox->value()) * 1000
         + frameBox->value() * 10;
}

QString MainWindow::formatTime(qint64 milliseconds)
{
  const qint64 totalSeconds = std::max<qint64>(0, milliseconds) / 1000;
  const qint64 hours = totalSeconds / 3600;
  const qint64 minutes = (totalSeconds / 60) % 60;
  const qint64 seconds = totalSeconds % 60;
  return QString("%1:%2:%3")
    .arg(hours)
    .arg(minutes, 2, 10, QLatin1Char('0'))
    .arg(seconds, 2, 10, QLatin1Char('0'));
}

void MainWindow::showBackendPlaceholder(const QString& action)
{
  addMessage(action + " is ready for backend wiring.");
}
