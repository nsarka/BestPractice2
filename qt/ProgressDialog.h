#ifndef BESTPRACTICE_QT_PROGRESSDIALOG_H
#define BESTPRACTICE_QT_PROGRESSDIALOG_H

#include <QDialog>

class QLabel;
class QProgressBar;

class ProgressDialog : public QDialog
{
public:
  explicit ProgressDialog(QWidget* parent = nullptr);

  bool wasCanceled() const;
  void setProgress(int percent);

private:
  bool canceled_ = false;
  QLabel* percentLabel_ = nullptr;
  QProgressBar* progressBar_ = nullptr;
};

#endif
