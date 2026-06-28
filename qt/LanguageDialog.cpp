#include "LanguageDialog.h"

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QHeaderView>
#include <QLocale>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

LanguageDialog::LanguageDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("Choose Language");
  resize(420, 340);

  auto* layout = new QVBoxLayout(this);
  languages_ = new QTableWidget(0, 2);
  languages_->setHorizontalHeaderLabels({"Language", "Code"});
  languages_->horizontalHeader()->setStretchLastSection(true);
  languages_->verticalHeader()->hide();
  languages_->setSelectionBehavior(QAbstractItemView::SelectRows);
  languages_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  layout->addWidget(languages_);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  buttons->button(QDialogButtonBox::Ok)->setText("Select Language");
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(languages_, &QTableWidget::cellDoubleClicked, this, [this] { accept(); });
  layout->addWidget(buttons);

  populateLanguages();
}

QString LanguageDialog::selectedLanguageCode() const
{
  const int row = languages_->currentRow();
  if (row < 0) {
    return {};
  }

  return languages_->item(row, 1)->text();
}

void LanguageDialog::populateLanguages()
{
  auto addRow = [this](const QString& name, const QString& code) {
    const int row = languages_->rowCount();
    languages_->insertRow(row);
    languages_->setItem(row, 0, new QTableWidgetItem(name));
    languages_->setItem(row, 1, new QTableWidgetItem(code));
  };

  addRow("(System default)", "");
  addRow("English (U.S.)", "en_US");

  QDir langDir(QCoreApplication::applicationDirPath() + "/lang");
  if (!langDir.exists()) {
    langDir = QDir(QCoreApplication::applicationDirPath() + "/../lang");
  }
  if (!langDir.exists()) {
    langDir = QDir("lang");
  }

  const QStringList dirs = langDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QString& code : dirs) {
    const QLocale locale(code);
    QString language = QLocale::languageToString(locale.language());
    const QString territory = QLocale::territoryToString(locale.territory());
    if (!territory.isEmpty()) {
      language += " (" + territory + ")";
    }
    addRow(language, code);
  }

  languages_->selectRow(0);
}
