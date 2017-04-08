// ---------------------------------------------------------------------
//
// Copyright (C) 2017 by Rene Gassmoeller
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

#include "prm_parameter_writer.h"


namespace dealii
{
  namespace ParameterGui
  {
    PRMParameterWriter::PRMParameterWriter(QTreeWidget *tree_widget)
                      : tree_widget(tree_widget)
    {
    }



    bool PRMParameterWriter::write_prm_file(QIODevice *device)
    {
      // loop over the elements
      for (int i = 0; i < tree_widget->topLevelItemCount(); ++i)
        {
        const QString item_string = item_to_string(tree_widget->topLevelItem(i),0);
        device->write(item_string.toAscii());
        }

      return true;
    }



    QString PRMParameterWriter::item_to_string(const QTreeWidgetItem *item,
                                               const unsigned int indentation_level)
    {
      QString item_string;

      // if the entry has no children we have a parameter
      if (item->childCount() == 0)
        {
          bool non_default_value;

          if (item->text(5).startsWith("[Double"))
            non_default_value = item->data(1,Qt::DisplayRole).toReal() != item->data(2,Qt::DisplayRole).toReal();
          else
            non_default_value = item->data(1,Qt::DisplayRole).toString() != item->data(2,Qt::DisplayRole).toString();


          if (non_default_value)
            {
              for (unsigned int i=0; i<indentation_level;++i)
                item_string.push_back("  ");

              item_string.push_back("set " + item->text(0) + " = " + item->data(1,Qt::EditRole).toString() + "\n");
            }
        }
      else
        {
          for (int i = 0; i < item->childCount(); ++i)
            item_string.push_back(item_to_string(item->child(i),indentation_level+1));

          if (!item_string.isEmpty())
            {
              item_string.push_front("subsection " + item->text(0).toAscii() + "\n");
              for (unsigned int i=0; i<indentation_level;++i)
                item_string.push_front("  ");


              for (unsigned int i=0; i<indentation_level;++i)
                item_string.push_back("  ");
              item_string.push_back("end\n\n");
            }
        }

      return item_string;
    }
  }
}
