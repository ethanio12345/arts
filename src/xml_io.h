/* Copyright (C) 2002 Oliver Lemke <olemke@uni-bremen.de>

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */


////////////////////////////////////////////////////////////////////////////
//   File description
////////////////////////////////////////////////////////////////////////////
/*!
  \file   xml_io.h
  \author Oliver Lemke <olemke@uni-bremen.de>
  \date   Fri May 10 09:43:57 2002

  \brief This file contains basic functions to handle XML data files.

*/


#ifndef xml_io_h
#define xml_io_h

#include <iosfwd>
#include "mystring.h"

////////////////////////////////////////////////////////////////////////////
//   Default file names
////////////////////////////////////////////////////////////////////////////

void
filename_xml (String&  filename,
              const String&  varname);


////////////////////////////////////////////////////////////////////////////
//   Generic IO routines for XML files
////////////////////////////////////////////////////////////////////////////

template<typename T> void
xml_read_from_file (const String& filename,
                          T&      data);

template<typename T> void
xml_write_to_file (const String& filename,
                   const      T& data);

#endif
