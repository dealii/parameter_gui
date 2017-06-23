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


#include "mainwindow.h"
#include "parameter_delegate.h"
#include "xml_parameter_reader.h"
#include "xml_parameter_writer.h"
#include "prm_parameter_writer.h"

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

namespace dealii
{
  namespace ParameterGui
  {
    MainWindow::MainWindow(const QString  &filename)
    {
      // load settings
      gui_settings = new QSettings ("deal.II", "parameterGUI");

      // tree for showing XML tags
      tree_widget = new QTreeWidget;

      // Setup the tree and the window first:
#if QT_VERSION >= 0x050000
      tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
      tree_widget->header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
      tree_widget->setHeaderLabels(QStringList() << tr("(Sub)Sections/Parameters")
                                                 << tr("Value"));

      // enables mouse events e.g. showing ToolTips
      // and documentation in the StatusLine
      tree_widget->setMouseTracking(true);
      tree_widget->setEditTriggers(QAbstractItemView::DoubleClicked|
                                   QAbstractItemView::SelectedClicked|
                                   QAbstractItemView::EditKeyPressed);

      //Enable right click menu in tree
      tree_widget->setContextMenuPolicy(Qt::ActionsContextMenu);
      context_menu = new QMenu(tree_widget);

      // set the delegate for editing items
      tree_widget->setItemDelegate(new ParameterDelegate(1));

      setCentralWidget(tree_widget);

      connect(tree_widget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(set_documentation_text(QTreeWidgetItem *, QTreeWidgetItem *)));
      // connect: if the tree changes, the window will know
      connect(tree_widget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(item_changed(QTreeWidgetItem *, int)));
      connect(tree_widget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(tree_was_modified()));

      QDockWidget *documentation_widget = new QDockWidget(tr("Parameter documentation:"), this);
      documentation_text_widget = new QTextEdit(QString (""), documentation_widget);
      documentation_text_widget->setReadOnly(true);

      documentation_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
      documentation_widget->setWidget(documentation_text_widget);

      addDockWidget(Qt::BottomDockWidgetArea, documentation_widget);

      // create window actions as "Open",...
      create_actions();
      // and menus
      create_menus();
      // and the toolbar
      create_toolbar();

      statusBar()->showMessage(tr("Ready, start editing by double-clicking or hitting F2!"));
      setWindowTitle(tr("[*]parameterGUI"));

      gui_settings->beginGroup("MainWindow");
      resize(gui_settings->value("size", QSize(800, 600)).toSize());
      move(gui_settings->value("pos", QPoint(0, 0)).toPoint());
      gui_settings->endGroup();

      // if there is a file_name, try to load the file.
      // a valid file has the xml extension, so we require size() > 3
      if (filename.size() > 3)
        load_file(filename);

      apply_settings();
    }



    void MainWindow::set_documentation_text(QTreeWidgetItem *selected_item,
                                            QTreeWidgetItem *previous_item)
    {
      documentation_text_widget->clear();
      documentation_text_widget->insertPlainText(selected_item->text(3));
    }



    void MainWindow::item_changed(QTreeWidgetItem *item,
                                  int column)
    {
      if (column != 1)
        return;

      bool has_default_value;

      if (item->text(5).startsWith("[Double"))
        has_default_value = item->data(1,Qt::DisplayRole).toReal() == item->data(2,Qt::DisplayRole).toReal();
      else
        has_default_value = item->data(1,Qt::DisplayRole).toString() == item->data(2,Qt::DisplayRole).toString();

      if (has_default_value)
        {
          QFont font = item->font(1);
          font.setWeight(QFont::Normal);
          item->setFont(1,font);

          const bool hide_items_with_default_value = gui_settings->value("Settings/hideDefault", false).toBool();
          if (hide_items_with_default_value)
            item->setHidden(true);
        }
      else
        {
          QFont font = item->font(1);
          font.setWeight(QFont::Bold);
          item->setFont(1,font);
        }
    }



