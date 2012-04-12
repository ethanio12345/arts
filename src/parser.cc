/* Copyright (C) 2008 Oliver Lemke <olemke@core-dump.info>

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

#include <iostream>
#include "parser.h"
#include "arts.h"
#include "exceptions.h"
#include "file.h"
#include "methods.h"
#include "parameters.h"
#include "wsv_aux.h"
#include "workspace_ng.h"


/** Constructs a new parser.

    \param[out] tasklist    Method list read from the controlfile.
    \param[in]  controlfile Path to the controlfile.

    \author Oliver Lemke
*/
ArtsParser::ArtsParser(Agenda& tasklist, String controlfile, const Verbosity& rverbosity)
  : mtasklist (tasklist), mcfile (controlfile), mcfile_version (1), verbosity (rverbosity)
{
  msource.AppendFile (mcfile);
}


/** Public interface to the main function of the parser.

    \author Oliver Lemke
*/
void ArtsParser::parse_tasklist ()
{
  parse_main ();
}


/** The main function of the parser. This will parse the entire
    text.

    \author Stefan Buehler, Oliver Lemke
*/
void ArtsParser::parse_main()
{
  CREATE_OUT0
  CREATE_OUT3
  
  try 
    {
      extern const Array<MdRecord> md_data;

      // For method ids:
      Index id;         
      // Output workspace variables (for generic methods):
      ArrayOfIndex output;              
      // Input workspace variables (for generic methods):
      ArrayOfIndex input;
      // For include statements, holding the include file's name
      String include_file;

      ArrayOfIndex  auto_vars;
      Array<TokVal> auto_vars_values;

      out3 << "\nParsing control text:\n";

      msource.Init();
      eat_whitespace();

      parse_method(id,output,input,mtasklist,auto_vars,auto_vars_values,
                   include_file,true);
          
      if ( "Arts" != md_data[id].Name() &&  "Arts2" != md_data[id].Name() )
        {
          ostringstream os;
          os << "The outermost agenda must be Arts2!\n"
             << "(But it seems to be " << md_data[id].Name() << ".)\n";
          throw runtime_error(os.str());
        }

      try {
        if (!msource.reachedEot()) eat_whitespace();
        if (!msource.reachedEot()) throw UnexpectedChar("", 
                                                        msource.File(),
                                                        msource.Line(),
                                                        msource.Column() ); 
      }
      catch (const Eot&)
      {
        // It's ok to reach the end of the file here,
        // that's actually what we want.
      }
      catch (const UnexpectedChar& x)
      {
        ostringstream os;
        os << "Unexpected character(s) at the end of the control file\n";
        os << "after the main agenda was already closed.\n";
        os << "File:   " << x.file() << '\n';
        os << "Line:   " << x.line() << '\n';
        os << "Column: " << x.column() << '\n';
        throw runtime_error(os.str());
      }
    }
  catch (const Eot& x)
    {
      // Unexpected end of the source text:
      ostringstream os;
      os << "Unexpected end of control script.\n";
      os << "File: " << x.file() << '\n';
      os << "Line: " << x.line() << '\n';
      throw runtime_error(os.str());
    }
  catch (const UnexpectedChar& x)
    {
      // Unexpected Character:
      ostringstream os;
      os << "Unexpected character:\n";
      os << x.what()   << '\n';
      os << "File:   "   << x.file() << '\n';
      os << "Line:   "   << x.line() << '\n';
      os << "Column: " << x.column() << '\n';
      throw runtime_error(os.str());
    }
  catch (const IllegalLinebreak& x)
    {
      // A line break in an illegal position:
      ostringstream os;
      os << "Illegal Line break:\n";
      os << x.what()   << '\n';
      os << "File: "   << x.file() << '\n';
      os << "Line: "   << x.line() << '\n';
      throw runtime_error(os.str());
    }
  catch (const UnknownMethod& x)
    {
      // Method unknown:
      // [**This should give a hint on how to obtain a list of allowed 
      // methods.]
      ostringstream os;
      os << "Unknown Method:\n";
      os << x.what()   << '\n';
      os << "File:   "   << x.file() << '\n';
      os << "Line:   "   << x.line() << '\n';
      os << "Column: " << x.column() << '\n';
      throw runtime_error(os.str());
    }
  catch (const UnknownWsv& x)
    {
      // Workspace variable unknown:
      // [**This should give a hint on how to obtain a list of allowed 
      // Wsvs.]
      ostringstream os;
      os << "Unknown workspace variable:\n";
      os << x.what()   << '\n';
      os << "File:   "   << x.file() << '\n';
      os << "Line:   "   << x.line() << '\n';
      os << "Column: " << x.column() << '\n';
      throw runtime_error(os.str());
    }
  catch (const WsvAlreadyExists& x)
    {
      // Trying to create the same variable twice:
      ostringstream os;
      os << "Attempt to create a workspace variable that already exists:\n";
      os << x.what()   << '\n';
      os << "File:   "   << x.file() << '\n';
      os << "Line:   "   << x.line() << '\n';
      os << "Column: " << x.column() << '\n';
      throw runtime_error(os.str());
    }
  catch (const WrongWsvGroup& x)
    {
      // Workspace variable unknown:
      // [**This should give a hint on how to obtain a list of Wsvs in 
      // this group.
      ostringstream os;
      os << "Workspace variable belongs to the wrong group:\n";
      os << x.what()   << '\n';
      os << "File:   "   << x.file() << '\n';
      os << "Line:   "   << x.line() << '\n';
      os << "Column: " << x.column() << '\n';
      throw runtime_error(os.str());
    }
  catch (const ParseError& x)
    {
      // General Parse Error (parent of all the above):
      ostringstream os;
      os << "Parse error:\n";
      os << x.what()   << '\n';
      os << "File:   "   << x.file() << '\n';
      os << "Line:   "   << x.line() << '\n';
      os << "Column: " << x.column() << '\n';
      throw runtime_error(os.str());
    }
}


/** Parse the Contents of text as ARTS control input. 
  
    This method is used to parse the list of methods given in the
    curly braces of an agenda method. So the end is marked by a
    closing curly brace ahead.

    \param[out] tasklist The method ids.
  
    \see eat_whitespace
    \see parse_method
          
    \author Stefan Buehler
*/
void ArtsParser::parse_agenda( Agenda& tasklist )
{
  CREATE_OUT2
  CREATE_OUT3
  
  extern const Array<MdRecord> md_data;

  // For method ids:
  Index id;             
  // Output workspace variables:
  ArrayOfIndex output;          
  // Input workspace variables:
  ArrayOfIndex input;
  // For Agenda, if there is any:
  Agenda tasks;
  // For include statements, holding the include file's name
  String include_file;

  ArrayOfIndex  auto_vars;
  Array<TokVal> auto_vars_values;

  eat_whitespace();

  while ( '}' != msource.Current() )
    {
      parse_method(id,output,input,tasks,
                   auto_vars,auto_vars_values,include_file);

      // If parse_method found an include statement it returnes -1 for the
      // method id
      if (id == -1)
        {
          // Command line parameters which give us the include search path.
          extern const Parameters parameters;

          if (!find_file (include_file, ".arts", parameters.includepath))
            {
              ostringstream os;
              os << "Cannot find include file " << include_file
                << ".\n";
              os << "File: "   << msource.File() << '\n';
              os << "Search path was: . " << parameters.includepath
                << "\n";
              throw runtime_error (os.str());
            }

          out2 << "- Including control file " << include_file << "\n";

          ArtsParser include_parser (tasks, include_file, verbosity);
          include_parser.parse_tasklist();

          for (Index i = 0; i < tasks.nelem(); i++)
            tasklist.push_back (tasks.Methods()[i]);
        }
      else
        {
          if (md_data[id].SetMethod())
            {
              // Append task to task list:      
              tasklist.push_back(MRecord(id,output,input,
                                         auto_vars_values[0],tasks));
            }
          else
            {
              tasklist_insert_set_delete(auto_vars, auto_vars_values,
                                         0, tasklist);

              // Append task to task list:      
              tasklist.push_back(MRecord(id,output,input,TokVal(),tasks));

              tasklist_insert_set_delete(auto_vars, auto_vars_values,
                                         1, tasklist);
            }

          {
            // Everything in this block is just to generate some
            // informative output.  

            out3 << "- " << md_data[id].Name() << "\n";

            // Output workspace variables for generic methods:
            if ( 0 < md_data[id].GOutType().nelem()
                 + md_data[id].GInType().nelem() )
              {
                out3 << "   Output: ";
                for ( Index j=0 ; j<output.nelem() ; ++j )
                  {
                    out3 << Workspace::wsv_data[output[j]].Name() << " ";
                  }
                out3 << "\n";

                out3 << "   Input: ";
                for ( Index j=0 ; j<input.nelem() ; ++j )
                  {
                    out3 << Workspace::wsv_data[input[j]].Name() << " ";
                  }
                out3 << "\n";
              }
          }
        }

      eat_whitespace();
    } 
}


