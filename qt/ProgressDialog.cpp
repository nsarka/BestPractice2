#include "ProgressDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

ProgressDialog::ProgressDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("Processing...");
  setModal(true);

  auto* layout = new QVBoxLayout(this);
  progressBar_ = new QProgressBar;
  progressBar_->setRange(0, 100);
  percentLabel_ = new QLabel("0 %");
  layout->addWidget(progressBar_);
  layout->addWidget(percentLabel_);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::rejected, this, [this] {
    canceled_ = true;
    reject();
  });
  layout->addWidget(buttons);
}

bool ProgressDialog::wasCanceled() const
{
  return canceled_;
}

void ProgressDialog::setProgress(int percent)
{
  progressBar_->setValue(percent);
  percentLabel_->setText(QString("%1 %").arg(percent));
}