    void MainWindow::set_to_default()
    {
      QTreeWidgetItem * current_item = tree_widget->currentItem();
      current_item->setText(1,current_item->text(2));
    }



    void MainWindow::open()
    {
      // check, if the content was modified
      if (maybe_save())
        {
          // open a file dialog
          QString  file_name =
                     QFileDialog::getOpenFileName(this, tr("Open XML Parameter File"),
                                                  QDir::currentPath(),
                                                  tr("XML Files (*.xml)"));

          // if a file was selected, load the content
          if (!file_name.isEmpty())
            load_file(file_name);
        }
    }



    bool MainWindow::save()
    {
      // if there is no file to save changes, open a dialog
      if (current_file.isEmpty())
        return save_as();
      else
        return save_file(current_file);
    }



    bool MainWindow::save_as()
    {
      const QString default_save_format = gui_settings->value("Settings/DefaultSaveFormat").toString();

      QString filters;
      if (default_save_format == "prm")
        filters = tr("PRM Files (*.prm);;XML Files (*.xml)");
      else
        filters = tr("XML Files (*.xml);;PRM Files (*.prm)");

      // open a file dialog
      QString  file_name =
                 QFileDialog::getSaveFileName(this, tr("Save Parameter File"),
                                              QDir::currentPath() + QDir::separator() + current_file,
                                              filters);

      // return if a file was saved
      if (file_name.isEmpty())
        return false;
      else
        return save_file(file_name);
    }



    void MainWindow::about()
    {
#ifdef Q_WS_MAC
      static QPointer<QMessageBox> old_msg_box;

      if (old_msg_box)
        {
          old_msg_box->show();
          old_msg_box->raise();
          old_msg_box->activateWindow();
          return;
        };
#endif

      QString title = "About parameterGUI";

      QString trAboutparameterGUIcaption;
      trAboutparameterGUIcaption = QMessageBox::tr(
        "<h3>parameterGUI: A GraphicalUserInterface for parameter handling in deal.II</h3>"
        "<p>This program uses Qt version %1.</p>"
        ).arg(QLatin1String(QT_VERSION_STR));

      QString trAboutparameterGUItext;
      trAboutparameterGUItext = QMessageBox::tr(
        "<p>The parameterGUI is a graphical user interface for editing XML parameter files "
        "created by the ParameterHandler class of deal.II. Please see "
        "<a href=\"http://www.dealii.org/7.0.0/doxygen/deal.II/classParameterHandler.html\">dealii.org/doc</a> for more information. "
        "The parameterGUI parses XML files into a tree structure and provides "
        " special editors for different types of parameters.</p>"

        "<p><b>Editing parameter values:</b><br>"
        "Parameters can be edited by (double-)clicking on the value or "
        "by pressing the platform edit key (F2 on Linux) over an parameter item.</p>"

        "<p><b>Editors for parameter values:</b>"
        " <ul>"
        "  <li>Integer- and Double-type parameters: SpinBox</li>"
        "  <li>Booleans: ComboBox</li>"
        "  <li>Selection: ComboBox</li>"
        "  <li>File- and DirectoryName parameters: BrowseLineEditor</li>"
        "  <li>Anything|MultipleSelection|List: LineEditor</li>"
        " </ul>"
        "</p>"

        "<p>Please see <a href=\"http://www.dealii.org\">dealii.org</a> for more information</p>"
        "<p><b>Authors:</b><br> "
        "Martin Steigemann,  <a href=\"mailto:martin.steigemann@mathematik.uni-kassel.de\">martin.steigemann@mathematik.uni-kassel.de</a><br>"
        "Wolfgang Bangerth,  <a href=\"mailto:bangerth@math.tamu.edu\">bangerth@math.tamu.edu</a></p>"
        );

      QMessageBox *msg_box = new QMessageBox;
      msg_box->setAttribute(Qt::WA_DeleteOnClose);
      msg_box->setWindowTitle(title);
      msg_box->setText(trAboutparameterGUIcaption);
      msg_box->setInformativeText(trAboutparameterGUItext);

      QPixmap pm(QLatin1String(":/images/logo_dealii_gui_128.png"));

      if (!pm.isNull())
        msg_box->setIconPixmap(pm);

#ifdef Q_WS_MAC
      old_msg_box = msg_box;
      msg_box->show();
#else
      msg_box->exec();
#endif
    }



