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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QDialog>
#include <QSettings>
#include <QToolBar>
#include <QToolButton>

#include "info_message.h"
#include "settings_dialog.h"


namespace dealii
{
/*! @addtogroup ParameterGui
 *@{
 */
  namespace ParameterGui
  {
/**
 * The MainWindow class of the the parameterGUI.
 * The parameterGUI is a graphical user interface for editing parameter files based on the XML format,
 * created by the @ref ParameterHandler::print_parameters() function with @ref ParameterHandler::XML as second argument.
 * Please see <tt>Representation of Parameters</tt> in the documentation of the @ref ParameterHandler class for more details.
 * The MainWindow class provides the basic functionality of the GUI as save- and load-file-actions and documentation.
 * The parameterGUI provides special editors for the different types of parameters defined in the ParameterHandler class.
 *
 * @note This class is used in the graphical user interface for the @ref ParameterHandler class.
 *       It is not compiled into the deal.II libraries and can not be used by applications using deal.II.
 *
 * @ingroup ParameterGui
 * @author Martin Steigemann, Wolfgang Bangerth, 2010
 */
    class MainWindow : public QMainWindow
    {
      Q_OBJECT

      public:
				     /**
				      * Constructor.
				      * If a @p filename is given,
				      * the MainWindow tries to open
				      * and parse the file.
				      */
        MainWindow(const QString  &filename = "");

      protected:
				     /**
				      * Reimplemented from QMainWindow.
				      * We ask, if changes should be saved.
				      */
        void closeEvent(QCloseEvent *event);

      private slots:

				     /**
				      * Open a parameter file.
				      */
        void open();
				     /**
				      * Save the parameter file.
				      */
        bool save();
				     /**
				      * Open a file dialog to save the parameter file.
				      */
        bool save_as();
				     /**
				      * Show some information on the parameterGUI
				      */
        void about();

				     /**
				      * A <tt>slot</tt> that should be always called,
				      * if parameter values were changed.
				      */
        void tree_was_modified();

                                     /**
                                      * A <tt>slot</tt> that is called when the active
                                      * item has changed. It updates the documentation
                                      * text in the documentation panel.
                                      */
        void set_documentation_text(QTreeWidgetItem *selected_item,
                                    QTreeWidgetItem *previous_item);

                                    /**
                                     * A <tt>slot</tt> that is called when the active
                                     * tree item should be set to its default value.
                                     */
        void set_to_default();

                                    /**
                                     * A <tt>slot</tt> that is called when any
                                     * item value has changed. It updates the font depending
                                     * on whether the item has a default value or not.
                                     */
        void item_changed(QTreeWidgetItem *item,
                          int column);

        /**
         * Show an information dialog, how
         * parameters can be edited.
         */
        void show_settings ();

        /**
         * Apply the new settings to this window.
         */
        void apply_settings ();

        /**
         * Hide all default items in tree_widget if set in gui_settings.
         * Otherwise restores all default values.
         */
        void update_visible_items();

        /**
         * Reads the font from gui_settings and applies it.
         */
        void update_font();

        /**
         * Changes whether default items should be displayed in the tree widget
         * and calls update_visible_items() to apply the changes.
         */
        void toggle_visible_default_items();

        /**
         * Function that displays a font selection dialog, stores the result
         * in gui_settings, and displays the new font.
         */
        void select_font();
      private:
				     /**
				      * Show an information dialog, how
				      * parameters can be edited.
				      */
        void show_message ();
				     /**
				      * This function creates all actions.
				      */
        void create_actions();
				     /**
				      * This function creates all menus.
				      */
        void create_menus();

        /**
         * This function creates the toolbar.
         */
        void create_toolbar();
				     /**
				      * This function checks, if parameters were changed
				      * and show a dialog, if changes should be saved.
				      * This function should be always called,
				      * before open a new parameter file or before closing the GUI
				      */
        bool maybe_save ();
				     /**
				      * Save parameters to @p filename in XML format.
				      */
        bool save_file (const QString &filename);
				     /**
				      * Load parameters from @p filename in XML format.
				      */
        void load_file (const QString &filename);
				     /**
				      * This functions writes the current @p filename to the window title.
				      */
        void set_current_file (const QString  &filename);

        /**
         * Determine if the item and all of its children have the default value,
         * and hide all default items. Returns true if the item and all of its
         * children have default values.
         */
        bool hide_default_item(QTreeWidgetItem *item);

				     /**
				      * This is the tree structure in which we store all parameters.
				      */
        QTreeWidget * tree_widget;
                                     /**
                                      * This is the documentation text area.
                                      */
        QTextEdit *documentation_text_widget;

        /** A tool button that allows to toggle between showing/hiding parameters
         * with default values.
         */
        QToolButton *hide_default;
				     /**
				      * This menu provides all file actions as <tt>open</tt>, <tt>save</tt>, <tt>save as</tt>
				      * and <tt>exit</tt>
				      */
        QMenu * file_menu;
				     /**
				      * This menu provides some informations <tt>about</tt> the parameterGUI
				      * and <tt>about Qt</tt>
				      */
        QMenu * help_menu;
                                     /**
                                      * This menu provides context menu options for the active tree item.
                                      */
        QMenu * context_menu;
				     /**
				      * QAction <tt>open</tt> a file.
				      */
        QAction * open_act;
				     /**
				      * QAction <tt>save</tt> a file.
				      */
        QAction * save_act;
				     /**
				      * QAction <tt>save as</tt> a file.
				      */
        QAction * save_as_act;
                                     /**
                                      * QAction <tt>save as</tt> a file.
                                      */
        QAction * settings_act;
				     /**
				      * QAction <tt>exit</tt> the GUI.
				      */
        QAction * exit_act;
				     /**
				      * QAction <tt>about</tt> the parameterGUI.
				      */
        QAction * about_act;
				     /**
				      * QAction <tt>about</tt> Qt.
				      */
        QAction * about_qt_act;
                                     /**
                                      * QAction <tt>set_to_default</tt>.
                                      */
        QAction * set_to_default_act;

				     /**
				      * This value stores the current <tt>filename</tt> we work on.
				      */
        QString  current_file;
				     /**
				      * This dialog shows a short information message after loading a file.
				      */
        InfoMessage * info_message;

        SettingsDialog * settings_dialog;

				     /**
				      * An object for storing user settings.
				      */
        QSettings * gui_settings;
    };
  }
/**@}*/
}


#endif
