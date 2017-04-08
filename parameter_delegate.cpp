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

#include "parameter_delegate.h"

#include <limits>


namespace dealii
{
  namespace ParameterGui
  {
    ParameterDelegate::ParameterDelegate(const int value_column, QObject *parent)
                     : QItemDelegate(parent)
    {
      this->value_column = value_column;
    }



    QSize ParameterDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
      if (index.column() == value_column)
        {
          return QSize(400,30);		// we increase the height of all lines to show editors

/*
      QString pattern_description = index.data(Qt::StatusTipRole).toString();	// load pattern description
										// stored in the StatusLine
      QRegExp  rx_string("\\b(FileName|DirectoryName)\\b");

      if (rx_string.indexIn (pattern_description) != -1)
        {
          return QSize(400,35);					// we increase the height for FileName and
        }							// DirectoryName to show a "browse" button
      else
        return QItemDelegate::sizeHint(option, index);
*/

        }
      else
        return QItemDelegate::sizeHint(option, index);
    }



    void ParameterDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
      if (index.column() == value_column)
        {
          QString pattern_description = index.data(Qt::StatusTipRole).toString();	// load pattern description
											// stored in the StatusLine
          QRegExp  rx_string("\\b(FileName|DirectoryName)\\b");				// if the type is Filename
											// or DirectoryName
          if (rx_string.indexIn (pattern_description) != -1)
            {
              QString value = index.model()->data(index, Qt::DisplayRole).toString();	// take the value

              QStyleOptionViewItem my_option = option;					// load options
              my_option.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;

              drawDisplay(painter, my_option, my_option.rect, value);			// print the text in the display
              drawFocus(painter, my_option, my_option.rect);				// if the line has the
											// focus, print a rectangle
            }
          else
            QItemDelegate::paint(painter, option, index);				// for all other types use
											// the standard delegate
        }
      else
        QItemDelegate::paint(painter, option, index);
    }



    QWidget *ParameterDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
    {
      if (index.column() == value_column)
        {
          QString pattern_description = index.data(Qt::StatusTipRole).toString();	// load pattern description
											// stored in the StatusLine
          QRegExp  rx_string("\\b(Anything|MultipleSelection|Map)\\b"),
                   rx_list("\\b(List)\\b"),
                   rx_filename("\\b(FileName)\\b"),	
                   rx_dirname("\\b(DirectoryName)\\b"),	
                   rx_integer("\\b(Integer)\\b"),
                   rx_double("\\b(Double|Float|Floating)\\b"),
                   rx_selection("\\b(Selection)\\b"),
                   rx_bool("\\b(Bool)\\b");

          if (rx_string.indexIn (pattern_description) != -1)				// if the type is "Anything"
            {
              QLineEdit * line_editor = new QLineEdit(parent);				// choose a LineEditor

              connect(line_editor, SIGNAL(editingFinished()),				// and connect editors signal
                      this, SLOT(commit_and_close_editor()));				// to the closer function

              return line_editor;
            }
          else if (rx_list.indexIn (pattern_description) != -1)                         // if the type is "List"
            {
              if (rx_filename.indexIn (pattern_description) != -1)
                {
                  BrowseLineEdit * filename_editor =                                        // choose a BrowseLineEditor
                      new BrowseLineEdit(BrowseLineEdit::files, parent);

                  connect(filename_editor, SIGNAL(editingFinished()),
                          this, SLOT(commit_and_close_editor()));

                  return filename_editor;
                }
              else
                {
                  QLineEdit * line_editor = new QLineEdit(parent);                                // choose a LineEditor

                  connect(line_editor, SIGNAL(editingFinished()),                           // and connect editors signal
                          this, SLOT(commit_and_close_editor()));                           // to the closer function

                  return line_editor;
                }
            }
          else if (rx_filename.indexIn (pattern_description) != -1)			// if the type is "FileName"
            {
              BrowseLineEdit * filename_editor =					// choose a BrowseLineEditor
                                 new BrowseLineEdit(BrowseLineEdit::file, parent);

              connect(filename_editor, SIGNAL(editingFinished()),
                      this, SLOT(commit_and_close_editor()));

              return filename_editor;
            }
          else if (rx_dirname.indexIn (pattern_description) != -1)			// if the type is "DirectoryName"
            {
              BrowseLineEdit * dirname_editor =						// choose a BrowseLineEditor
                                 new BrowseLineEdit(BrowseLineEdit::directory, parent);

              connect(dirname_editor, SIGNAL(editingFinished()),
                      this, SLOT(commit_and_close_editor()));

              return dirname_editor;
            }
          else if (rx_integer.indexIn (pattern_description) != -1)		// if the type is "Integer"
            {
              const QStringList default_pattern = pattern_description.split(" ").filter("...");
              const QStringList default_values = default_pattern[0].split("...");

              QLineEdit * line_edit = new QLineEdit(parent);
              line_edit->setValidator(new QIntValidator(default_values[0].toInt(), default_values[1].toInt(), line_edit));

              connect(line_edit, SIGNAL(editingFinished()),			// connect editors signal to the closer function
                      this, SLOT(commit_and_close_editor()));

              return line_edit;
            }
          else if (rx_double.indexIn (pattern_description) != -1)		// the same with "Double"
            {
              const QStringList default_pattern = pattern_description.split(" ").filter("...");
              QStringList default_values = default_pattern[0].split("...");

              // Unfortunately conversion of MAX_DOUBLE to string and back fails
              // sometimes, therefore use MAX_DOUBLE/2 to make sure we are below.
              // In practice MAX_DOUBLE just means VERY large, it is normally not
              // important how large.
              const double max_double = std::numeric_limits<double>::max()/2;
              default_values = default_values.replaceInStrings("MAX_DOUBLE",
                                                               QVariant(max_double).toString());

              const unsigned int number_of_decimals = 14;

              QLineEdit * line_edit = new QLineEdit(parent);
              line_edit->setValidator(new QDoubleValidator(default_values[0].toDouble(),
                                                           default_values[1].toDouble(),
                                                           number_of_decimals,
                                                           line_edit));

              connect(line_edit, SIGNAL(editingFinished()),		// connect editors signal to the closer function
                      this, SLOT(commit_and_close_editor()));

              return line_edit;
            }
          else if (rx_selection.indexIn (pattern_description) != -1)		// and selections
            {
              QComboBox * combo_box = new QComboBox(parent);

              // we assume, that pattern_description is of the form
              // "Type: [Selection item1|item2| ....|item ]".
              // Find the first space after the first '[',
              // which indicates the start of the first option
              int begin_pattern = pattern_description.indexOf("[");
              begin_pattern = pattern_description.indexOf(" ",begin_pattern) + 1;

              // Find the last ']', which signals the end of the options
              const int end_pattern = pattern_description.lastIndexOf("]");

              // Extract the options from the string
              QString pattern = pattern_description.mid(begin_pattern,end_pattern-begin_pattern);

              // Remove trailing whitespaces
              while (pattern.endsWith(' '))
                pattern.chop(1);

              // Split the list
              const QStringList choices = pattern.split("|");
              combo_box->addItems(choices);

              combo_box->setEditable(false);

              connect(combo_box, SIGNAL(currentIndexChanged(int)),		// connect editors signal to the closer function
                      this, SLOT(commit_and_close_editor()));

              return combo_box;
           }
          else if (rx_bool.indexIn (pattern_description) != -1)			// and booleans
            {
              QComboBox * combo_box = new QComboBox(parent);

              std::vector<std::string> choices;					// list with the different items
              choices.push_back(std::string("true"));				// add true
              choices.push_back(std::string("false"));				// and false

              for (unsigned int i=0; i<choices.size(); ++i)			// add items to the combo box
                combo_box->addItem (tr(choices[i].c_str()), tr(choices[i].c_str()));

              combo_box->setEditable(false);

              connect(combo_box, SIGNAL(currentIndexChanged(int)),		// connect editors signal to the closer function
                      this, SLOT(commit_and_close_editor()));

              return combo_box;
            }
          else
            {
              return QItemDelegate::createEditor(parent, option, index);
            };
        };

      return 0;				// if it is not the column "parameter values", do nothing
    }



    void ParameterDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
    {
      if (index.column() == value_column)
        {
          if (BrowseLineEdit * filename_editor = qobject_cast<BrowseLineEdit *>(editor))
            {
              QString file_name = index.data(Qt::DisplayRole).toString();
              filename_editor->setText(file_name);
            }
          else if (QComboBox * combo_box = qobject_cast<QComboBox *>(editor))
            {
              QRegExp  rx(index.data(Qt::DisplayRole).toString());

              for (int i=0; i<combo_box->count(); ++i)                                  // we look, which index
                if (rx.exactMatch(combo_box->itemText(i)))                              // the data has and set
                  combo_box->setCurrentIndex(i);
            }
          else
            QItemDelegate::setEditorData(editor, index);
        }
    }



    void ParameterDelegate::commit_and_close_editor()
    {
      QWidget * editor = qobject_cast<QWidget *>(sender());
      emit commitData(editor);
      emit closeEditor(editor);
    }



    void ParameterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
    {
      if (index.column() == value_column)
        {
          if (QLineEdit * line_edit = qobject_cast<QLineEdit *>(editor))
            {
              QString line_edit_content = line_edit->text();
              int position = 0;

              // If the editor has a validator, only accept the content if it contains
              // correct input. Otherwise bad cases
              // can happen, when some unfinished number violates the bounds, and
              // the user clicks elsewhere to lose the focus of the editor.
              if (line_edit->validator())
                {
                  if (line_edit->validator()->validate(line_edit_content,position) == QValidator::Acceptable)
                    model->setData(index, line_edit_content);
                }
              else
                model->setData(index, line_edit_content);
            }
          else if (BrowseLineEdit * filename_editor = qobject_cast<BrowseLineEdit *>(editor))
            {
              QString value = filename_editor->text();
              model->setData(index, value);
            }
          else if (QComboBox * combo_box = qobject_cast<QComboBox *>(editor))
            {
              QString value = combo_box->currentText();
              model->setData(index, value);
            }
          else
            QItemDelegate::setModelData(editor, model, index);
        }
    }
  }
}