    void MainWindow::tree_was_modified()
    {
      // store, that the window was modified
      // this is a function from the QMainWindow class
      // and we use the windowModified mechanism to show a "*"
      // in the window title, if content was modified
      setWindowModified(true);
    }



    void MainWindow::show_message ()
    {
      QString title = "parameterGUI";

      info_message = new InfoMessage(this);

      info_message->setWindowTitle(title);
      info_message->setInfoMessage(tr("Start Editing by double-clicking on the parameter value or"
                                      " by hitting the platform edit key. For example, on Linux this is the F2-key!"));
      info_message->showMessage();
    }



    void MainWindow::show_settings ()
    {
      settings_dialog = new SettingsDialog(gui_settings,this);

      connect(settings_dialog, SIGNAL(accepted()), this, SLOT(apply_settings()));
      settings_dialog->exec();
      disconnect(settings_dialog, SIGNAL(accepted()), this, SLOT(apply_settings()));
    }



    void MainWindow::apply_settings ()
    {
      update_visible_items();
      update_font();
    }



    void MainWindow::toggle_visible_default_items()
    {
      const bool hide_default_values = gui_settings->value("Settings/hideDefault", false).toBool();
      gui_settings->setValue("Settings/hideDefault", !hide_default_values);
      update_visible_items();
    }



    void MainWindow::update_visible_items()
    {
      const bool hide_default_values = gui_settings->value("Settings/hideDefault", false).toBool();

      if (hide_default_values)
        {
        for (int i = 0; i < tree_widget->topLevelItemCount(); ++i)
            hide_item_with_default_value(tree_widget->topLevelItem(i));
        hide_default->setChecked(true);
        }
      else
        {
          QTreeWidgetItemIterator it(tree_widget,QTreeWidgetItemIterator::Hidden);
          while (*it)
            {
              (*it)->setHidden(false);
              ++it;
            }
          hide_default->setChecked(false);
          }
    }



    bool MainWindow::hide_item_with_default_value(QTreeWidgetItem *item)
    {
      bool has_default_value = true;

      if (item->childCount() == 0)
        {
          if (item->text(5).startsWith("[Double"))
            has_default_value = item->data(1,Qt::DisplayRole).toReal() == item->data(2,Qt::DisplayRole).toReal();
          else
            has_default_value = item->data(1,Qt::DisplayRole).toString() == item->data(2,Qt::DisplayRole).toString();
        }
      else
        {
          // If this element has children recurse into them and check for default values
          for (int i = 0; i < item->childCount(); ++i)
            {
              const bool child_has_default_value = hide_item_with_default_value(item->child(i));
              has_default_value = has_default_value & child_has_default_value;
            }
        }

      if (has_default_value)
        item->setHidden(true);

      return has_default_value;
    }



    void MainWindow::closeEvent(QCloseEvent *event)
    {
      // Reimplement the closeEvent from the QMainWindow class.
      // First check, if we have to save modified content.
      // If not, or the content was saved, accept the event, otherwise ignore it
      if (maybe_save())
        {
          gui_settings->beginGroup("MainWindow");
          gui_settings->setValue("size", size());
          gui_settings->setValue("pos", pos());
          gui_settings->endGroup();

          event->accept();
        }
      else
        event->ignore();
    }



