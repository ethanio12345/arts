/* Copyright (C) 2002 Patrick Eriksson <Patrick.Eriksson@rss.chalmers.se>
                      Stefan Buehler   <sbuehler@uni-bremen.de>
                            
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



/*===========================================================================
  ===  File description
  ===========================================================================*/

/*!
  \file   m_physics.cc
  \author Patrick Eriksson <Patrick.Eriksson@rss.chalmers.se>
  \date   2002-08-20 

  \brief  Workspace methods of physical character.

  This file includes workspace methods for operations that have some
  connection to basic physics. Example of methods are:  <br>
  1. Setting WSV to hold blackbody radiation. <br>
  2. Conversion to brightness temperature.

  These functions are listed in the doxygen documentation as entries of the
  file auto_md.h.
*/



/*===========================================================================
  === External declarations
  ===========================================================================*/

#include "arts.h"
#include "auto_md.h"
#include "check_input.h"
#include "logic.h"
#include "math_funcs.h"
#include "messages.h"
#include "physics_funcs.h"

extern const Numeric COSMIC_BG_TEMP;



/*===========================================================================
  === The functions (in alphabetical order)
  ===========================================================================*/



//! emissionPlanck
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2005-05-14
*/
void emissionPlanck(
              Vector&   emission,
        const Vector&   f,
        const Numeric&  t )
{
  const Index   n = f.nelem();

  emission.resize(n);

  for( Index i=0; i<n; i++ )
    { emission[i] = planck( f[i], t ); }
}



//! MatrixCBR
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2002-05-28
*/
void MatrixCBR(
        // WS Output:
              Matrix&   m,
        // WS Generic Output Names:
        const String&   m_name,
        // WS Input:
        const Index&    stokes_dim,
        // WS Generic Input:
        const Vector&   f,
        // WS Generic Input Names:
        const String&   f_name )
{
  // To avoid compiler warnings
  String s = f_name;

  const Index n = f.nelem();

  if( n == 0 )
    throw runtime_error( "The given frequency vector is empty." );

  out2 << "  Setting *" << m_name << "* to hold cosmic background "
       << "radiation.\n";

  m.resize(n,stokes_dim);
  m = 0;

  for( Index i=0; i<n; i++ )
    { m(i,0) = planck( f[i], COSMIC_BG_TEMP ); }
}



//! MatrixPlanck
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2002-05-28
*/
void MatrixPlanck(
        // WS Output:
              Matrix&   m,
        // WS Generic Output Names:
        const String&   m_name,
        // WS Input:
        const Index&    stokes_dim,
        // WS Generic Input:
        const Vector&   f,
        const Numeric&  t,
        // WS Generic Input Names:
        const String&   f_name,
        const String&   t_name )
{
  // To avoid compiler warnings
  String s1 = f_name;
  String s2 = t_name;

  const Index n = f.nelem();

  if( n == 0 )
    throw runtime_error( "The given frequency vector is empty." );

  out2 << "  Setting *" << m_name << "* to hold blackbody radiation for a\n"
       << "temperature of " << t << " K.\n";

  m.resize(n,stokes_dim);
  m = 0;

  for( Index i=0; i<n; i++ )
    { m(i,0) = planck( f[i], t ); }
}



//! MatrixToTbByPlanck
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2002-08-11
*/
void MatrixToTbByPlanck(
              Matrix&   y_out,
        const String&   y_out_name,
        const Matrix&   sensor_pos,
        const Matrix&   sensor_los,
        const Vector&   sensor_response_f,
        const Vector&   sensor_response_za,
        const Vector&   sensor_response_aa,
        const Index&    sensor_response_pol,
        const Matrix&   y_in,
        const String&   y_in_name )
{
  Index   nf, npol, nspectra;

  chk_y_with_sensor( nf, npol, nspectra,  y_in(joker,0), y_in_name, sensor_pos,
                     sensor_los, sensor_response_f, sensor_response_za, 
                     sensor_response_aa, sensor_response_pol );

  out2 << "   " << y_out_name << " = inv_of_rj(" << y_in_name << ")\n" ;
 
  const Index   ncols = y_in.ncols();

  // Note that y_in and y_out can be the same matrix
  if ( &y_out != &y_in )
    { y_out.resize(y_in.nrows(),ncols); }

  for( Index iv=0; iv<nf; iv++ )
    {
      for( Index isp=0; isp<nspectra; isp++ )
        {  
          const Index   i0 = nf*npol*isp + iv * npol;

          for( Index ipol=0; ipol<npol; ipol++ )
            {
              for( Index icol=0; icol<ncols; icol++ )
                { 
                  y_out(i0+ipol,icol) = 
                       invplanck( y_in(i0+ipol,icol), sensor_response_f[iv] );
                }
            }
        }
    }
}