/** Parse the Contents of text as ARTS control input. 

  Either values or tasks will be empty.

  \param[out]    id           Method id.
  \param[out]    output       Output workspace variables (for generic methods).
  \param[out]    input        Input workspace variables (for generic methods).
  \param[out]    tasks        A list of other methods.
  \param[out]    auto_vars    Indexes of automatically created variables.
  \param[out]    auto_vars_values    Values of automatically created variables.
  \param[out]    include_file The input to parse.
  \param[in]     no_eot       Suppress throwing an error on EOT after the
                              closing curly brace.
   
  \see read_name
  \see eat_whitespace
  \see assertain_character
  \see parse_String
  \see parse_integer
  \see parse_numeric
  \see parse_Stringvector
  \see parse_intvector
  \see parse_numvector
  \see parse_matrix
   
  \exception UnknownMethod
  \exception UnknownWsv
  \exception WrongWsvGroup

  \author Stefan Buehler
*/
void ArtsParser::parse_method(Index&         id, 
                              ArrayOfIndex&  output,
                              ArrayOfIndex&  input,
                              Agenda&        tasks,
                              ArrayOfIndex&  auto_vars,
                              Array<TokVal>& auto_vars_values,
                              String&        include_file,
                              bool           no_eot)
{
  CREATE_OUT3
  
  String methodname;            // We need this out here, since it is
                                // set once and later modified.

  const MdRecord* mdd;          // Handle on the method record. Needed here,
                                // because it is modified.

  bool found_curly_brace = false;

  // Clear all output variables:
  id = 0;
  output.resize( 0 );
  input.resize(  0 );
  tasks.resize(  0 );
  auto_vars.resize(  0 );
  auto_vars_values.resize(  0 );
  include_file = "";
  
  read_name(methodname);

  if (methodname == "INCLUDE")
    {
      eat_whitespace ();
      parse_String (include_file);

      id = -1;

      return;
    }
  else
    {
      if (methodname == "Arts2")
        {
          mcfile_version = 2;
        }
      else if (methodname == "Arts")
        {
          throw runtime_error("Arts version 1 controlfiles are no longer supported.");
        }

      eat_whitespace();

      parse_method_args(mdd, id, methodname, output, input,
                        auto_vars, auto_vars_values);

      eat_whitespace();

      // Now look for the curly braces:
      if (msource.Current() == '{')
        {
          msource.AdvanceChar();
          eat_whitespace();
          found_curly_brace = true;
        }

      // There are two kind of methods, agenda methods, which have other
      // methods in the body, and normal methods, expecting keywords and
      // values. Let's take the agenda case first...
      if ( mdd->AgendaMethod() )
        {
          out3 << "- " << mdd->Name() << "\n";
          out3 << "{\n";
          parse_agenda(tasks);
          out3 << "}\n";
        }

      // Curly braces in non-agenda methods are not valid in v2 controlfiles
      if (mcfile_version == 2 && !mdd->AgendaMethod() && found_curly_brace)
        {
          ostringstream os;
          os << "Expected method name , but got `" << msource.Current() << "'.";
          throw UnexpectedChar( os.str(),
                                msource.File(),
                                msource.Line(),
                                msource.Column() );os << "" << endl;
        }
    }

  // Now look for the closing curly braces.  We have to catch Eot,
  // because after a method description may be a good place to end
  // the control file.
  if (found_curly_brace)
    {
      try
        {
          assertain_character('}');
        }
      catch (const Eot x)
        {
          // Re-throw the error if the no_eot flag is not set:
          if (!no_eot) throw Eot(x);
        }
    }
}


//! Set generic input to default value.
/** Sets the value of the generic input with the given index to its default
    value.
 
  \param[in]     mdd        Method object.
  \param[out]    auto_vars  Indexes of automatically created variables.
  \param[out]    auto_vars_values    Values of automatically created variables.
  \param[in]     gin_index  Index of the generic input which should be set to
                            its default value.

  \returns Workspace variable name containing the default value

  \author Oliver Lemke
  \date   2008-07-16
*/
String ArtsParser::set_gin_to_default(const MdRecord*       mdd,
                                            ArrayOfIndex&   auto_vars,
                                            Array<TokVal>&  auto_vars_values,
                                            Index           gin_index)
{
  String name;

  if (mdd->GInDefault()[gin_index] != NODEF)
    {
      TokVal tv;
      // Now parse the key value. This can be:
      // String, Index, Numeric, ArrayOfString, ArrayOfIndex, Vector
      bool failed = false;
      if (mdd->GInType()[gin_index] == get_wsv_group_id ("String")) 
        {
          tv = mdd->GInDefault()[gin_index];
        }
      else if (mdd->GInType()[gin_index] == get_wsv_group_id ("Index")) 
        {
          Index n;
          istringstream is(mdd->GInDefault()[gin_index]);
          is >> n;
          tv = n;
          if (is.bad () || is.fail ())
            failed = true;
        }
      else if (mdd->GInType()[gin_index] == get_wsv_group_id ("Numeric")) 
        {
          Numeric n;
          istringstream is(mdd->GInDefault()[gin_index]);
          is >> n;
          tv = n;
          if (is.bad () || is.fail ())
            failed = true;
        }
      else if (mdd->GInType()[gin_index] == get_wsv_group_id ("ArrayOfString")) 
        {
          ArrayOfString v;
          String s = mdd->GInDefault()[gin_index];
          if (!parse_stringarray_from_string(v, s))
            {
              failed = true;
            }
          tv = v;
        }
      else if (mdd->GInType()[gin_index] == get_wsv_group_id ("Vector")) 
        {
          Vector v;
          String s = mdd->GInDefault()[gin_index];
          if (!parse_numvector_from_string(v, s))
            {
              failed = true;
            }
          tv = v;
        }
      else
      {
        extern const ArrayOfString wsv_group_names;
        ostringstream os;
        os
        << "Default values for generic inputs with type "
        << wsv_group_names[mdd->GInType()[gin_index]] << " are not supported.\n"
        << "Either remove the default value for generic input '"
        << mdd->GIn()[gin_index] << "' in workspace method\n"
        << "*" << mdd->Name() << "* in methods.cc or discuss this "
        << "issue on the arts-dev mailing list.\n";
        throw ParseError (os.str(),
                          msource.File(),
                          msource.Line(),
                          msource.Column());
      }
      
      Index wsvid;

      {
          ostringstream os;
          os << gin_index;

          name = "auto_" + mdd->Name() + "_" + "gin" + os.str() + "_"
            + mdd->GIn()[gin_index];
      }

      map<String, Index>::const_iterator wsvit =
        Workspace::WsvMap.find(name);
      if (wsvit == Workspace::WsvMap.end())
        {
          wsvid = Workspace::add_wsv(
            WsvRecord(name.c_str(),
                      "Automatically allocated variable.",
                      mdd->GInType()[gin_index],
                      true));
        }
      else
        {
          wsvid = wsvit->second;
        }

      auto_vars.push_back(wsvid);
      auto_vars_values.push_back(tv);

      if (failed)
        {
          ostringstream os;
          os << "Failed to assign default value for generic '"
            << mdd->GIn()[gin_index] << "'.\n"
            << "Check the documentation of workspace method *"
            << mdd->Name() << "*.\n";
          throw ParseError (os.str(),
                            msource.File(),
                            msource.Line(),
                            msource.Column());
        }
    }
  else
    {
      ostringstream os;
      os << "Generic input '" << mdd->GIn()[gin_index]
        << "' omitted but no default value found.\n"
        << "Check the documentation of workspace method *"
        << mdd->Name() << "*.\n";
      throw ParseError (os.str(),
                        msource.File(),
                        msource.Line(),
                        msource.Column());
    }

  return name;
}


