/* Copyright (C) 2000 Stefan Buehler <sbuehler@uni-bremen.de>

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

/*
  This file contains utility routines for IO files.
  SAB 05.02.2000
 */
#ifndef file_h
#define file_h

#include <iostream>
#include "vecmat.h"

/**
   Open a file for writing. If the file cannot be opened, the
   exception IOError is thrown. 
   @param     file File pointer 
   @param     name Name of the file to open
   @author    Stefan Buehler
   @version   1
   @exception ios_base::failure Could for example mean that the
                      directory is read only. */
void open_output_file(ofstream& file, const string& name);
//  throw (ios::failure);    Does not yet work in egcs

/**
   Open a file for reading. If the file cannot be opened, the
   exception IOError is thrown. 
   @param     file File pointer 
   @param     name Name of the file to open
   @author    Stefan Buehler
   @version   1
   @exception ios_base::failure Somehow the file cannot be opened. */
void open_input_file(ifstream& file, const string& name);
//  throw (ios::failure);  Does not yet work in egcs


/**
   Read an ASCII stream and append the contents to the string array
   text.  TEXT IS NOT OVERWRITTEN, BUT APPENDED!

   @param text Output. The contents fo the file
   @param is Stream from which to read
   @exception IOError Some error occured during the read
   @version   1
   @author Stefan Buehler */
void read_text_from_stream(ARRAY<string>& text, istream& is);

/**
   Reads an ASCII file and appends the contents to the string vector
   text. This uses the function @see read_text_from_stream. TEXT IS
   NOT OVERWRITTEN, BUT APPENDED!  

   @param text Output. The contents fo the file
   @param  filename Name of file to read
   @exception IOError
   @version   1
   @author Stefan Buehler */
void read_text_from_file(ARRAY<string>& text, const string& name);


/** 
    Replace all occurances of `what' in `s' with `with'.

    @param s Output. The string to act on.
    @param what The string to replace.
    @param with The replacement.
    
    @author Stefan Buehler */
void replace_all(string& s, const string& what, const string& with);


//-------------------------< MATRIX/VECTOR IO Routines >-------------------------



/** A helper function that writes an array of matrix to a stream. This
    is the generic output function for VECTORs, MATRIXs, and ARRAYof
    both. All these are converted first to ARRAYofMATRIX, and then
    written by this function.

    @param os   Output. The stream to write to.
    @param am    The matrix to write.

    @author Stefan Buehler */
void write_array_of_matrix_to_stream(ostream& os,
                                     const ARRAYofMATRIX& am);


/** A helper function that writes an array of matrix to a file. Uses
    write_array_of_matrix_to_stream. 

    @param filename    The name of the file.
    @param am          The array of matrix to write.

    @author Stefan Buehler */
void write_array_of_matrix_to_file(const string& filename,
                                   const ARRAYofMATRIX& am);


/** A helper function that reads an array of matrix from a stream.

    @param am   Output. The array of matrix to read.
    @param is   Output. The input stream.

    @author Stefan Buehler */
void read_array_of_matrix_from_stream(ARRAYofMATRIX& am,
                                      istream& is);



/** A helper function that reads an array of matrix from a file. 
    Uses read_array_of_matrix_from_stream.

    @param am        Output. The array of matrix to read.
    @param filename  The name of the file to read.

    @author Stefan Buehler */
void read_array_of_matrix_from_file(ARRAYofMATRIX& am,
                                    const string& filename);






#endif