    void MainWindow::create_actions()
    {
      QStyle * style = tree_widget->style();

      // Create actions, and set icons, shortcuts, status tip and connect to
      // activate the action.
      open_act = new QAction(tr("&Open..."), this);
      open_act->setIcon(style->standardPixmap(QStyle::SP_DialogOpenButton));
      open_act->setShortcut(Qt::CTRL + Qt::Key_O);
      open_act->setStatusTip(tr("Open a XML file"));
      connect(open_act, SIGNAL(triggered()), this, SLOT(open()));

      save_act = new QAction(tr("&Save ..."), this);
      save_act->setIcon(style->standardPixmap(QStyle::SP_DialogSaveButton));
      save_act->setShortcut(Qt::CTRL + Qt::Key_S);
      save_act->setStatusTip(tr("Save the current XML file"));
      connect(save_act, SIGNAL(triggered()), this, SLOT(save()));

      save_as_act = new QAction(tr("&Save As..."), this);
      save_as_act->setIcon(style->standardPixmap(QStyle::SP_DialogSaveButton));
      save_as_act->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Q);
      save_as_act->setStatusTip(tr("Save the current XML file as"));
      connect(save_as_act, SIGNAL(triggered()), this, SLOT(save_as()));

      exit_act = new QAction(tr("E&xit"), this);
      exit_act->setIcon(style->standardPixmap(QStyle::SP_DialogCloseButton));
      exit_act->setShortcut(Qt::CTRL + Qt::Key_Q);
      exit_act->setStatusTip(tr("Exit the parameterGUI application"));
      connect(exit_act, SIGNAL(triggered()), this, SLOT(close()));

      about_act = new QAction(tr("&About"), this);
      about_act->setIcon(style->standardPixmap(QStyle::SP_FileDialogInfoView));
      about_act->setStatusTip(tr("Show the parameterGUI About box"));
      connect(about_act, SIGNAL(triggered()), this, SLOT(about()));

      about_qt_act = new QAction(tr("About &Qt"), this);
      about_qt_act->setStatusTip(tr("Show the Qt library's About box"));
      connect(about_qt_act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

      settings_act = new QAction(tr("Settings"), this);
      settings_act->setStatusTip(tr("Show the Settings Dialog"));
      connect(settings_act, SIGNAL(triggered()), this, SLOT(show_settings()));

      set_to_default_act = new QAction("Set to default",context_menu);
      tree_widget->addAction(set_to_default_act);
      connect(set_to_default_act, SIGNAL(triggered()), this, SLOT(set_to_default()));
    }



    void MainWindow::create_menus()
    {
      // create a file menu, and add the entries
      file_menu = menuBar()->addMenu(tr("&File"));
      file_menu->addAction(open_act);
      file_menu->addAction(save_act);
      file_menu->addAction(save_as_act);
      file_menu->addSeparator();
      file_menu->addAction(settings_act);
      file_menu->addSeparator();
      file_menu->addAction(exit_act);

      menuBar()->addSeparator();

      // create a help menu
      help_menu = menuBar()->addMenu(tr("&Help"));
      help_menu->addAction(about_act);
      help_menu->addAction(about_qt_act);
    }



    void MainWindow::create_toolbar()
    {
      QToolBar *toolbar = new QToolBar(tr("Toolbar"),this);

      // add entries
      toolbar->addAction(open_act);
      toolbar->addAction(save_act);
      toolbar->addAction(save_as_act);

      toolbar->addSeparator();

      hide_default = new QToolButton(toolbar);
      hide_default->setText(tr("Hide default values"));
      hide_default->setCheckable(true);
      connect(hide_default, SIGNAL(clicked()), this, SLOT(toggle_visible_default_items()));
      QAction *hide_default_act = toolbar->addWidget(hide_default);

      QToolButton *change_font = new QToolButton(toolbar);
      change_font->setText(tr("Change font"));
      connect(change_font, SIGNAL(clicked()), this, SLOT(select_font()));
      QAction *change_font_act = toolbar->addWidget(change_font);

      addToolBar(toolbar);
    }



    bool MainWindow::maybe_save()
    {
      // if content was modified, ask if content should be saved
      if (isWindowModified())
        {
          QMessageBox::StandardButton ret;
          ret = QMessageBox::warning(this, tr("parameterGUI"),
                                     tr("The content has been modified.\n"
                                        "Do you want to save your changes?"),
                  QMessageBox::Save | QMessageBox::Discard |QMessageBox::Cancel);

          if (ret == QMessageBox::Save)
            return save();
          else if (ret == QMessageBox::Cancel)
            return false;
        };

      return true;
    }