//! Parse method's argument list.
/** This function parses a method's argument list.

  \param[in]  mdd        Method
  \param[out] id         Method index in md_data.
  \param[out] methodname Name of the WSM
  \param[out] output     Output WSVs
  \param[out] input      Input WSVs
  \param[out] auto_vars  Indexes of automatically created variables.
  \param[out] auto_vars_values    Values of automatically created variables.

  \author Oliver Lemke
  \date   2008-03-05
*/
void ArtsParser::parse_method_args(const MdRecord*&       mdd,
                                         Index&           id,
                                         String&          methodname,
                                         ArrayOfIndex&    output,
                                         ArrayOfIndex&    input,
                                         ArrayOfIndex&    auto_vars,
                                         Array<TokVal>&   auto_vars_values)
{
  extern const map<String, Index> MdRawMap;
  extern const Array<MdRecord> md_data;
  extern const Array<MdRecord> md_data_raw;
  extern const map<String, Index> MdMap;

  bool still_supergeneric=true; // Flag that our MdRecord still is
                                // from md_data_raw, not from
                                // md_data. 

                                // Find method raw id in raw map:
  const map<String, Index>::const_iterator i = MdRawMap.find(methodname);
  if ( i == MdRawMap.end() )
    throw UnknownMethod(methodname,
                        msource.File(),
                        msource.Line(),
                        msource.Column());

  id = i->second;

  // Get a convenient handle on the data record for this method. We
  // have to use a pointer here, not a reference, because we later
  // want to change where mdd is pointing!
  mdd = &md_data_raw[id];

  // Is this a supergeneric method? If not, take the record in
  // md_data, rather than in md_data_raw:
  if ( !mdd->Supergeneric() )
    {
      // Find explicit method id in MdMap:
      const map<String, Index>::const_iterator i2 = MdMap.find(methodname);
      assert ( i2 != MdMap.end() );
      id = i2->second;            

      mdd = &md_data[id];

      still_supergeneric = false;
    }

  if (msource.Current() == '(')
    {
      bool first = true;        // To skip the first comma.
      String supergeneric_args;
      Index supergeneric_index = -1;

      msource.AdvanceChar();
      eat_whitespace();

      parse_specific_output(mdd, output, first);

      parse_generic_output(mdd, id, methodname, output,
                           first, still_supergeneric, supergeneric_args,
                           supergeneric_index);

      parse_specific_input(mdd, input, auto_vars, auto_vars_values, first);

      parse_generic_input(mdd, id, methodname, input,
                          auto_vars, auto_vars_values,
                          first, still_supergeneric, supergeneric_args,
                          supergeneric_index);

      assert(!still_supergeneric);
      assertain_character(')');
    }
  else
    {
      if (mdd->GOut().nelem())
      {
        ostringstream os;
        os << "This method has generic output. "
           << "You have to pass a variable!";
        throw ParseError( os.str(),
                         msource.File(),
                         msource.Line(),
                         msource.Column() );
      }
      // If the parenthesis were omitted we still have to add the implicit
      // outputs and inputs to the methods input and output variable lists
      ArrayOfIndex vo=mdd->Out();
      for (ArrayOfIndex::const_iterator outs=vo.begin();
           outs<vo.end(); ++outs)
        {
          output.push_back (*outs);
        }

      const ArrayOfIndex &vi = mdd->InOnly();
      for (ArrayOfIndex::const_iterator ins=vi.begin(); ins<vi.end(); ++ins)
        {
          input.push_back (*ins);
        }

        {
          // Make sure all keywords have default values, otherwise the
          // user has to specify them in the controlfile.
          bool all_gin_have_defaults = true;
          for (Index gin = 0;
               all_gin_have_defaults && gin < mdd->GIn().nelem();
               ++gin)
            {
              Index wsvid; // Workspace variable id, is used to
                           // access data in wsv_data.
                            
              if (mdd->GInDefault()[gin] == NODEF)
                all_gin_have_defaults = false;
              else
                {
                  String wsvname;
                  wsvname = set_gin_to_default(mdd, auto_vars,
                                               auto_vars_values, gin);
                    {
                      // Find Wsv id:
                      const map<String, Index>::const_iterator wsvit =
                        Workspace::WsvMap.find(wsvname);
                      if ( wsvit == Workspace::WsvMap.end() )
                        {
                          throw UnknownWsv( wsvname,
                                            msource.File(),
                                            msource.Line(),
                                            msource.Column() );
                        }

                      wsvid = wsvit->second;
                    }
                  input.push_back(wsvid);
                }
            }

          if (!all_gin_have_defaults)
            {
              ostringstream os;
              os << "Not all generic inputs of this method have default "
                << "values, you have to specify them!";
              throw ParseError( os.str(),
                                    msource.File(),
                                    msource.Line(),
                                    msource.Column() );
            }
        }
    }
}


/** Parse the generic input WSVs for current method from the controlfile.

  \param[in]  mdd        Pointer to the current WSM
  \param[out] id         Method index in md_data.
  \param[out] methodname Name of the WSM
  \param[out] input      Input WSVs
  \param[out] auto_vars  Indexes of automatically created variables.
  \param[out] auto_vars_values    Values of automatically created variables.
  \param[in,out]  first  If set to false, there must be a comma before the first
                         WSV in the controlfile
  \param[in,out]  still_supergeneric  True if the supergeneric method has not
                                      been expanded for the different types yet.

  \author Oliver Lemke
  \date   2008-03-05
  */
