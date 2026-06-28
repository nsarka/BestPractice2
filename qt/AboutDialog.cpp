#include "AboutDialog.h"

#include "version.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("About BestPractice");
  setModal(true);

  auto* layout = new QVBoxLayout(this);

  auto* title = new QLabel(QStringLiteral("BestPractice ") + BP_VERSION);
  QFont titleFont = title->font();
  titleFont.setPointSize(titleFont.pointSize() + 5);
  titleFont.setBold(true);
  title->setFont(titleFont);
  layout->addWidget(title);

  auto* body = new QLabel(
    "Released under the GPL version 2<br>"
    "(c) 2007 Robert Moerland<br><br>"
    "<a href=\"http://bestpractice.sourceforge.net\">bestpractice.sourceforge.net</a><br>"
    "akrip32 library (c) Jay A. Key<br>"
    "<a href=\"http://akrip.sourceforge.net\">akrip.sourceforge.net</a>");
  body->setOpenExternalLinks(true);
  body->setTextFormat(Qt::RichText);
  layout->addWidget(body);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttons);
}
