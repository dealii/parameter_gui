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


#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QCheckBox>


namespace dealii
{
  /*! @addtogroup ParameterGui
   *@{
   */
  namespace ParameterGui
  {
    /**
     * The SettingsDialog class implements a settings dialog for the parameterGUI.
     * The dialog shows all available settings, and when the user clicks on 'OK'
     * stores them in the QSettings object handed over in the constructor (which
     * in turn stores them on disk to allow persistent settings).
     *
     * @ingroup ParameterGui
     * @author Rene Gassmoeller, 2017
     */
    class SettingsDialog : public QDialog
    {
      Q_OBJECT

    public:
      /**
       * Constructor
       */
      SettingsDialog (QSettings *settings,
                      QWidget *parent = 0);

    public slots:
      /**
       * Function that displays a font selection dialog and stores the result.
       */
      void selectFont();

      /**
       * Function that stores the checked state of the "Hide default" checkbox.
       */
      void changeHideDefault(int state);

      /**
       * Function that stores the new settings in the settings object
       * (i.e. on disk).
       */
      void writeSettings();

      /**
       * Function that loads the settings from the settings file.
       */
      void loadSettings();

    private:
      /**
       * This variable stores if the default values should be hidden. This
       * might seem duplicative, since it is also stored in <tt>settings</tt>,
       * but this variable stores the 'current' state of the checkbox,
       * while <tt>settings</tt> is only updated after clicking OK.
       */
      bool hide_default_values;

      /**
       * The selected font as shown in the Change Font dialog.
       */
      QFont selected_font;

      /**
       * The <tt>Ok</tt> button.
       */
      QPushButton *ok;

      /**
       * The <tt>Cancel</tt> button.
       */
      QPushButton *cancel;

      /**
       * The <tt>Change font</tt> button.
       */
      QPushButton *change_font;

      /**
       * The checkbox<tt>Hide default values</tt>.
       */
      QCheckBox *hide_default;

      /**
       * An object for storing <tt>settings</tt> in a file.
       */
      QSettings *settings;
    };
  }
  /**@}*/
}


#endif