void ArtsParser::parse_generic_input(const MdRecord*&     mdd,
                                           Index&         id,
                                           String&        methodname,
                                           ArrayOfIndex&  input,
                                           ArrayOfIndex&  auto_vars,
                                           Array<TokVal>& auto_vars_values,
                                           bool&          first,
                                           bool&          still_supergeneric,
                                           String&        supergeneric_args,
                                           Index&         supergeneric_index _U_)
{
  String wsvname;
  Index wsvid;
  extern const ArrayOfString wsv_group_names;
  extern const Array<MdRecord> md_data;
  extern const map<String, Index> MdMap;

  // Then parse all generic input variables
  for ( Index j=0 ; j<mdd->GInType().nelem() ; ++j )
    {
      if (first)
        first = false;
      else
        {
          if (msource.Current() != ')')
            {
              assertain_character(',');
              eat_whitespace();
            }
        }

      // If there is a comma or a closing brace means no value was
      // specified and we use the default value instead (if there is one)
      if (msource.Current() == ',' || msource.Current() == ')')
        {
          wsvname = set_gin_to_default(mdd, auto_vars, auto_vars_values, j);
        }
      else
        {
          ostringstream os;
          os << j;
          if (read_name_or_value(wsvname, auto_vars, auto_vars_values,
                                 "generic" + os.str(),
                                 mdd, mdd->GInType()[j]) == -1
              && mdd->SetMethod())
            {
              throw ParseError("Only constants can be passed to Set methods.\n"
                               "You might want to use the *Copy* here.",
                               msource.File(),
                               msource.Line(),
                               msource.Column() );
            }
        }

        {
          // Find Wsv id:
          const map<String, Index>::const_iterator wsvit =
            Workspace::WsvMap.find(wsvname);
          if ( wsvit == Workspace::WsvMap.end() )
            {
              throw UnknownWsv( wsvname,
                                msource.File(),
                                msource.Line(),
                                msource.Column() );
            }

          wsvid = wsvit->second;
        }

      // Is the method data record still supergeneric? This could
      // be the case if there are no output arguments, only input
      // arguments. In that case, let's find out the actual group!
      if ( still_supergeneric )
        {
          ostringstream os;
          if (wsv_group_names[mdd->GInType()[j]] == "Any")
            supergeneric_args +=
              wsv_group_names[Workspace::wsv_data[wsvid].Group()];
          os << mdd->Name() << "_sg_" << supergeneric_args;
          methodname = os.str();

          // Find explicit method id in MdMap:
          const map<String, Index>::const_iterator mdit =
            MdMap.find(methodname);
          if (mdit != MdMap.end())
            {
              id = mdit->second;         

              mdd = &md_data[id];

              still_supergeneric = false;
            }
        }

      // Now we have explicitly the method record for the right
      // group. From now on no special treatment of supergeneric
      // methods should be necessary.

      // Check that this Wsv belongs to the correct group:
      if (mdd->GInType()[j] == get_wsv_group_id ("Any")
          && mdd->GInSpecType()[j].nelem() )
        {
          if (supergeneric_index == -1)
            {
              bool wrong_group_id = true;
              for (Index i = 0; wrong_group_id && i < mdd->GInSpecType()[j].nelem(); i++)
                {
                  if (Workspace::wsv_data[wsvid].Group()
                      == mdd->GInSpecType()[j][i])
                    {
                      wrong_group_id = false;
                      supergeneric_index = i;
                    }
                }

              if (wrong_group_id)
                {
                  ostringstream os;
                  bool firsttype = true;
                  for (Index i = 0; i < mdd->GInSpecType()[j].nelem(); i++)
                    {
                      if (!firsttype) os << ", "; else firsttype = false;
                      os << wsv_group_names[mdd->GInSpecType()[j][i]];
                    }

                  throw WrongWsvGroup( "*" + mdd->Name()+"* is not defined for "
                                       +wsv_group_names[Workspace::wsv_data[wsvid].Group()]
                                       +" input. Check the online docs.",
                                       msource.File(),
                                       msource.Line(),
                                       msource.Column() );
                }
            }
          else
            {
              if (Workspace::wsv_data[wsvid].Group()
                  != mdd->GInSpecType()[j][supergeneric_index])
                {
                  throw WrongWsvGroup( wsvname+" is not "+
                                       wsv_group_names[mdd->GInSpecType()[j][supergeneric_index]]
                                       +", it is "+ 
                                       wsv_group_names[Workspace::wsv_data[wsvid].Group()],
                                       msource.File(),
                                       msource.Line(),
                                       msource.Column());
                }
            }
        }
      else if (Workspace::wsv_data[wsvid].Group() != mdd->GInType()[j])
        {
          throw WrongWsvGroup( wsvname+" is not "+
                               wsv_group_names[mdd->GInType()[j]]+", it is "+ 
                               wsv_group_names[Workspace::wsv_data[wsvid].Group()],
                               msource.File(),
                               msource.Line(),
                               msource.Column() );
        }

      // Add this one to the list of input variables:
      input.push_back(wsvid);

      eat_whitespace();
    }

}


/** Parse the generic output WSVs for current method from the controlfile.

  \param[in]  mdd        Pointer to the current WSM
  \param[out] id         Method index in md_data.
  \param[out] methodname Name of the WSM
  \param[out] output     Output WSVs
  \param[in,out]  first  If set to false, there must be a comma before the first
                         WSV in the controlfile
  \param[in,out]  still_supergeneric  True if the supergeneric method has not
                                      been expanded for the different types yet.

  \author Oliver Lemke
  \date   2008-03-05
  */
void ArtsParser::parse_generic_output(const MdRecord*&     mdd,
                                            Index&         id,
                                            String&        methodname,
                                            ArrayOfIndex&  output,
                                            bool&          first,
                                            bool&          still_supergeneric,
                                            String&        supergeneric_args,
                                            Index&         supergeneric_index)
{
  String wsvname;
  Index wsvid;
  extern const ArrayOfString wsv_group_names;
  extern const Array<MdRecord> md_data;
  extern const map<String, Index> MdMap;

  // Parse all generic output variables
  for ( Index j=0 ; j<mdd->GOut().nelem() ; ++j )
    {
      if (first)
        first = false;
      else
        {
          assertain_character(',');
          eat_whitespace();
        }

      read_name(wsvname);

        {
          wsvid = -1;
          // Find Wsv id:
          map<String, Index>::const_iterator wsvit =
            Workspace::WsvMap.find(wsvname);
          if ( wsvit == Workspace::WsvMap.end() )
            {
              if (still_supergeneric)
                {
                  ostringstream os;
                  os << "This might be either a typo or you have to create "
                    << "the variable\nby calling TYPECreate(" << wsvname
                    << ") first. Replace TYPE with the\n"
                    << "WSV group your variable should belong to.";

                  throw UnknownWsv( os.str(),
                                    msource.File(),
                                    msource.Line(),
                                    msource.Column() );
                }
              else
                {
                  if (mdd->Name().length() > 6
                      && mdd->Name().substr (mdd->Name().length() - 6)
                      != "Create")
                    {
                      ostringstream os;
                      os << "This might be either a typo or you have to create "
                        << "the variable\nby calling "
                        << wsv_group_names[mdd->GOutType()[j]]
                        << "Create( " << wsvname
                        << " ) first.\n";

                      throw UnknownWsv( os.str(),
                                        msource.File(),
                                        msource.Line(),
                                        msource.Column() );
                    }
                  else
                    {
                      wsvid = Workspace::add_wsv(
                                                 WsvRecord(wsvname.c_str(),
                                                           "Automatically allocated variable.",
                                                           mdd->GOutType()[j],
                                                           true));
                    }
                }
            }

          if (wsvid == -1)
            {
              if (mdd->Name().length() > 6 &&
                  mdd->Name().find ("Create") == mdd->Name().length() - 6)
                {
                  ostringstream os;
                  os << wsvname << " already exists. A variable can only be created once.\n";
                  throw WsvAlreadyExists( os.str(),
                                          msource.File(),
                                          msource.Line(),
                                          msource.Column() );
                }
              wsvid = wsvit->second;
            }
        }

      // If this is a supergeneric method, now is the time to find
      // out the actual group of the argument(s)!
      // If the method also has supergeneric input arguments, we'll
      // look for a match later again.
      if ( still_supergeneric )
        {
          ostringstream os;
          if (wsv_group_names[mdd->GOutType()[j]] == "Any")
            supergeneric_args +=
              wsv_group_names[Workspace::wsv_data[wsvid].Group()];
          os << mdd->Name() << "_sg_" << supergeneric_args;
          methodname = os.str();

          // Find explicit method id in MdMap:
          const map<String, Index>::const_iterator mdit = MdMap.find(methodname);
          if (mdit != MdMap.end() )
            {
              id = mdit->second;         

              mdd = &md_data[id];

              still_supergeneric = false;
            }
        }

      // Now we have explicitly the method record for the right
      // group. From now on no special treatment of supergeneric
      // methods should be necessary.

      // Check that this Wsv belongs to the correct group:
      if (mdd->GOutType()[j] == get_wsv_group_id ("Any")
          && mdd->GOutSpecType()[j].nelem() )
        {
          if (supergeneric_index == -1)
            {
              bool wrong_group_id = true;
              for (Index i = 0; wrong_group_id && i < mdd->GOutSpecType()[j].nelem(); i++)
                {
                  if (Workspace::wsv_data[wsvid].Group()
                      == mdd->GOutSpecType()[j][i])
                    {
                      wrong_group_id = false;
                      supergeneric_index = i;
                    }
                }

              if (wrong_group_id)
                {
                  ostringstream os;
                  bool firsttype = true;
                  for (Index i = 0; i < mdd->GOutSpecType()[j].nelem(); i++)
                    {
                      if (!firsttype) os << ", "; else firsttype = false;
                      os << wsv_group_names[mdd->GOutSpecType()[j][i]];
                    }

                  throw WrongWsvGroup( "*" + mdd->Name() + "* is not defined for "
                                       +wsv_group_names[Workspace::wsv_data[wsvid].Group()]
                                       +" output. Check the online docs.",
                                       msource.File(),
                                       msource.Line(),
                                       msource.Column() );
                }
            }
          else
            {
              if (Workspace::wsv_data[wsvid].Group()
                  != mdd->GOutSpecType()[j][supergeneric_index])
                {
                  throw WrongWsvGroup( wsvname+" is not "+
                                       wsv_group_names[mdd->GOutSpecType()[j][supergeneric_index]]
                                       +", it is "+ 
                                       wsv_group_names[Workspace::wsv_data[wsvid].Group()],
                                       msource.File(),
                                       msource.Line(),
                                       msource.Column());
                }
            }
        }
      else if ( Workspace::wsv_data[wsvid].Group() != mdd->GOutType()[j] )
        {
          throw WrongWsvGroup( wsvname+" is not "+
                               wsv_group_names[mdd->GOutType()[j]]+", it is "+ 
                               wsv_group_names[Workspace::wsv_data[wsvid].Group()],
                               msource.File(),
                               msource.Line(),
                               msource.Column() );
        }

      // Add this one to the list of output workspace variables:
      output.push_back(wsvid);

      eat_whitespace();
    }
}


