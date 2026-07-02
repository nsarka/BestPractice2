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
    "Original BestPractice software (c) 2007 Robert Moerland<br>"
    "<a href=\"https://bestpractice.sourceforge.net/\">"
    "bestpractice.sourceforge.net</a><br>"
    "PaulStretch algorithm (c) 2006-2011 Nasca Octavian Paul<br>"
    "Signalsmith Stretch (c) Geraint Luff<br>"
    "Rubber Band Library (c) 2007-2024 Particular Programs Ltd<br><br>"
    "<a href=\"https://github.com/nsarka/BestPractice2\">"
    "github.com/nsarka/BestPractice2</a>");
  body->setOpenExternalLinks(true);
  body->setTextFormat(Qt::RichText);
  layout->addWidget(body);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttons);
}
