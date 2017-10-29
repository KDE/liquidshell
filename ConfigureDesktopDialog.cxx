/*
  Copyright 2017 Martin Koller, kollix@aon.at

  This file is part of liquidshell.

  liquidshell is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  liquidshell is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with liquidshell.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ConfigureDesktopDialog.hxx>

//--------------------------------------------------------------------------------

ConfigureDesktopDialog::ConfigureDesktopDialog(QWidget *parent, const DesktopWidget::Wallpaper &wp)
  : QDialog(parent), wallpaper(wp)
{
  ui.setupUi(this);

  ui.kcolorcombo->setColor(wallpaper.color);
  ui.kurlrequester->setUrl(QUrl::fromLocalFile(wallpaper.fileName));

  connect(ui.kcolorcombo, &KColorCombo::activated,
          [this](const QColor &col) { wallpaper.color = col; emit changed(); });

  connect(ui.kurlrequester, &KUrlRequester::urlSelected,
          [this](const QUrl &url) { wallpaper.fileName = url.toLocalFile(); emit changed(); });

  connect(ui.kurlrequester, QOverload<const QString &>::of(&KUrlRequester::returnPressed),
          [this](const QString &text) { wallpaper.fileName = text; emit changed(); });

  if ( wallpaper.mode == "Scaled" )
    ui.scaledIgnoreRatioButton->setChecked(true);
  else if ( wallpaper.mode == "ScaledKeepRatio" )
    ui.scaledKeepRatioButton->setChecked(true);
  else if ( wallpaper.mode == "ScaledKeepRatioExpand" )
    ui.scaledKeepRatioClipButton->setChecked(true);
  else
    ui.origSizeButton->setChecked(true);

  buttonGroup.addButton(ui.origSizeButton);
  buttonGroup.addButton(ui.scaledIgnoreRatioButton);
  buttonGroup.addButton(ui.scaledKeepRatioButton);
  buttonGroup.addButton(ui.scaledKeepRatioClipButton);

  connect(&buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
          [this](QAbstractButton *button)
          {
            if ( button == ui.origSizeButton )
              wallpaper.mode = "";
            else if ( button == ui.scaledIgnoreRatioButton )
              wallpaper.mode = "Scaled";
            else if ( button == ui.scaledKeepRatioButton )
              wallpaper.mode = "ScaledKeepRatio";
            else if ( button == ui.scaledKeepRatioClipButton )
              wallpaper.mode = "ScaledKeepRatioExpand";

            emit changed();
          });
}

//--------------------------------------------------------------------------------