    bool MainWindow::save_file(const QString &filename)
    {
      QFile  file(filename);

      // open a file dialog
      if (!file.open(QFile::WriteOnly | QFile::Text))
        {
          QMessageBox::warning(this, tr("parameterGUI"),
                                     tr("Cannot write file %1:\n%2.")
                                     .arg(filename)
                                     .arg(file.errorString()));
          return false;
        };

      if (filename.endsWith(".xml",Qt::CaseInsensitive))
        {
          // create a xml writer and write the xml file
          XMLParameterWriter writer(tree_widget);
          if (!writer.write_xml_file(&file))
            return false;

          gui_settings->setValue("Settings/DefaultSaveFormat", "xml");
        }
      else if (filename.endsWith(".prm",Qt::CaseInsensitive))
        {
          // create a prm writer and write the prm file
          PRMParameterWriter writer(tree_widget);
          if (!writer.write_prm_file(&file))
            return false;

          gui_settings->setValue("Settings/DefaultSaveFormat", "prm");
        }
      else
        {
          QMessageBox::warning(this, tr("parameterGUI"),
                                     tr("Unknown output format: %1.")
                                     .arg(filename));
          file.remove();

          return false;
        }

      // if we succeed, show a message and reset the window
      statusBar()->showMessage(tr("File saved"), 2000);
      set_current_file(filename);

      return true;
    }



    void MainWindow::load_file(const QString &filename)
    {
      QFile  file(filename);

      // open the file
      if (!file.open(QFile::ReadOnly | QFile::Text))
        {
          QMessageBox::warning(this, tr("parameterGUI"),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(filename)
                                     .arg(file.errorString()));
          return;
        };

      // clear the tree and read the xml file
      tree_widget->clear();
      XMLParameterReader xml_reader(tree_widget);

      if (!xml_reader.read_xml_file(&file))
        {
          QMessageBox::warning(this, tr("parameterGUI"),
                                     tr("Parse error in file %1:\n\n%2")
                                     .arg(filename)
                                     .arg(xml_reader.error_string()));
        }
      else
        {
          // show a message and set current file
          statusBar()->showMessage(tr("File loaded - Start editing by double-clicking or hitting F2"), 25000);
          set_current_file(filename);

          // show some informations how values can be edited
          show_message ();
        };
    }



    void MainWindow::set_current_file(const QString  &filename)
    {
      // We use the windowModified mechanism from the
      // QMainWindow class to indicate in the window title,
      // if the content was modified.
      // If there is "[*]" in the window title, a * will
      // added automatically at this position, if the
      // window was modified.
      // We set the window title to
      // file_name[*] - XMLParameterHandler

      // set the (global) current file to file_name
      current_file = filename;

      // and create the window title,
      std::string win_title = (filename.toStdString());

      // if file_name is empty set the title to our application name
      if (current_file.isEmpty())
        win_title = "[*]parameterGUI";
      else
        {
          // if there is a file_name, add the
          // the file_name and a minus to the title
          win_title += "[*] - parameterGUI";
        }

      // set the window title and reset window modified
      setWindowTitle(tr(win_title.c_str()));
      setWindowModified(false);
    }



    void MainWindow::update_font()
    {
      QString current_font_string = gui_settings->value("Settings/Font", QFont().toString()).toString();
      QFont current_font;
      current_font.fromString(current_font_string);
      setFont(current_font);
    }



    void MainWindow::select_font()
    {
      QString current_font_string = gui_settings->value("Settings/Font", QFont().toString()).toString();
      QFont current_font;
      current_font.fromString(current_font_string);

      bool ok;
      QFont new_font = QFontDialog::getFont(
                      &ok, current_font, this);
      if (ok) {
          gui_settings->setValue("Settings/Font", new_font.toString());
          setFont(new_font);
      }
    }
  }
}
