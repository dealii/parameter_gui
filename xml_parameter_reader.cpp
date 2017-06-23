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


#include "xml_parameter_reader.h"

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

namespace dealii
{
  namespace ParameterGui
  {
    XMLParameterReader::XMLParameterReader(QTreeWidget *tree_widget)
                      : tree_widget(tree_widget)
    {
      QStyle * style = tree_widget->style();

      subsection_icon.addPixmap(style->standardPixmap(QStyle::SP_DirClosedIcon), QIcon::Normal, QIcon::Off);
      subsection_icon.addPixmap(style->standardPixmap(QStyle::SP_DirOpenIcon), QIcon::Normal, QIcon::On);

      parameter_icon.addPixmap(style->standardPixmap(QStyle::SP_FileIcon));
    }



    bool XMLParameterReader::read_xml_file(QIODevice *device)
    {
      xml.setDevice(device);

      // We look for a StartElement "ParameterHandler"
      // and start parsing after this.
      //  <ParameterHandler>
      //   <subsection>
      //    ...
      //   </subsection>
      //  </ParameterHandler>

      while (xml.readNext() != QXmlStreamReader::Invalid)
        {
          if (xml.isStartElement())
            if (xml.name() == "ParameterHandler")
              {
                parse_parameters();

                return !xml.error();;
              };
        };

      xml.raiseError(QObject::tr("The file is not an ParameterHandler XML file."));

      return !xml.error();
    }



    QString XMLParameterReader::error_string() const
    {
      return QObject::tr("%1\nLine %2, column %3")
             .arg(xml.errorString())
             .arg(xml.lineNumber())
             .arg(xml.columnNumber());
    }



    void XMLParameterReader::parse_parameters()
    {
      Q_ASSERT(xml.isStartElement() && xml.name() == "ParameterHandler");

      // go to the next <start_element>
      while (xml.readNext() != QXmlStreamReader::Invalid)
        {
          // if it is the closing element of ParameterHandler, break the loop
          if (xml.isEndElement() && xml.name() == "ParameterHandler")
            break;

          // if it is a start element it must be a subsection or a parameter
          if (xml.isStartElement())
            read_subsection_element(0);
        };
    }



    void XMLParameterReader::read_subsection_element(QTreeWidgetItem *parent)
    {
      // The structure of the parameter file is assumed to be of the form
      //
      //  <subsection>
      //    <subsection>
      //      ...
      //        <parameter>
      //          <value> ... </value>
      //          ...
      //          <pattern_description> ... </pattern_description>
      //        </parameter>
      //        <parameter>
      //        ...
      //        </parameter>
      //        ...
      //    </subsection>
      //    <subsection>
      //      ...
      //    </subsection>
      //    ...
      //  </subsection>
      //
      // Any subsection has a user-specified name also any parameter, but we do not know
      // the userspecified names and we can not assume anything. So, when parsing the file,
      // we do not know, if the actual <start_element> is a <subsection> or a <parameter>
      // in a subsection. To decide, if the element is a subsection- or a parameter-name,
      // we assume, that if the next <start_element> is <value>, we have a <parameter>
      // and a parameter has the entries <value>, <default_value>, <documentation>,
      // <pattern> and <pattern_description>

      // the actual element is <subsection>
      Q_ASSERT(xml.isStartElement());

      // create a new subsection in the tree
      QTreeWidgetItem * subsection = create_child_item(parent);

      subsection->setIcon(0, subsection_icon);
      subsection->setText(0, demangle(xml.name().toString()));

      // the folder is not expanded
      tree_widget->setItemExpanded(subsection, 0);

      // read the next element
      while (xml.readNext() != QXmlStreamReader::Invalid)
        {
          // if the next element is </subsection>, break the loop
          if (xml.isEndElement())
            break;

          if (xml.isStartElement())
            {
              // it can be <value>, then we have found a parameter,
              if (xml.name() == "value")
                {
                  // values can be edited
                  subsection->setFlags(subsection->flags() | Qt::ItemIsEditable);
                  read_parameter_element (subsection);
                }
              // or it can be a new <subsection>
              else
                {
                  // subsections can not be edited
                  subsection->setFlags(subsection->flags() | Qt::NoItemFlags);
                  read_subsection_element (subsection);
                };
            };
        };
    }