/** Parse the specific input WSVs for current method from the controlfile.

  \param[in]  mdd        Pointer to the current WSM
  \param[out] input      Indexes of input variables for the WSM
  \param[out] auto_vars  Indexes of automatically created variables.
  \param[out] auto_vars_values    Values of automatically created variables.
  \param[in]  first  If set to false, there must be a comma before the first WSV
                     in the controlfile

  \author Oliver Lemke
  \date   2008-03-05
  */
void ArtsParser::parse_specific_input(const MdRecord*      mdd,
                                            ArrayOfIndex&  input,
                                            ArrayOfIndex&  auto_vars,
                                            Array<TokVal>& auto_vars_values,
                                            bool&          first)
{
  extern const ArrayOfString wsv_group_names;

  // There are two lists of parameters that we have to read.
  ArrayOfIndex  vo=mdd->Out();  // Output 
  const ArrayOfIndex &vi = mdd->InOnly(); // Input

  Index wsvid;  // Workspace variable id, is used to
                // access data in wsv_data.

  for (ArrayOfIndex::const_iterator ins=vi.begin(); ins<vi.end(); ++ins)
    {
      String wsvname;

      if (first)
        first = false;
      else
        {
          try
            {
              assertain_character(',');
            }
          catch (UnexpectedChar)
            {
              ostringstream os;
              os << "Expected input WSV *" << Workspace::wsv_data[*ins].Name() << "*";
            }
          eat_whitespace();
        }

      read_name_or_value(wsvname, auto_vars, auto_vars_values,
                         Workspace::wsv_data[*ins].Name(),
                         mdd, Workspace::wsv_data[*ins].Group());

      {
        // Find Wsv id:
        const map<String, Index>::const_iterator wsvit =
          Workspace::WsvMap.find(wsvname);
        if ( wsvit == Workspace::WsvMap.end() )
          throw UnknownWsv( wsvname,
                            msource.File(),
                            msource.Line(),
                            msource.Column() );

        wsvid = wsvit->second;
      }

      // Check that this Wsv belongs to the correct group:
      if ( Workspace::wsv_data[wsvid].Group() != Workspace::wsv_data[*ins].Group() )
        {
          throw WrongWsvGroup( wsvname+" is not "+
                               wsv_group_names[Workspace::wsv_data[*ins].Group()]+", it is "+ 
                               wsv_group_names[Workspace::wsv_data[wsvid].Group()],
                               msource.File(),
                               msource.Line(),
                               msource.Column() );
        }

      input.push_back(wsvid);
    }

  eat_whitespace();
}


/** Parse the output WSVs for current method from the controlfile.

  \param[in]  mdd     Pointer to the current WSM
  \param[out] output  Indexes of output variables for the WSM
  \param[in]  first   If set to false, there must be a comma before the first WSV
                      in the controlfile

  \author Oliver Lemke
  \date   2008-03-05
  */
void ArtsParser::parse_specific_output(const MdRecord*     mdd,
                                             ArrayOfIndex& output,
                                             bool&         first)
{
  extern const ArrayOfString wsv_group_names;

  ArrayOfIndex  vo=mdd->Out();

  Index wsvid;                  // Workspace variable id, is used to
                                // access data in wsv_data.

  for (ArrayOfIndex::const_iterator outs=vo.begin(); outs<vo.end(); ++outs)
    {
      String wsvname;

      if (first)
        first = false;
      else
        {
          try
            {
              assertain_character(',');
            }
          catch (UnexpectedChar)
            {
              ostringstream os;
              os << "Expected output WSV *" << Workspace::wsv_data[*outs].Name() << "*";
              throw ParseError( os.str(),
                                msource.File(),
                                msource.Line(),
                                msource.Column() );
            }
          eat_whitespace();
        }

      read_name(wsvname);

      {
        wsvid = -1;
        // Find Wsv id:
        map<String, Index>::const_iterator wsvit =
          Workspace::WsvMap.find(wsvname);
        if ( wsvit == Workspace::WsvMap.end() )
          {
            if (mdd->Name().length() > 6
                && mdd->Name().substr (mdd->Name().length() - 6)
                != "Create")
              {
                ostringstream os;
                os << "This might be either a typo or you have to create "
                  << "the variable\nby calling "
                  << wsv_group_names[Workspace::wsv_data[*outs].Group()]
                  << "Create( " << wsvname
                  << " ) first.\n";

                throw UnknownWsv( os.str(),
                                  msource.File(),
                                  msource.Line(),
                                  msource.Column() );
              }
            else
              {
                wsvid = Workspace::add_wsv(
                                           WsvRecord(wsvname.c_str(),
                                             "Automatically allocated variable.",
                                             Workspace::wsv_data[*outs].Group(),
                                             true));
              }
          }

        if (wsvid == -1)
          wsvid = wsvit->second;
      }

      // Check that this Wsv belongs to the correct group:
      if ( Workspace::wsv_data[wsvid].Group() != Workspace::wsv_data[*outs].Group() )
        {
          throw WrongWsvGroup( wsvname+" is not "+
                               wsv_group_names[Workspace::wsv_data[*outs].Group()]+", it is "+ 
                               wsv_group_names[Workspace::wsv_data[wsvid].Group()],
                               msource.File(),
                               msource.Line(),
                               msource.Column() );
        }

      output.push_back(wsvid);
    }

  eat_whitespace();
}


