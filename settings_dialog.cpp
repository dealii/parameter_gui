// ---------------------------------------------------------------------
//
// Copyright (C) 2017 by Martin Steigemann and Wolfgang Bangerth
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------


#include "settings_dialog.h"

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

namespace dealii
{
  namespace ParameterGui
  {
    SettingsDialog::SettingsDialog(QSettings *gui_settings,
                                   QWidget *parent)
               : QDialog(parent, 0)
    {
      setWindowTitle("Settings");

      settings = gui_settings;
      loadSettings();

      QFormLayout * grid = new QFormLayout(this);

      // add a choose font button
      change_font = new QPushButton(this);
      change_font->setText(QErrorMessage::tr("Change font"));
      connect(change_font, SIGNAL(clicked()), this, SLOT(selectFont()));
      grid->addRow("Change Font",change_font);

      // add a checkbox
      hide_default = new QCheckBox(this);
      hide_default->setChecked(hide_default_values);
      connect(hide_default, SIGNAL(stateChanged(int)), this, SLOT(changeHideDefault(int)));
      grid->addRow("Hide default values",hide_default);

      // add an OK button
      ok = new QPushButton(this);
      ok->setText(QErrorMessage::tr("&OK"));
      connect(ok, SIGNAL(clicked()), this, SLOT(accept()));

      // add a Cancel button
      cancel = new QPushButton(this);
      cancel->setText(QErrorMessage::tr("&Cancel"));
      connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
      grid->addRow(ok,cancel);

      connect(this, SIGNAL(accepted()), this, SLOT(writeSettings()));
    }



    void SettingsDialog::selectFont()
    {
      bool ok;
      QFont new_font = QFontDialog::getFont(
                      &ok, selected_font, this);
      if (ok) {
          selected_font = new_font;
      }
    }



    void SettingsDialog::changeHideDefault(int state)
    {
      hide_default_values = state;
    }



    void SettingsDialog::loadSettings()
    {
      settings->beginGroup("Settings");
      hide_default_values = settings->value("hideDefault", false).toBool();

      QString stored_font_string = settings->value("Font", QFont().toString()).toString();
      selected_font.fromString(stored_font_string);
      settings->endGroup();
    }



    void SettingsDialog::writeSettings()
    {
      settings->beginGroup("Settings");

      settings->setValue("hideDefault", hide_default_values);
      settings->setValue("Font", selected_font.toString());

      settings->endGroup();
    }
  }
}

