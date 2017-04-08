// ---------------------------------------------------------------------
//
// Copyright (C) 2010 - 2013 by Martin Steigemann and Wolfgang Bangerth
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


#include <QtGui>

#include "info_message.h"


namespace dealii
{
  namespace ParameterGui
  {
    InfoMessage::InfoMessage(QWidget *parent)
               : QDialog(parent, 0)
    {
      // this variable stores, if the
      // the info message should be shown again
      show_again = true;
      QGridLayout * grid = new QGridLayout(this);

      // set an icon
      icon = new QLabel(this);
#ifndef QT_NO_MESSAGEBOX
      icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
      icon->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
#endif
      // add the icon in the upper left corner
      grid->addWidget(icon, 0, 0, Qt::AlignTop);

      // set the new message
      message = new QTextEdit(this);
      message->setReadOnly(true);

      // and add the message on the right
      grid->addWidget(message, 0, 1);

      // add a check box
      again = new QCheckBox(this);
      again->setChecked(true);
      again->setText(QErrorMessage::tr("&Show this message again"));
      grid->addWidget(again, 1, 1, Qt::AlignTop);

      // and finally a OK button
      ok = new QPushButton(this);
      ok->setText(QErrorMessage::tr("&OK"));
#ifdef QT_SOFTKEYS_ENABLED
      // define the action for the button
      ok_action = new QAction(ok);
      ok_action->setSoftKeyRole(QAction::PositiveSoftKey);
      ok_action->setText(ok->text());
      connect(ok_action, SIGNAL(triggered()), this, SLOT(accept()));
      addAction(ok_action);
#endif
      connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
      // and set the focus on the button
      ok->setFocus();
      grid->addWidget(ok, 2, 0, 1, 2, Qt::AlignCenter);

      grid->setColumnStretch(1, 42);
      grid->setRowStretch(0, 42);

      // load settings from an ini-file
      QString  settings_file = QDir::currentPath() + "/settings.ini";

      settings = new QSettings (settings_file, QSettings::IniFormat);

      // we store settings of this class in the group infoMessage
      settings->beginGroup("infoMessage");
      show_again = settings->value("showInformation", true).toBool();
      settings->endGroup();
    }



    void InfoMessage::setInfoMessage(const QString &message)
    {
      // set the message
      this->message->setText(message);
    }



    void InfoMessage::showMessage()
    {
      // and show the message
      if (show_again)
        show();
    }



    void InfoMessage::done(int r)
    {
      // if the box is not checked, store this to settings
      if(!again->isChecked())
        {
          settings->beginGroup("infoMessage");
          settings->setValue("showInformation", false);
          settings->endGroup();
        };

      QDialog::done(r);
    }
  }
}

