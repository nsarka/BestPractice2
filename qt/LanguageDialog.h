#ifndef BESTPRACTICE_QT_LANGUAGEDIALOG_H
#define BESTPRACTICE_QT_LANGUAGEDIALOG_H

#include <QDialog>

class QTableWidget;

class LanguageDialog : public QDialog
{
public:
  explicit LanguageDialog(QWidget* parent = nullptr);
  QString selectedLanguageCode() const;

private:
  void populateLanguages();

  QTableWidget* languages_ = nullptr;
};

#endif