/** Insert Set and Delete methods for automatically allocated output WSVs.

    This function inserts either a bunch of Set or Delete methods for
    implicitly allocated output WSVs. This needs to be done if the controlfile contains
    a value instead of an output variable name.

    \see ArtsParser::parse_agenda

    \param[in]  auto_vars        Indexes of automatically created variables.
    \param[in]  auto_vars_values Values of automatically created variables.
    \param[in]  method_type      0 = insert Set method, 1 = insert Delete method.
    \param[out] tasklist         Agenda to which the methods should be appended.
*/
void ArtsParser::tasklist_insert_set_delete(const ArrayOfIndex&  auto_vars,
                                            const Array<TokVal>& auto_vars_values,
                                            const Index          method_type,
                                                  Agenda&        tasklist)
{
  extern const map<String, Index> MdMap;
  extern const ArrayOfString wsv_group_names;

  for (Index i=0; i<auto_vars.nelem(); i++)
    {
      map<String, Index>::const_iterator mdit;
      Index         init_mdid;
      TokVal        auto_keyword_value;
      ArrayOfIndex  auto_output_var;
      ArrayOfIndex  auto_input_var;
      Agenda        auto_tasks;

      const Index auto_group = Workspace::wsv_data[auto_vars[i]].Group();
      if (auto_group != get_wsv_group_id("Index")
          && auto_group != get_wsv_group_id("Numeric")
          && auto_group != get_wsv_group_id("ArrayOfIndex")
          && auto_group != get_wsv_group_id("ArrayOfString")
          && auto_group != get_wsv_group_id("String")
          && auto_group != get_wsv_group_id("Vector")
          && auto_group != get_wsv_group_id("Matrix"))
        {
          ostringstream os;
          os << "Passing a "
            << wsv_group_names[Workspace::wsv_data[auto_vars[i]].Group()]
            << " constant to a WSM is not supported!";
          throw ParseError (os.str(),
                            msource.File(),
                            msource.Line(),
                            msource.Column());
        }

      String method_name;
      switch (method_type)
        {
        case 0:
          auto_keyword_value = auto_vars_values[i];
          auto_output_var.push_back(auto_vars[i]);
          method_name = wsv_group_names[
            Workspace::wsv_data[auto_vars[i]].Group()] + "Set";
          break;
        case 1:
          auto_input_var.push_back(auto_vars[i]);
          method_name = "Delete_sg_" + wsv_group_names[Workspace::wsv_data[auto_vars[i]].Group()];
          break;
        default:
          throw runtime_error("Invalid method_type");
        }

      mdit = MdMap.find(method_name);
      assert ( mdit != MdMap.end() );
      init_mdid = mdit->second;         

      tasklist.push_back(MRecord(init_mdid,
                                 auto_output_var, auto_input_var,
                                 auto_keyword_value,
                                 auto_tasks));
    }
}


/** Returns true if this character is considered whitespace. This
    includes the comment sign `\#'. This function is used by other
    functions to test for delimiting whitespace. 

    The whitespace cases implemented here must be consistent with
    eat_whitespace! 

    \param[in] c Character to test.

    \see eat_whitespace  */
bool ArtsParser::is_whitespace(const char c)
{
  switch (c) 
    {
    case ' ':
    case '\r':
    case '\t':
    case '#':
      return true;
      break;
    }

  return false;
}


/** Eats whitespace. Comments are a special case of
    whitespace. Everything from the `\#' to the end of the line is
    eaten. 
  
    The whitespace cases implemented here must be consistent with
    is_whitespace! 
    \see is_whitespace    

    \exception UnexpectedChar Non-whitespace character encountered. */
void ArtsParser::eat_whitespace()
{
  char dummy;

  while (is_whitespace(dummy=msource.Current())) 
    {
      switch (dummy)
        {
        case ' ':
        case '\r':
        case '\t':
          msource.AdvanceChar();
          break;
        case '#':
          msource.AdvanceLine();
          break;
        default:
          {
            ostringstream os;
            os << "Expected whitespace, but got `" << dummy << "'.";
            throw UnexpectedChar( os.str(),
                                  msource.File(),
                                  msource.Line(),
                                  msource.Column() ); 
            break;
          }
        }
    }
}


/** Eats whitespace from a String.

  \param[in]     str String.
  \param[in,out] pos Current position in the String.
 */
void ArtsParser::eat_whitespace_from_string(String& str, size_t& pos)
{
  while (pos < str.length() && is_whitespace(str[pos]))
    pos++;
}


/** Reads name of method, keyword, or workspace variable.
  
    These names may consist only of letters (case matters!), numbers, and
    underscores.
    Line break or any other character ends the name.
  
    Whitespace has to have been eaten before. Scanns source for the
    name, starting at position specified by line and column.
    
    \param[out] name Method, keyword or WSV name
*/
void ArtsParser::read_name(String& name)
{
  bool stop = false;
  name = "";

  if (!isalpha(msource.Current()))
    {
      ostringstream os;
      os << "Workspace variable names must start with a letter!";
      throw ParseError( os.str(),
                        msource.File(),
                        msource.Line(),
                        msource.Column() );
    }

  while (!stop) 
    {
      char dummy = msource.Current();

      if ( isalnum(dummy) || '_'==dummy )
        {
          name += dummy;
          // AdvanceChar sets LineBreak if a line break occured.
          msource.LineBreak() = false;
          msource.AdvanceChar();
          if ( msource.LineBreak() ) stop = true;
        }
      else
        {
          stop = true;
        }
    }
}


/** Reads name of a workspace variable or a value.
  
    These names may consist only of letters (case matters!), numbers, and
    underscores.
    Line break or any other character ends the name.
  
    Whitespace has to have been eaten before. Scanns source for the
    name, starting at position specified by line and column.

    \param[out] name       WSV name or value
    \param[out] auto_vars  Indexes of automatically created variables.
    \param[out] auto_vars_values    Values of automatically created variables.
    \param[in]  default_name        Default WSV name.
    \param[in]  mdd        Pointer to the current WSM
    \param[in]  group      Expected WSV group index
    
    \return -1 If a WSV name was found or the index of the newly created
               WSV
*/
Index ArtsParser::read_name_or_value(      String&        name,
                                           ArrayOfIndex&  auto_vars,
                                           Array<TokVal>& auto_vars_values,
                                     const String&        default_name,
                                     const MdRecord*      mdd,
                                     const Index          group)
{
  name = "";

  if (isalpha(msource.Current()))
    {
      read_name(name);
      return -1;
    }

  if (group == get_wsv_group_id ("Any")) 
    {
      ostringstream os;
      os << "Passing constants as supergeneric parameters is not supported.";
      throw ParseError (os.str (),
                        msource.File(),
                        msource.Line(),
                        msource.Column());
    }

  // If a value was given instead of a variable name, we create
  // a new variable in the workspace and fill it with the given
  // value

  Index wsvid;

  name = "auto_" + mdd->Name() + "_" + default_name;
  map<String, Index>::const_iterator wsvit = Workspace::WsvMap.find(name);
  if (wsvit == Workspace::WsvMap.end())
    {
      wsvid = Workspace::add_wsv(WsvRecord(name.c_str(),
                                           "Automatically allocated variable.",
                                           group,
                                           true));
    }
  else
    {
      wsvid = wsvit->second;
    }

  auto_vars.push_back(wsvid);

  // Now parse the value. This can be:
  // String_, Index_, Numeric_, Array_String_, Array_Index_, Vector_, Matrix_
  if (group == get_wsv_group_id("String"))
    {
      String dummy;
      parse_String(dummy);
      auto_vars_values.push_back(dummy);
    }
  else if (group == get_wsv_group_id("Index"))
    {
      Index n;
      parse_integer(n);
      auto_vars_values.push_back(n);
    }
  else if (group == get_wsv_group_id("Numeric"))
    {
      Numeric n;
      parse_numeric(n);
      auto_vars_values.push_back(n);
    }
  else if (group == get_wsv_group_id("ArrayOfString"))
    {
      ArrayOfString dummy;
      parse_Stringvector(dummy);
      auto_vars_values.push_back(dummy);
    }
  else if (group == get_wsv_group_id("ArrayOfIndex"))
    {
      ArrayOfIndex dummy;
      parse_intvector(dummy);
      auto_vars_values.push_back(dummy);
    }
  else if (group == get_wsv_group_id("Vector"))
    {
      Vector dummy;
      parse_numvector(dummy);
      auto_vars_values.push_back(dummy);
    }
  else if (group == get_wsv_group_id("Matrix"))
    {
      Matrix dummy;
      parse_matrix(dummy);
      auto_vars_values.push_back(dummy);
    }
  else
    {
      extern const ArrayOfString wsv_group_names;
      ostringstream os;
      os << "Unsupported parameter type: " << wsv_group_names[group];
      throw ParseError (os.str(),
                        msource.File(),
                        msource.Line(),
                        msource.Column());
    }

  return wsvid;
}


