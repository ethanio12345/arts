/*
  ---------------------------------------
  This is the global header file for ARTS
  ---------------------------------------
  This file is included by each and every ARTS source file. It must
  therefor not contain stuff that should not always be present.

  For Documentation on ARTS see: FIXME: Complete this!

  History:
  SAB 16.05.99 Started with version 1.0.
*/

#ifndef arts_h
#define arts_h


//----------< First of all, include the configuration header >----------
// This header is generated by the configure script.
#if HAVE_CONFIG_H
#include "config.h"
#endif		
// FIXME: Put an else clause here that stops the compilation with an
// error message.

//----------< Standard library header files: >----------

#include <iostream>		// Standard stream library
#include <iomanip>
#include <fstream>
#include <strstream>
#include <string>		// Standard string library
#include <map>
#include <stdarg.h>
#include <math.h>
#include <cfloat>
#include <typeinfo>
#include <stdexcept>
#include <algorithm>
#include <ctype.h>
// #include <valarray>		// Valarray class from the STL
                                // This should not be needed explicitly


// Make the names of the standard library visible everywhere.
// SAB 29.01.2000: Better not to use this, if matrices and vectors are
// used the proper names are anyway made visible by vecmat.h.
// using namespace std;       


//--------------------< Set precision >--------------------
/** The type to use for all floating point numbers. You should never
    use float or double explicitly, unless you have a very good
    reason. Always use this type instead.  */
typedef double Numeric;

//--------------------< Set NDEBUG? >--------------------
/* Define this in order to turn off all debuggin feature (TNT range
    checking, assertions, ...) */
#undef NDEBUG
// #define NDEBUG 1

// C Assert macro:
// Could be moved to config.h in the future. This must be included
// *after* the definition of NDEBUG, since NDEBUG changes the
// behaviour of assert (assert does nothing if NDEBUG is set).
#include <cassert>


//-----------< define a quick output for std::vector<> >----------
/* A quick hack output for std::vector<>. This is only for
    debugging purposes.
    @return As for all output operator functions, the output stream is 
            returned.
    @param  os Output stream
    @param  v  Vector to print                      
    @author SAB  
    @version 1 */  
// template<class T>
// ostream& operator<<(ostream& os, const std::vector<T>& v)
// {
//   for (std::vector<T>::const_iterator i=v.begin(); i<v.end()-1; ++i)
//     os << *i << ", ";
//   os << *(v.end()-1);
//   return os;
// }




//---------------< Our own header files: >---------------
/* It's not a good idea to include our own headers here, since then
   everything will depend on everything else. So we have to always
   include the headers we need. */
// #include "exceptions.h"
// #include "messages.h"
// #include "token_4.h"
// #include "tracer.h"
// #include "workspace.h"
// #include "make_vector.h"

//---------------< Global variable declarations >---------------
// (Definitions of these are in FIXME: where?)


//---------------< Global function declarations: >---------------

/* The function execute executes workspace methods one by one.*/
//void execute(WsLimbBasic& ws, const MethodBatch& mb);


//
// Physical constants are now in constants.cc
//



#endif // arts_h



