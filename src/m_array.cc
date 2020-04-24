/* Copyright (C) 2020
 * Richard Larsson <larsson@mps.mpg.de>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA. */

/**
 * @file   m_array.cc
 * @author Richard Larsson
 * @date   2020-04-24
 * 
 * @brief  Stuff related to manipulating arrays
 */


#include "artstime.h"
#include "matpackI.h"
#include "messages.h"
#include "sorting.h"


template <class T>
Array<T> TimeSortTemplate(const Array<T>& arr, const ArrayOfTime& time_stamps)
{
  // Size of problem
  const Index n=time_stamps.nelem();
  if (arr.nelem() not_eq n)
    throw std::runtime_error("Cannot sort, time array does not agree with sorting array size");
  
  // Sorted index
  ArrayOfIndex sortings(n);
  get_sorted_indexes(sortings, time_stamps);
  
  // Fill the data into a njew array
  Array<T> out(n);
  for (Index i=0; i<n; i++)
    out[i] = arr[sortings[i]];
  
  return out;
}

#define TIME_SORT_MACRO(VAR) \
void TimeSort(VAR & out, const ArrayOfTime& time_stamps, const VAR & in, const Verbosity&) \
{out = TimeSortTemplate(in, time_stamps);}

TIME_SORT_MACRO(ArrayOfTime)
TIME_SORT_MACRO(ArrayOfVector)

#undef TIME_SORT_MACRO

template <class T>
Array<T> FlattenArrayTemplate(const Array<Array<T>>& in)
{
  // Size of problem
  Index n=0;
  for (auto& array: in)
    n += array.nelem();
  
  // Allocate output
  Array<T> out(n);
  
  // Assignment
  Index i=0;
  for (auto& array: in) {
    for (auto& val: array) {
      out[i] = val;
      i++;
    }
  }
  
  return out;
}

#define FLATTEN_MACRO(VAR) \
void Flatten(VAR & out, const Array< VAR > & in, const Verbosity&) \
{out = FlattenArrayTemplate(in);}

FLATTEN_MACRO(ArrayOfTime)
FLATTEN_MACRO(ArrayOfVector)

#undef FLATTEN_MACRO

void Flatten(Matrix& out, const ArrayOfVector& in, const Verbosity&)
{
  if (in.nelem() == 0) {
    out = Matrix(0, 0);
  } else {
    const Index n = in.nelem();
    const Index m = in[0].nelem();
    
    if (not std::all_of(in.cbegin(), in.cend(), [m](auto& v){return m == v.nelem();}))
      throw std::runtime_error("Can only flatten array of same length data");
    
    out = Matrix(n, m);
    for (Index i=0; i<n; i++)
      out(i, joker) = in[i];
  }
}