/** Make sure that the current character is equal to c and go to the
    next character.

    \param[in] c Expected character.
  
    \exception UnexpectedChar The character is not right. */
void ArtsParser::assertain_character(char c)
{
  if ( c != msource.Current() )
    {
      ostringstream os;
      os << "Expected '" << c << "', but got '" << msource.Current() << "'.";
      throw UnexpectedChar( os.str(),
                            msource.File(),
                            msource.Line(),
                            msource.Column() ); 
    }
  
  msource.AdvanceChar();
}


/** Reads a String, complete with quotation marks. Whitespace has to
    have been eaten before, that is, the current character must be
    the quotation mark (").
    Quotation marks inside Strings are currently not possible.
  
    Line breaks inside Strings are not allowed. 

    \param[out] res Output string.

    \exception IllegalLinebreak An illegal linebreak has occured. */
void ArtsParser::parse_String(String& res)
{
  bool stop = false;
  res = "";

  msource.LineBreak() = false;
  assertain_character('"');
  if ( msource.LineBreak() )
    throw IllegalLinebreak( "Line break before end of String.",
                            msource.File(),
                            msource.Line(),
                            msource.Column() ); 

  while (!stop) 
    {
      char dummy = msource.Current();
      if ( dummy != '"' )
        {
          res += dummy;
          msource.AdvanceChar();

          if ( msource.LineBreak() )
            throw IllegalLinebreak( "Line break before end of String.",
                                    msource.File(),
                                    msource.Line(),
                                    msource.Column() ); 
        }
      else
        {
          stop = true;
          msource.AdvanceChar();
        }
    }
}


/** Reads an integer. Whitespace has to
    have been eaten before, that is, the current character must be
    a number or `+' or `-'.
  
    Whitespace or line breaks terminate the scanning!
    There are no whitespaces allowed anywhere, consisten with ANSI C
    scanf. 

    \param[out] res Output string containing the integer.

    \exception IllegalLinebreak An illegal linebreak has occured. 
    \exception UnexpectedChar Unexpected character encountered. */
void ArtsParser::read_integer(String& res)
{
  bool stop = false;
  res = "";
  char dummy;
  msource.LineBreak() = false;

  dummy = msource.Current();
  if ( '+' == dummy || '-' == dummy )
    {
      res += dummy;
      msource.AdvanceChar();
      if ( msource.LineBreak() )
        throw IllegalLinebreak( "Line break after sign.",
                                msource.File(),
                                msource.Line(),
                                msource.Column() ); 
    }

  if (!isdigit(msource.Current()))
    {
      ostringstream os;
      os << "Expected digit or variable name, but got `" << msource.Current()
        << "'.";
      throw UnexpectedChar(os.str(),
                           msource.File(),
                           msource.Line(),
                           msource.Column());
    }

  while (!stop) 
    {
      char chtmp = msource.Current();
      if ( isdigit(chtmp) )
        {
          res += chtmp;
          msource.AdvanceChar();
          if ( msource.LineBreak() ) stop = true;
        }
      else
        {
          stop = true;
        }
    }
}


/** Reads a floating point number. Whitespace has to
    have been eaten before, that is, the current character must be
    a number or `+' or `-'.
  
    Example numbers: 23.,  1.0,  -.3,  -3.3e5,  +3e8,  1.0E-9
  
    Illegal numbers: ., 3e, e3, 2e-
  
    Whitespace is not allowed inside the number.
    Line breaks or whitespace terminates the scanning. 

    \param[out] res Output string containing the numeric.

    \exception IllegalLinebreak Illegal line break.
    \exception ParseError Cannot parse this as a number. */
void ArtsParser::read_numeric(String& res)
{
  bool stop;
  res = "";
  char dummy;
  msource.LineBreak() = false;

  // To make sure that there is at least one digit:
  bool found_digit = false;

  // Check if there is a sign:
  dummy = msource.Current();
  if ( '+' == dummy || '-' == dummy )
    {
      res += dummy;
      msource.AdvanceChar();
      if ( msource.LineBreak() )
        throw IllegalLinebreak( "Linebreak after sign.",
                                msource.File(),
                                msource.Line(),
                                msource.Column() ); 
    }

  // There could be some digits here:
  stop = false;
  while (!stop) 
    {
      char chtmp = msource.Current();
      if ( isdigit(chtmp) )
        {
          found_digit = true;
          res += chtmp;
          msource.AdvanceChar();
          if ( msource.LineBreak() ) return; // Line break ends scanning immediately.
        }
      else
        {
          stop = true;
        }
    }

  // Next there can be a decimal point
  if ( '.' == msource.Current() )
    {
      res += ".";
      msource.AdvanceChar();
      if ( msource.LineBreak() )
        {
          if (found_digit)
            {
              // Line break ends scanning immediately, if we have
              // already found at least one digit.
              return;
            }
          else
            {
              throw IllegalLinebreak("Expected at least one digit.",
                                     msource.File(),
                                     msource.Line(),
                                     msource.Column());
            }
        }

      // ... followed by optional more digits
      stop = false;
      while (!stop) 
        {
          char chtmp = msource.Current();
          if ( isdigit(chtmp) )
            {
              found_digit = true;
              res += chtmp;
              msource.AdvanceChar();
              if ( msource.LineBreak() ) return; // Line break ends scanning immediately.
            }
          else
            {
              stop = true;
            }
        }    
    }

  // At this point, we must have found at least one digit.
  if (!found_digit)
    throw ParseError("Expected at least one digit.",
                     msource.File(),
                     msource.Line(),
                     msource.Column());

  // Now there could be a `e' or `E':
  dummy = msource.Current();
  if ( 'e' == dummy || 'E' == dummy )
    {
      res += dummy;
      msource.AdvanceChar();
      if ( msource.LineBreak() )
        throw IllegalLinebreak( "Linebreak after e/E.",
                                msource.File(),
                                msource.Line(),
                                msource.Column() );
      
      // Now there must be an integer (with optional sign)
      {
        String s;
        read_integer(s);
        res += s;
      }
    }
}


/** Use a String stream to parse an integer number.
 
  \param[out] n Parsed integer.
*/
void ArtsParser::parse_integer(Index& n)
{
  String res;
  read_integer(res);
  istringstream is(res);
  is >> n;
}


/** Use a String stream to parse a floating point number.
 
  \param[out] n Parsed numeric.
 */
void ArtsParser::parse_numeric(Numeric& n)
{
  String res;
  read_numeric(res);
  istringstream is(res);
  is >> n;
}


/** Read a vector of Strings. This looks as follows in the control
    file: ["String1","String2"]

    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
  
    Quotation marks inside Strings are currently not possible.
  
    Line breaks are allowed before and after each String. Line breaks
    inside Strings are not allowed. 
   
    \see parse_String */
void ArtsParser::parse_Stringvector(ArrayOfString& res)
{
  bool first = true;            // To skip the first comma.
  res.resize(0);                // Clear the result vector (just in case).

  // Make sure that the current character really is `[' and proceed.
  assertain_character('[');
  // There might have occured a linebreak, which is fine.
  
  eat_whitespace();

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( ']' != msource.Current() )
    {
      String dummy;

      if (first)
        first = false;
      else
        {
          assertain_character(',');
          eat_whitespace();
        }

      parse_String(dummy);
      res.push_back(dummy);
      eat_whitespace();
    }

  msource.AdvanceChar();
}


