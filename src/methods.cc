/* Implementation of MdRecord and of the compound to hold the
   records. See methods.h for more documentation. */

#include "arts.h"
#include "make_array.h"
#include "workspace.h"
#include "methods.h"

// Some #defines and typedefs to make the records better readable:
#define NAME(x) x
#define DESCRIPTION(x) x
#define OUTPUT   make_array<size_t> 
#define INPUT    make_array<size_t> 
#define GOUTPUT  make_array<size_t> 
#define GINPUT   make_array<size_t> 
#define KEYWORDS make_array<string>
#define TYPES    make_array<TokValType>

/** The lookup information for the workspace methods. */
ARRAY<MdRecord> md_data;

/** The map associated with md_data. */
std::map<string, size_t> MdMap;


/** Initializes the method lookup data. */
void define_md_data()
{
  // Initialize to zero, just in case:
  md_data.clear();

  /* Here's an empty template record entry:

  md_data.push_back
    ( MdRecord
      ( NAME(""),
	DESCRIPTION(""),
	OUTPUT(),
	INPUT(),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS(""),
	TYPES()
	));
  */


//======================================================================
//=== IO methods
//======================================================================

//
//=== Scalars
//
  md_data.push_back
    ( MdRecord
      ( NAME("IntSet"),
	DESCRIPTION("Sets an integer workspace variable to the given value."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(int_t),
	GINPUT(),
	KEYWORDS("value"),
	TYPES(int_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("NumericSet"),
	DESCRIPTION("Sets a workspace variable of type Numeric to a value."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(Numeric_),
	GINPUT(),
	KEYWORDS("value"),
	TYPES(Numeric_t)));



//
//=== Vector initialization
//

  md_data.push_back
    ( MdRecord
      ( NAME("VectorSet"),
	DESCRIPTION("Creates a workspace vector with the specified length\n"
                    "and initializes the vector with the given value."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(VECTOR_),
	GINPUT(),
	KEYWORDS("length", "value"),
	TYPES(int_t, Numeric_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("VectorLinSpace"),
	DESCRIPTION("Creates a linearly spaced vector with defined spacing.\n"
                    "Format: VectorLinSpace(x){start,stop,step}\n"
		    "The first element of x is always start.\n"
		    "The next value is start+step etc.\n"
		    "Note that the last value can deviate from stop.\n"
		    "The step can be both positive and negative."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(VECTOR_),
	GINPUT(),
	KEYWORDS("start", "stop", "step"),
	TYPES(Numeric_t, Numeric_t, Numeric_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("VectorNLinSpace"),
	DESCRIPTION("Creates a linearly spaced vector with defined length.\n"
		    "The length must be larger than 1."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(VECTOR_),
	GINPUT(),
	KEYWORDS("start", "stop", "n"),
	TYPES(Numeric_t, Numeric_t, int_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("VectorNLogSpace"),
	DESCRIPTION("Creates a logarithmically spaced vector with defined length.\n"
		    "The length must be larger than 1."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(VECTOR_),
	GINPUT(),
	KEYWORDS("start", "stop", "n"),
	TYPES(Numeric_t, Numeric_t, int_t)));



//
//=== Array of Matrix and Array of Vector Write Methods
//
  md_data.push_back
    ( MdRecord
      ( NAME("ArrayOfMatrixWriteToFile"),
	DESCRIPTION("Writes a variable of this type to a file.\n"
		    "The filename is <basename>.<variable_name>.a.\n"
		    "The format is as follows:\n\n"
		    "# <comments>\n"
		    "<n_array_elements>\n"
		    "<n_rows> <n_columns>\n"
		    "<elements>\n"
		    "<n_rows> <n_columns>\n"
		    "<elements>\n"
		    "...\n\n"
		    "Example:\n"
		    "# Generated by arts-0.0.16, Apr 29 2000, 17:38:44\n"
		    "2\n"
		    "3 4\n"
		    "xx xx xx xx\n"
		    "xx xx xx xx\n"
		    "xx xx xx xx\n"
		    "2 2\n"
		    "yy yy\n"
		    "yy yy"),
	OUTPUT(),
	INPUT(),
	GOUTPUT(),
	GINPUT(ARRAYofMATRIX_),
	KEYWORDS(),
	TYPES()));

  md_data.push_back
    ( MdRecord
      ( NAME("ArrayOfMatrixWriteToNamedFile"),
	DESCRIPTION("Writes a variable of this type to a named file.\n"
		    "The filename has to be specified.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(),
	GINPUT(ARRAYofMATRIX_),
	KEYWORDS("filename"),
	TYPES(string_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("ArrayOfVectorWriteToFile"),
	DESCRIPTION("Writes a variable of this type to a file.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(), 
	INPUT(),
	GOUTPUT(),
	GINPUT(ARRAYofVECTOR_),
	KEYWORDS(),
	TYPES()));
  
  md_data.push_back
    ( MdRecord
      ( NAME("ArrayOfVectorWriteToNamedFile"),
	DESCRIPTION("Writes a variable of this type to a named file.\n"
		    "The filename has to be specified.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(),
	GINPUT(ARRAYofVECTOR_),
	KEYWORDS("filename"),
	TYPES(string_t)));



//
//=== Matrix and Vector Write Methods
//
  md_data.push_back
    ( MdRecord
      ( NAME("MatrixWriteToFile"),
	DESCRIPTION("Writes a variable of this type to a file.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(),
	GINPUT(MATRIX_),
	KEYWORDS(),
	TYPES()));

  md_data.push_back
    ( MdRecord
      ( NAME("MatrixWriteToNamedFile"),
	DESCRIPTION("Writes a variable of this type to a named file.\n"
		    "The filename has to be specified.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(),
	GINPUT(MATRIX_),
	KEYWORDS("filename"),
	TYPES(string_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("VectorWriteToFile"),
	DESCRIPTION("Writes a variable of this type to a file.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(), 
	INPUT(),
	GOUTPUT(),
	GINPUT(VECTOR_),
	KEYWORDS(),
	TYPES()));
  
  md_data.push_back
    ( MdRecord
      ( NAME("VectorWriteToNamedFile"),
	DESCRIPTION("Writes a variable of this type to a named file.\n"
		    "The filename has to be specified.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(),
	GINPUT(VECTOR_),
	KEYWORDS("filename"),
	TYPES(string_t)));



//
//=== Array of Matrix and Vector Read Methods
//
  md_data.push_back
    ( MdRecord
      ( NAME("ArrayOfMatrixReadFromFile"),
	DESCRIPTION("Reads a variable of this type from a file.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(ARRAYofMATRIX_),
	GINPUT(),
	KEYWORDS(),
	TYPES()));

  md_data.push_back
    ( MdRecord
      ( NAME("ArrayOfVectorReadFromFile"),
	DESCRIPTION("Reads a variable of this type from a named file.\n"
		    "The filename has to be specified.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(ARRAYofVECTOR_),
	GINPUT(),
	KEYWORDS(),
	TYPES()));
  

//
//=== Matrix and Vector Read Methods
//
  md_data.push_back
    ( MdRecord
      ( NAME("MatrixReadFromFile"),
	DESCRIPTION("Reads a variable of this type from a file.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(MATRIX_),
	GINPUT(),
	KEYWORDS(),
	TYPES()));

  md_data.push_back
    ( MdRecord
      ( NAME("VectorReadFromFile"),
	DESCRIPTION("Reads a variable of this type from a file.\n"
		    "See `ArrayOfMatrixWriteToFile' for a description of the format."),
	OUTPUT(),
	INPUT(),
	GOUTPUT(VECTOR_),
	GINPUT(),
	KEYWORDS(),
	TYPES()));
  


//======================================================================
//=== Absorption methods
//======================================================================
  
  md_data.push_back
    ( MdRecord
      ( NAME("AllAbsExample"),
	DESCRIPTION("Reads all important absorption related variables from the\n"
		    "given files."),
	OUTPUT(f_abs_, p_abs_, t_abs_, abs_),
	INPUT(),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS(),
	TYPES()));
  


//======================================================================
//=== LOS/RTE methods
//======================================================================

  md_data.push_back
    ( MdRecord
      ( NAME("losBasic"),
  	DESCRIPTION(
          "A general function to determine LOS for a 1D atmosphere.\n"
          "Refraction variables and ground altitude and emission\n"
          "must be set when using this function."),
	OUTPUT(los_),
	INPUT( z_plat_ ,view1_, l_step_, p_abs_, z_abs_, 
               refr_, l_step_refr_, refr_index_,
               z_ground_, e_ground_ ),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS(),
	TYPES()));

  md_data.push_back
    ( MdRecord
      ( NAME("sourceBasic"),
  	DESCRIPTION(
          "Determines the mean source function between the points of a 1D LOS.\n" 
          "This assumes no scattering and local thermodynamic equilibrium, i.e.\n"
          "the source function equals the Planck function."),
	OUTPUT( source_ ),
	INPUT( los_, p_abs_, t_abs_, f_abs_ ),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS(),
	TYPES()));

  md_data.push_back
    ( MdRecord
      ( NAME("transBasic"),
  	DESCRIPTION(
          "Calculates the transmission between the points of a 1D LOS."),
	OUTPUT( trans_ ),
	INPUT( los_, p_abs_, abs_ ),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS(),
	TYPES()));

  md_data.push_back
    ( MdRecord
      ( NAME("y_spaceStd"),
  	DESCRIPTION(
          "Standard selection for the radiation entering the atmosphere at the\n"
          "start of the LOS. The selections are:\n"
          "  0 no radiation\n"
          "  1 cosmic background radiation (planck for 2.7 K)\n"
          "  2 solar radiation (planck for 6000 K)"),
	OUTPUT( y_space_ ),
	INPUT( f_abs_ ),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS("nr"),
	TYPES(int_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("y_spacePlanck"),
  	DESCRIPTION(
          "Sets the radiation entering the atmosphere at the start of the LOS\n"
          "to the Planck function for the given temperature."),
	OUTPUT( y_space_ ),
	INPUT( f_abs_ ),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS("temp"),
	TYPES(Numeric_t)));

  md_data.push_back
    ( MdRecord
      ( NAME("yGeneral"),
  	DESCRIPTION(
          "Solves the radiative transfer equation (RTE) along the LOS."),
	OUTPUT( y_ ),
	INPUT( los_, f_abs_, y_space_, source_, trans_, e_ground_, t_ground_ ),
	GOUTPUT(),
	GINPUT(),
	KEYWORDS(),
	TYPES()));

}

void define_md_map()
{
  extern const ARRAY<MdRecord> md_data;
  extern std::map<string, size_t> MdMap;

  for ( size_t i=0 ; i<md_data.size() ; ++i)
    {
      MdMap[md_data[i].Name()] = i;
    }
}


ostream& MdRecord::PrintTemplate(ostream& os, bool show_description=true) const
{
  extern const  ARRAY<string> wsv_group_names;

  if (show_description)
    {
      // FIXME: Print description string!
    }
  
  os << Name();

  // Is this a generic method? -- Then we need round braces.
  if ( 0 != GOutput().size()+GInput().size() )
    {
      // First entry needs to comma before:
      bool first=true;

      os << '(';

      for (size_t i=0; i<GOutput().size(); ++i)
	{
	  if (first) first=false;
	  else os << ",\n";

	  os << wsv_group_names[GOutput()[i]];
	}

      for (size_t i=0; i<GInput().size(); ++i)
	{
	  if (first) first=false;
	  else os << ",\n";

	  os << wsv_group_names[GInput()[i]];
	}

      os << ')';
    }

  // Now the keywords:

  os << '{';

  // Determine the length of the longest keyword:
  size_t maxsize = 0;
  for (size_t i=0; i<Keywords().size(); ++i)
    if ( Keywords()[i].size() > maxsize )
      maxsize = Keywords()[i].size();


  for (size_t i=0; i<Keywords().size(); ++i)
    {
      os << "\t" << setw(maxsize)
	 << Keywords()[i] << " = \n";
    }

  os << '}';

  return os;
}

ostream& operator<<(ostream& os, const MdRecord& mdr)
{
  extern const ARRAY<WsvRecord> wsv_data;
  extern const ARRAY<string> wsv_group_names;
  extern const string TokValTypeName[];
  bool first;

  os << "Name = " << mdr.Name() << '\n'
     << "Description =\n" << mdr.Description() << "\n\n";

  // Output:
  first = true;
  os << "Output = ";
  for ( size_t i=0; i<mdr.Output().size(); ++i )
    {
      if (first) first=false;
      else os << ", ";

      os << wsv_data[mdr.Output()[i]].Name();
    }
  os << '\n';

  // Input:
  first = true;
  os << "Input = ";
  for ( size_t i=0; i<mdr.Input().size(); ++i )
    {
      if (first) first=false;
      else os << ", ";

      os << wsv_data[mdr.Input()[i]].Name();
    }
  os << '\n';
      
  // GOutput:
  first = true;
  os << "GOutput = ";
  for ( size_t i=0; i<mdr.GOutput().size(); ++i )
    {
      if (first) first=false;
      else os << ", ";

      os << wsv_group_names[mdr.GOutput()[i]];
    }
  os << '\n';

  // GInput:
  first = true;
  os << "GInput = ";
  for ( size_t i=0; i<mdr.GInput().size(); ++i )
    {
      if (first) first=false;
      else os << ", ";

      os << wsv_group_names[mdr.GInput()[i]];
    }
  os << '\n';

  // Keywords:
  first = true;
  os << "Keywords = ";
  for ( size_t i=0; i<mdr.Keywords().size(); ++i )
    {
      if (first) first=false;
      else os << ", ";

      os << mdr.Keywords()[i];
    }
  os << '\n';

  // Types:
  first = true;
  os << "Types = ";
  for ( size_t i=0; i<mdr.Types().size(); ++i )
    {
      if (first) first=false;
      else os << ", ";

      os << TokValTypeName[mdr.Types()[i]];
    }
  os << '\n';

  return os;
}