//! MatrixToTbByRJ
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2003_07-13
*/
void MatrixToTbByRJ(
              Matrix&   y_out,
        const String&   y_out_name,
        const Matrix&   sensor_pos,
        const Matrix&   sensor_los,
        const Vector&   sensor_response_f,
        const Vector&   sensor_response_za,
        const Vector&   sensor_response_aa,
        const Index&    sensor_response_pol,
        const Matrix&   y_in,
        const String&   y_in_name )
{
  Index   nf, npol, nspectra;

  chk_y_with_sensor( nf, npol, nspectra,  y_in(joker,0), y_in_name, sensor_pos,
                     sensor_los, sensor_response_f, sensor_response_za, 
                     sensor_response_aa, sensor_response_pol );

  out2 << "   " << y_out_name << " = inv_of_rj(" << y_in_name << ")\n" ;
 
  const Index   ncols = y_in.ncols();

  // Note that y_in and y_out can be the same matrix
  if ( &y_out != &y_in )
    { y_out.resize(y_in.nrows(),ncols); }


  // Here we try to save time by determining the scaling from radiances
  // to brightness temperature for each frequency, and applying this
  // scaling on each repition for that frequency. Note that relationship
  // between radiance and Tb is linear for a given frequency.

  for( Index iv=0; iv<nf; iv++ )
    {
      const Numeric   scfac = invrayjean( 1, sensor_response_f[iv] );

      for( Index isp=0; isp<nspectra; isp++ )
        {  
          const Index   i0 = nf*npol*isp + iv * npol;

          for( Index ipol=0; ipol<npol; ipol++ )
            {
              for( Index icol=0; icol<ncols; icol++ )
                { y_out(i0+ipol,icol) = scfac * y_in(i0+ipol,icol); }
            }
        }
    }
}



//! MatrixUnitIntensity
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2005-05-18
*/
void MatrixUnitIntensity(
        // WS Output:
              Matrix&   m,
        // WS Generic Output Names:
        const String&   m_name,
        // WS Input:
        const Index&    stokes_dim,
        // WS Generic Input:
        const Vector&   f,
        // WS Generic Input Names:
        const String&   f_name )
{
  // To avoid compiler warnings
  String s1 = f_name;

  const Index n = f.nelem();

  if( n == 0 )
    throw runtime_error( "The given frequency vector is empty." );

  out2 << "  Setting *" << m_name << "* to hold unpolarised radiation with\n"
       << "an intensity of 1.\n";

  m.resize(n,stokes_dim);
  m = 0;

  for( Index i=0; i<n; i++ )
    { m(i,0) = 1.0; }
}



