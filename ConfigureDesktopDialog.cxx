#include <ConfigureDesktopDialog.hxx>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

ConfigureDesktopDialog::ConfigureDesktopDialog(QWidget *parent, const DesktopWidget::Wallpaper &wp)
  : QDialog(parent), wallpaper(wp)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(i18n("Configure Wallpaper"));

  ui.setupUi(this);

  connect(ui.kcolorcombo, &KColorCombo::activated,
          [this](const QColor &col) { wallpaper.color = col; fill(); });

  connect(ui.kurlrequester, &KUrlRequester::urlSelected,
          [this](const QUrl &url) { wallpaper.fileName = url.toLocalFile(); fill(); });

  connect(ui.kurlrequester, QOverload<const QString &>::of(&KUrlRequester::returnPressed),
          [this](const QString &text) { wallpaper.fileName = text; fill(); });

  fill();
}

//--------------------------------------------------------------------------------

void ConfigureDesktopDialog::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event)

  ui.preview->setPixmap(wallpaper.getFinalPixmap(size()));
}

//--------------------------------------------------------------------------------

void ConfigureDesktopDialog::fill()
{
  QPalette pal = ui.preview->palette();
  pal.setColor(ui.preview->backgroundRole(), wallpaper.color);
  ui.preview->setPalette(pal);

  ui.kcolorcombo->setColor(wallpaper.color);
  ui.kurlrequester->setUrl(QUrl::fromLocalFile(wallpaper.fileName));

  ui.preview->setPixmap(wallpaper.getFinalPixmap(size()));
}

//--------------------------------------------------------------------------------
