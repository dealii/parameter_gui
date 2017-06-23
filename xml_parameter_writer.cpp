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


#include "xml_parameter_writer.h"

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

namespace dealii
{
  namespace ParameterGui
  {
    XMLParameterWriter::XMLParameterWriter(QTreeWidget *tree_widget)
                      : tree_widget(tree_widget)
    {
      xml.setAutoFormatting(true);
    }



    bool XMLParameterWriter::write_xml_file(QIODevice *device)
    {
      xml.setDevice(device);

      // write the head <?xml ... ?>
      xml.writeStartDocument();

      // write the root element <ParameterHandler>
      xml.writeStartElement("ParameterHandler");

      // loop over the elements and write them
      for (int i = 0; i < tree_widget->topLevelItemCount(); ++i)
        write_item(tree_widget->topLevelItem(i));

      // close the first element
      xml.writeEndDocument();

      return true;
    }



    void XMLParameterWriter::write_item(QTreeWidgetItem *item)
    {
      // store the element name
      QString tag_name = mangle(item->text(0));

      // and write <tag_name> to the file
      xml.writeStartElement(tag_name);

      // if the "value"-entry of this item is not empty, write a parameter
      if (!item->text(1).isEmpty())
        {
          xml.writeTextElement("value", item->data(1,Qt::EditRole).toString());
          xml.writeTextElement("default_value", item->text(2));
          xml.writeTextElement("documentation", item->text(3));
          xml.writeTextElement("pattern", item->text(4));
          xml.writeTextElement("pattern_description", item->text(5));
        };

      // go over the childrens recursively
      for (int i = 0; i < item->childCount(); ++i)
        write_item(item->child(i));

      // write closing </tag_name>
      xml.writeEndElement();
    }



    QString XMLParameterWriter::mangle (const QString &s)
    {
      // this function is copied from the ParameterHandler class and adapted to mangle QString
      std::string  s_temp (s.toStdString());

      std::string u;
      u.reserve (s_temp.size());

      static const std::string allowed_characters
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

      // for all parts of the string, see if it is an allowed character or not
      for (unsigned int i=0; i<s_temp.size(); ++i)
        if (allowed_characters.find (s_temp[i]) != std::string::npos)
          u.push_back (s_temp[i]);
        else
          {
	    u.push_back ('_');
	    static const char hex[16]
	      = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	    u.push_back (hex[static_cast<unsigned char>(s_temp[i])/16]);
	    u.push_back (hex[static_cast<unsigned char>(s_temp[i])%16]);
          }

      QString  v (u.c_str());

      return v;
    }
  }
}