    void XMLParameterReader::read_parameter_element(QTreeWidgetItem *parent)
    {
      // the actual element is <value>,
      // then we have found a parameter-item
      Q_ASSERT(xml.isStartElement() && xml.name() == "value");

      QString value = xml.readElementText();
      // store as text to the item
      parent->setText(1, value);
      // change the icon of parent
      parent->setIcon(0, parameter_icon);

      // go to the next <start_element>
      while (xml.readNext() != QXmlStreamReader::Invalid)
        {
          if (xml.isStartElement())
            {
              // if it is <default_value> store it
              if (xml.isStartElement() && xml.name() == "default_value")
                {
                  QString default_value = xml.readElementText();
                  parent->setText(2, default_value);
                }
              // if it is <documentation> store it
              else if (xml.isStartElement() && xml.name() == "documentation")
                {
                  QString documentation = xml.readElementText();
                  parent->setText(3, documentation);
                }
              // if it is <pattern> store it as text,
              // we only need this value for writing back to XML later
              else if (xml.isStartElement() && xml.name() == "pattern")
                {
                  QString pattern = xml.readElementText();
                  parent->setText(4, pattern);
                }
              // if it is <pattern_description> store it as text
              else if (xml.isStartElement() &&  xml.name() == "pattern_description")
                {
                  QString pattern_description = xml.readElementText();

                  // show the type and default
                  // in the StatusLine when
                  // hovering over column 0 or 1
                  parent->setText(5, pattern_description);
                  parent->setStatusTip(0, "Type: " + pattern_description + "   Default: " + parent->text(2));
                  parent->setStatusTip(1, "Type: " + pattern_description + "   Default: " + parent->text(2));

                  // in order to store values as correct data types,
                  // we check the following types in the pattern_description:
                  QRegExp  rx_string("\\b(Anything|FileName|DirectoryName|Selection|List|MultipleSelection)\\b"),	
                           rx_integer("\\b(Integer)\\b"),
                           rx_double("\\b(Float|Floating|Double)\\b"),
                           rx_bool("\\b(Selection true|false)\\b");

                  // store the type "Anything" or "Filename" as a QString
                  if (rx_string.indexIn (pattern_description) != -1)
                    {
                      QString value = parent->text(1);

                      parent->setData(1, Qt::EditRole, value);
                      parent->setData(1, Qt::DisplayRole, value);
                    }
                  // store the type "Integer" as an int
                  else if (rx_integer.indexIn (pattern_description) != -1)
                    {
                      QString text = parent->text(1);

                      bool ok = true;
                      int value = text.toInt(&ok);

                      if (ok)
                        {
                          parent->setData(1, Qt::EditRole, value);
                          parent->setData(1, Qt::DisplayRole, value);
                        }
                      else
                        xml.raiseError(QObject::tr("Cannot convert integer type to integer!"));
                    }
                  // store the type "Double" as an double
                  else if (rx_double.indexIn (pattern_description) != -1)
                    {
                      QString text = parent->text(1);

                      bool ok  = true;

                      double value = text.toDouble(&ok);

                      if (ok)
                        {
                          parent->setData(1, Qt::EditRole, value);
                          parent->setData(1, Qt::DisplayRole, value);
                        }
                      else
                        xml.raiseError(QObject::tr("Cannot convert double type to double!"));
                    }

                  // store the type "Bool" as an boolean
                  if (rx_bool.indexIn (pattern_description) != -1)
                    {
                      QRegExp  test(parent->text(1));

                      bool value = true;

                      if (test.exactMatch("true"))
                        value = true;
                      else if (test.exactMatch("false"))
                        value = false;
                      else
                        xml.raiseError(QObject::tr("Cannot convert boolen type to boolean!"));

                      // this is needed because we use for booleans the standard delegate
                      parent->setText(1, "");
                      parent->setData(1, Qt::EditRole, value);
                      parent->setData(1, Qt::DisplayRole, value);
                    }

                  break;
                }
              // if there is any other element, raise an error
              else
                {
                  xml.raiseError(QObject::tr("Incomplete or unknown Parameter!"));
                  break;
                }
            }
        }
    }



    QTreeWidgetItem *XMLParameterReader::create_child_item(QTreeWidgetItem *item)
    {
      // create a new child-item
      QTreeWidgetItem * child_item;

      // if item is not empty, append the new item as a child
      if (item)
        child_item = new QTreeWidgetItem(item);
      // otherwise create a new item in the tree
      else
        child_item = new QTreeWidgetItem(tree_widget);

      // set xml.tag_name as data
      child_item->setData(0, Qt::DisplayRole, xml.name().toString());
      child_item->setText(0, xml.name().toString());

      return child_item;
    }



    QString XMLParameterReader::demangle (const QString &s)
    {
      // this function is copied from the ParameterHandler class
      std::string  s_temp (s.toStdString());

      std::string u;
      u.reserve (s_temp.size());

      for (unsigned int i=0; i<s_temp.size(); ++i)
        if (s_temp[i] != '_')
          u.push_back (s_temp[i]);
        else
          {
            Q_ASSERT (i+2 < s_temp.size());

            unsigned char c = 0;
            switch (s_temp[i+1])
            {
            case '0':  c = 0 * 16;  break;
            case '1':  c = 1 * 16;  break;
            case '2':  c = 2 * 16;  break;
            case '3':  c = 3 * 16;  break;
            case '4':  c = 4 * 16;  break;
            case '5':  c = 5 * 16;  break;
            case '6':  c = 6 * 16;  break;
            case '7':  c = 7 * 16;  break;
            case '8':  c = 8 * 16;  break;
            case '9':  c = 9 * 16;  break;
            case 'a':  c = 10 * 16;  break;
            case 'b':  c = 11 * 16;  break;
            case 'c':  c = 12 * 16;  break;
            case 'd':  c = 13 * 16;  break;
            case 'e':  c = 14 * 16;  break;
            case 'f':  c = 15 * 16;  break;
            default:
              Q_ASSERT (false);
            }
	switch (s_temp[i+2])
	{
	case '0':  c += 0;  break;
	case '1':  c += 1;  break;
	case '2':  c += 2;  break;
	case '3':  c += 3;  break;
	case '4':  c += 4;  break;
	case '5':  c += 5;  break;
	case '6':  c += 6;  break;
	case '7':  c += 7;  break;
	case '8':  c += 8;  break;
	case '9':  c += 9;  break;
	case 'a':  c += 10;  break;
	case 'b':  c += 11;  break;
	case 'c':  c += 12;  break;
	case 'd':  c += 13;  break;
	case 'e':  c += 14;  break;
	case 'f':  c += 15;  break;
	default:
	  Q_ASSERT (false);
	}

	u.push_back (static_cast<char>(c));

	// skip the two characters
	i += 2;
          }

      QString  v (u.c_str());

      return v;
    }
  }
}