//! SparseToTbByPlanck
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2005-06-03
*/
void SparseToTbByPlanck(
              Sparse&   y_out,
        const String&   y_out_name,
        const Matrix&   sensor_pos,
        const Matrix&   sensor_los,
        const Vector&   sensor_response_f,
        const Vector&   sensor_response_za,
        const Vector&   sensor_response_aa,
        const Index&    sensor_response_pol,
        const Sparse&   y_in,
        const String&   y_in_name )
{
  Index   nf, npol, nspectra;

  Vector ytest(y_in.nrows());

  chk_y_with_sensor( nf, npol, nspectra, ytest, y_in_name, sensor_pos,
                     sensor_los, sensor_response_f, sensor_response_za, 
                     sensor_response_aa, sensor_response_pol );

  out2 << "   " << y_out_name << " = inv_of_rj(" << y_in_name << ")\n" ;
 
  const Index   ncols = y_in.ncols();

  // Note that y_in and y_out can be the same matrix
  if ( &y_out != &y_in )
    { y_out.resize(y_in.nrows(),ncols); }

  for( Index iv=0; iv<nf; iv++ )
    {
      for( Index isp=0; isp<nspectra; isp++ )
        {  
          const Index   i0 = nf*npol*isp + iv * npol;

          for( Index ipol=0; ipol<npol; ipol++ )
            {
              for( Index icol=0; icol<ncols; icol++ )
                { 
                  if( y_out(i0+ipol,icol) > 0 )
                    {
                      y_out.rw(i0+ipol,icol) = 
                       invplanck( y_in(i0+ipol,icol), sensor_response_f[iv] );
                    }
                }
            }
        }
    }
}



//! SparseToTbByRJ
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2005-06-03
*/
void SparseToTbByRJ(
              Sparse&   y_out,
        const String&   y_out_name,
        const Matrix&   sensor_pos,
        const Matrix&   sensor_los,
        const Vector&   sensor_response_f,
        const Vector&   sensor_response_za,
        const Vector&   sensor_response_aa,
        const Index&    sensor_response_pol,
        const Sparse&   y_in,
        const String&   y_in_name )
{
  Index   nf, npol, nspectra;

  Vector ytest(y_in.nrows());

  chk_y_with_sensor( nf, npol, nspectra, ytest, y_in_name, sensor_pos,
                     sensor_los, sensor_response_f, sensor_response_za, 
                     sensor_response_aa, sensor_response_pol );

  out2 << "   " << y_out_name << " = inv_of_rj(" << y_in_name << ")\n" ;
 
  const Index   ncols = y_in.ncols();

  // Note that y_in and y_out can be the same matrix
  if ( &y_out != &y_in )
    { y_out.resize(y_in.nrows(),ncols); }


  // Here we try to save time by determining the scaling from radiances
  // to brightness temperature for each frequency, and applying this
  // scaling on each repition for that frequency. Note that relationship
  // between radiance and Tb is linear for a given frequency.

  for( Index iv=0; iv<nf; iv++ )
    {
      const Numeric   scfac = invrayjean( 1, sensor_response_f[iv] );

      for( Index isp=0; isp<nspectra; isp++ )
        {  
          const Index   i0 = nf*npol*isp + iv * npol;

          for( Index ipol=0; ipol<npol; ipol++ )
            {
              for( Index icol=0; icol<ncols; icol++ )
                { 
                  if( y_out(i0+ipol,icol) > 0 )
                    { y_out.rw(i0+ipol,icol) = scfac * y_in(i0+ipol,icol); }
                }
            }
        }
    }
}



//! Tensor6ToTbByPlanck
/*! 
   See the the online help (arts -d FUNCTION_NAME)
*/
void Tensor6ToTbByPlanck( // WS Generic Output:
                         Tensor6&   y_out,
                         // WS Generic Output Names:
                         const String&   y_out_name,
                         // WS Specific Input: 
                         const Index& scat_f_index,
                         const Vector&   f_grid,
                         // WS Generic Input:
                         const Tensor6&   y_in,
                         // WS Generic Input Names:
                         const String&   y_in_name)
                
{
  // Some lengths
  const Index nv    = y_in.nvitrines();
  const Index ns    = y_in.nshelves();
  const Index nb    = y_in.nbooks();
  const Index np    = y_in.npages();
  const Index nr    = y_in.nrows();
  const Index nc    = y_in.ncols();
  
  Numeric f = f_grid[scat_f_index];
   
  // Note that y_in and y_out can be the same matrix
  if ( &y_out != &y_in )
    { 
      y_out.resize(nv, ns, nb, np, nr, nc);
    }
  
  out2 << "   " << y_out_name << " = inv_of_planck(" << y_in_name << ")\n" ;
   
  for( Index iv = 0; iv < nv; ++ iv )
    {
      for( Index is = 0; is < ns; ++ is )
        {
          for( Index ib = 0; ib < nb; ++ ib )
            {
              for( Index ip = 0; ip < np; ++ ip )
                {
                  for( Index ir = 0; ir < nr; ++ ir )
                    {
                      for( Index ic = 0; ic < nc; ++ ic)
                        {
                          y_out(iv, is, ib, ip, ir, ic) = 
                           invplanck( y_in (iv, is, ib, ip, ir, ic ), f);  
                        }
                    }
                }
            }
        }
    }
}




