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


#ifndef PRMPARAMETERWRITER_H
#define PRMPARAMETERWRITER_H

#include <QTreeWidget>
#include <QTreeWidgetItem>


namespace dealii
{
  /*! @addtogroup ParameterGui
   *@{
   */
  namespace ParameterGui
  {
    /**
     * The PRMParameterWriter class provides an interface to write parameters
     * stored in a QTreeWidget to a file in deal.II's PRM format.
     * This class only writes parameters that deviate from their default values to
     * improve the readability of the created file.
     *
     * @note This class is used in the graphical user interface for the
     * @ref ParameterHandler class.
     * It is not compiled into the deal.II libraries and can not be used by
     * applications using deal.II.
     *
     * @ingroup ParameterGui
     * @author Rene Gassmoeller, 2017
     */
    class PRMParameterWriter
    {
    public:
      /**
       * Constructor.
       * Parameter values from @p tree_widget will be written.
       */
      PRMParameterWriter (QTreeWidget *tree_widget);

      /**
       * This function writes the parameter values stored in <tt>tree_widget</tt>
       * to @p device in the PRM format.
       */
      bool write_prm_file (QIODevice *device);

    private:
      /**
       * This function creates a string that corresponds to the data written
       * to a prm file for a given @p item of <tt>tree_widget</tt>.
       * If the @p item is a parameter it is only written if its value differs
       * from its default value. If the @p item is a subsection, a start element
       * <code>subsection</code> is written and <tt>write_item</tt> is called
       * recursively to write the next <tt>item</tt>.
       * If no items in this subsection differ from their default values then
       * the subsection is not written.
       * @p indentation_level describes the level the current item belongs to.
       * 0 describes a top level item and each subsection increases the level
       * by one.
       *
       */
      QString item_to_string (const QTreeWidgetItem *item,
                              const unsigned int indentation_level);

      /**
       * A pointer to the QTreeWidget structure
       * which stores the parameters.
       */
      QTreeWidget *tree_widget;
    };
  }
  /**@}*/
}


#endif