/** Read a vector of integers. This looks as follows in the control
    file: [123,5,334]
    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
    
    Line breaks are allowed before and after each number. Line breaks
    inside numbers are not allowed. 
   
    \see parse_integer */
void ArtsParser::parse_intvector(ArrayOfIndex& res)
{
  bool first = true;            // To skip the first comma.
  res.resize(0);                        // Clear the result vector (just in case).

  // Make sure that the current character really is `[' and proceed.
  assertain_character('[');
  // There might have occured a linebreak, which is fine.
  
  eat_whitespace();

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( ']' != msource.Current() )
    {
      Index dummy;

      if (first)
        first = false;
      else
        {
          assertain_character(',');
          eat_whitespace();
        }

      parse_integer(dummy);
      res.push_back(dummy);
      eat_whitespace();
    }

  msource.AdvanceChar();
}


/** Read a vector of Numerics. This looks as follows in the control
    file: [1.3, 5, 3.4]
    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
  
    Line breaks are allowed before and after each number. Line breaks
    inside numbers are not allowed. 
   
    \see parse_numeric */
void ArtsParser::parse_numvector(Vector& res)
{
  bool first = true;            // To skip the first comma.

  // We need a temporary Array<Numeric>, so that we can use push_back
  // to store the values. FIXME: Need also constructor for Vector from
  // Array<Numeric>.
  Array<Numeric> tres;

  // Make sure that the current character really is `[' and proceed.
  assertain_character('[');
  // There might have occured a linebreak, which is fine.
  
  eat_whitespace();

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( ']' != msource.Current() )
    {
      Numeric dummy;

      if (first)
        first = false;
      else
        {
          assertain_character(',');
          eat_whitespace();
        }

      parse_numeric(dummy);
      tres.push_back(dummy);
      eat_whitespace();
    }

  // Copy tres to res:
  res.resize(tres.nelem());
  for (int i = 0; i < tres.nelem (); i++)
    {
      res[i] = tres[i];
    }

  msource.AdvanceChar();
}


/** Read a Matrix. This looks as follows in the control
    file: [1, 2, 3; 4, 5, 6]
    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty matrix is allowed.
  
    Line breaks are allowed before and after each number. Line breaks
    inside numbers are not allowed. 
   
    \see parse_numeric */
void ArtsParser::parse_matrix(Matrix& res)
{
  bool first = true;            // To skip the first comma.

  // We need a temporary Array<Numeric>, so that we can use push_back
  // to store the values. FIXME: Need also constructor for Vector from
  // Array<Numeric>.
  Array<Numeric> tres;

  // Make sure that the current character really is `[' and proceed.
  assertain_character('[');
  // There might have occured a linebreak, which is fine.
  
  eat_whitespace();

  Index ncols = -1;
  Index cur_ncols = 1;
  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( ']' != msource.Current() )
    {
      Numeric dummy;

      if (first)
        first = false;
      else
        {
          if (',' == msource.Current())
            {
              cur_ncols++;
              if (ncols != -1 && cur_ncols > ncols)
                {
                  ostringstream os;
                  os << "Expected ';', but got '" << msource.Current() << "'. Check Matrix dimensions.";
                  throw UnexpectedChar( os.str(),
                                        msource.File(),
                                        msource.Line(),
                                        msource.Column() );
                }
              msource.AdvanceChar();
              eat_whitespace();
            }
          else if (';' == msource.Current())
            {
              if (ncols == -1)
                {
                  ncols = cur_ncols;
                }
              else if (ncols != cur_ncols)
                {
                  ostringstream os;
                  os << "Expected ',', but got '" << msource.Current() << "'. Check Matrix dimensions.";
                  throw UnexpectedChar( os.str(),
                                        msource.File(),
                                        msource.Line(),
                                        msource.Column() );
                }
              cur_ncols = 1;
              msource.AdvanceChar();
              eat_whitespace();
            }
          else
            {
              char c = ';';
              if (ncols > cur_ncols)
                c = ',';
              ostringstream os;
              os << "Expected '" << c << "', but got '" << msource.Current() << "'. Check Matrix dimensions.";
              throw UnexpectedChar( os.str(),
                                    msource.File(),
                                    msource.Line(),
                                    msource.Column() );
            }
        }

      parse_numeric(dummy);
      tres.push_back(dummy);
      eat_whitespace();
    }

  if (ncols == -1) ncols = cur_ncols;
  if (ncols != cur_ncols)
    {
      throw ParseError("Missing element(s) in last row of matrix",
                       msource.File(),
                       msource.Line(),
                       msource.Column());
    }


  // Copy tres to res:
  Index nrows = tres.nelem() / ncols;
  res.resize(nrows, ncols);
  for (Index i = 0; i < nrows; i++)
    for (Index j = 0; j < ncols; j++)
      res(i, j) = tres[i*ncols+j];

  msource.AdvanceChar();
}


/** Read a vector of Numerics from a String. This looks as follows: [1.3, 5]
    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
  
    \see parse_numeric */
bool ArtsParser::parse_numvector_from_string (Vector& res, String& str)
{
  bool first = true;            // To skip the first comma.
  size_t pos = 0;

  // We need a temporary Array<Numeric>, so that we can use push_back
  // to store the values.
  Array<Numeric> tres;

  eat_whitespace_from_string (str, pos);

  // Make sure that the current character really is `[' and proceed.
  if (str[pos] != '[')
    {
      throw runtime_error ("No opening bracket\n");
    }

  pos++;

  eat_whitespace_from_string (str, pos);

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( pos < str.length() && str[pos] != ']'  )
    {
      if (first)
        first = false;
      else
        {
          if (str[pos] != ',')
            {
              return false;
            }
          pos++;
          eat_whitespace_from_string (str, pos);
        }

      Numeric dummy;
      istringstream is (str.substr(pos));
      is >> dummy;
      if (is.bad () || is.fail ())
        return false;
      tres.push_back(dummy);
      while (pos < str.length()
             && (isdigit(str[pos]) || str[pos] == '-' || str[pos] == '.'
                 || str[pos] == 'e'))
        pos++;
      eat_whitespace_from_string (str, pos);
    }

  // Copy tres to res:
  res.resize(tres.nelem());
  for (int i = 0; i < tres.nelem (); i++)
    {
      res[i] = tres[i];
    }

  return true;
}


/** Read an Array of Strings from a String. This looks as follows:
    [ "String1", "String2"]
    Whitespace has to have been eaten before, that is, the current
    character must be `['.
  
    The empty vector is allowed.
  
    \see parse_numeric */
bool ArtsParser::parse_stringarray_from_string (ArrayOfString& res, String& str)
{
  bool first = true;            // To skip the first comma.
  size_t pos = 0;

  // We need a temporary Array<Numeric>, so that we can use push_back
  // to store the values.
  ArrayOfString tres;

  eat_whitespace_from_string (str, pos);

  // Make sure that the current character really is `[' and proceed.
  if (str[pos] != '[')
    {
      throw runtime_error ("No opening bracket\n");
    }

  pos++;

  eat_whitespace_from_string (str, pos);

  // Read the elements of the vector (`]' means that we have
  // reached the end):
  while ( pos < str.length() && str[pos] != ']'  )
    {
      if (first)
        first = false;
      else
        {
          if (str[pos] != ',')
            {
              return false;
            }
          pos++;
          eat_whitespace_from_string (str, pos);
        }

      if (str[pos] != '"')
        {
          throw runtime_error ("Expected quotes\n");
        }

      pos++;

      String dummy;
      while ( pos < str.length() && str[pos] != '"'  )
        {
          dummy += str[pos];
          pos++;
        }

      if (pos == str.length() || str[pos] != '"')
        return false;

      tres.push_back(dummy);

      eat_whitespace_from_string (str, pos);
    }

  // Copy tres to res:
  res.resize(tres.nelem());
  for (int i = 0; i < tres.nelem (); i++)
    {
      res[i] = tres[i];
    }

  return true;
}