//! VectorToTbByPlanck
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2003-07-13
*/
void VectorToTbByPlanck(
              Vector&   y_out,
        const String&   y_out_name,
        const Matrix&   sensor_pos,
        const Matrix&   sensor_los,
        const Vector&   sensor_response_f,
        const Vector&   sensor_response_za,
        const Vector&   sensor_response_aa,
        const Index&    sensor_response_pol,
        const Vector&   y_in,
        const String&   y_in_name )
{
  Index   nf, npol, nspectra;

  chk_y_with_sensor( nf, npol, nspectra,  y_in, y_in_name, sensor_pos,
                     sensor_los, sensor_response_f, sensor_response_za, 
                     sensor_response_aa, sensor_response_pol );

  out2 << "   " << y_out_name << " = inv_of_planck(" << y_in_name << ")\n" ;

  // Note that y_in and y_out can be the same vector
  if ( &y_out != &y_in )
    { y_out.resize(nf*npol*nspectra); }

  for( Index iv=0; iv<nf; iv++ )
    {
      for( Index isp=0; isp<nspectra; isp++ )
        {  
          const Index   i0 = nf*npol*isp + iv * npol;
          for( Index ipol=0; ipol<npol; ipol++ )
            { 
              y_out[i0+ipol] = invplanck(y_in[i0+ipol],sensor_response_f[iv]); 
           }
        }
    }
}



//! VectorToTbByRJ
/*! 
   See the the online help (arts -d FUNCTION_NAME)

   \author Patrick Eriksson
   \date   2003-07-13
*/
void VectorToTbByRJ(
              Vector&   y_out,
        const String&   y_out_name,
        const Matrix&   sensor_pos,
        const Matrix&   sensor_los,
        const Vector&   sensor_response_f,
        const Vector&   sensor_response_za,
        const Vector&   sensor_response_aa,
        const Index&    sensor_response_pol,
        const Vector&   y_in,
        const String&   y_in_name )
{
  Index   nf, npol, nspectra;

  chk_y_with_sensor( nf, npol, nspectra,  y_in, y_in_name, sensor_pos,
                     sensor_los, sensor_response_f, sensor_response_za, 
                     sensor_response_aa, sensor_response_pol );

  out2 << "   " << y_out_name << " = inv_of_rj(" << y_in_name << ")\n" ;

  // Note that y_in and y_out can be the same vector
  if ( &y_out != &y_in )
    { y_out.resize(nf*npol*nspectra); }

  // To be as fast as possible, there is a special version for cases where
  // the frequenciers not are repeated.
  
  if( nf == y_in.nelem() )
    {
      // Here we just loop the frequencies and call invrayjean
      for( Index iv=0; iv<nf; iv++ )
        { y_out[iv] = invrayjean( y_in[iv], sensor_response_f[iv] ); }
    }

  else
    {
      // Here we try to save time by determining the scaling from radiances
      // to brightness temperature for each frequency, and applying this
      // scaling on each repition for that frequency. Note that relationship
      // between radiance and Tb is linear for a given frequency.

      for( Index iv=0; iv<nf; iv++ )
        {
          const Numeric   scfac = invrayjean( 1, sensor_response_f[iv] );

          for( Index isp=0; isp<nspectra; isp++ )
            {  
              const Index   i0 = nf*npol*isp + iv * npol;
              for( Index ipol=0; ipol<npol; ipol++ )
                { y_out[i0+ipol] = scfac * y_in[i0+ipol]; }
            }
        }
    }
}



