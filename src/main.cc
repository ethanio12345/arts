#include "arts.h"
#include "parameters.h"
#include "messages.h"
#include "exceptions.h"
#include "file.h"
#include "wsv.h"		// Automatically generated!
#include "workspace.h"
#include "methods.h"
#include "parser.h"
#include "md.h"


/** Print the error message and exit. */
void give_up(const string& message)
{
  out0 << message << '\n';
  exit(1);
}


/** The arts executor. This executes the methods specified in tasklist
    on the given workspace. It also checks for errors during the
    method execution and stops the program if an error has
    occured. FIXME: Eventually, it should do a good housekeeping of
    which variables are occupied and which are not.

    @param workspace Output. The workspace to act on.
    @param tasklist The list of methods to execute (including keyword data).

    @authore Stefan Buehler  */
void executor(WorkSpace& workspace, const ARRAY<MRecord>& tasklist)
{
  // The method description lookup table:
  const extern ARRAY<MdRecord> md_data;

  // The workspace variable lookup table:
  const extern ARRAY<WsvRecord> wsv_data;
  
  // The array holding the pointers to the getaway functions:
  const extern void (*getaways[])(WorkSpace&, const MRecord&);

  // We need a place to remember which workspace variables are
  // occupied and which aren't.
  // 
  // For some weird reason, ARRAYs of bool do not work, although all
  // other types seem to work fine. So in this single case, I'll use
  // the stl vector directly.
  std::vector<bool> occupied(wsv_data.size(),false);

  out3 << "\nExecuting methods:\n";

  for (size_t i=0; i<tasklist.size(); ++i)
    {
      // Runtime method data for this method:
      const MRecord&  mrr = tasklist[i];
      // Method data for this method:
      const MdRecord& mdd = md_data[mrr.Id()];
      // Needed to store variable lists:
      ARRAY<size_t> v;
      
      try
	{
	  out1 << "- " << mdd.Name() << '\n';

	
	  { // Check if all specific input variables are occupied:
	    v = mdd.Input();
	    for (size_t s=0; s<v.size(); ++s)
	      if (!occupied[v[s]])
		give_up("Method "+mdd.Name()+" needs input variable: "+
			wsv_data[v[s]].Name());
	  }

	  { // Check if all generic input variables are occupied:
	    v = mrr.Input();
	    for (size_t s=0; s<v.size(); ++s)
	      if (!occupied[v[s]])
		give_up("Generic Method "+mdd.Name()+" needs input variable: "+
			wsv_data[v[s]].Name());
	  }

	  // Call the getaway function:
	  getaways[mrr.Id()]
	    ( workspace, mrr );

	  { // Flag the specific output variables as occupied:
	    v = mdd.Output();
	    for (size_t s=0; s<v.size(); ++s) occupied[v[s]] = true;
	  }

	  { // Flag the generic output variables as occupied:
	    v = mrr.Output();
	    for (size_t s=0; s<v.size(); ++s) occupied[v[s]] = true;
	  }

	}
      catch (runtime_error x)
	{
	  out0 << "Error in method: " << mdd.Name() << '\n'
	       << x.what() << '\n';
	  exit(1);
	}
    }
}




/** This is the main function of ARTS. (You never guessed that, did you?)
    The getopt_long function is used to parse the command line parameters.
 
    @return    0=ok, 1=error
    @param     argc Number of command line parameters 
    @param     argv Values of command line parameters
    @author    Stefan Buehler */
int main (int argc, char **argv)
{
  extern const Parameters parameters; // Global variable that holds
                                      // all command line parameters. 

  //--------------------< Get command line parameters >--------------------
  if ( get_parameters(argc, argv) )
    {
      // Print an error message and exit:
      cerr << "Try `arts --help' for help.\n";
      exit(1);
    }

  if (parameters.help)
    {
      // Just print a help message and then exit.
      cerr << '\n' << parameters.usage << "\n\n";
      cerr << parameters.helptext << "\n\n";
      return(0);
    }

  if (parameters.version)
    {
      extern const string full_name;
      // Just print version information and then exit.
      cerr << "This is " << full_name << '\n';
      return(0);
    }

  // There must be at least one controlfile (if not this must be
  // caught by get_parameters).
  assert(0  != parameters.controlfiles.size());

  // Set the basename according to the first control file, if not
  // explicitly specified.
  if ( "" == parameters.basename )
    {
      extern string basename;
      basename = parameters.controlfiles[0];
      // Find the last . in the name
      string::size_type p = basename.rfind('.');
      // Kill everything starting from the `.'
      basename.erase(p);
    }

  //--------------------< Open report file >--------------------
  // This one needs its own little try block, because we have to
  // write error messages to cerr directly since the report file
  // will not exist.
  try
    {
      const extern string basename;     // Basis for file name
      extern ofstream report_file;	// Report file pointer

      //      cout << "rep = " << basename+".rep" << '\n';
      open_output_file(report_file, basename+".rep");
    }
  catch (runtime_error x)
    {
      cerr << x.what() << '\n'
	   << "I have to be able to write to my report file.";
      exit(1);
    }

  // Now comes the global try block. Exceptions caught after this
  // one are general stuff like file opening errors.
  // FIXME: Maybe this is not really necessary, since methods
  // using files could always check for these errors? Think about
  // which way is easier.
  try
    {
      // Some global variables that we need:
      extern WorkSpace workspace;
      //      extern ARRAY<WsvRecord> wsv_data;
      //      extern ARRAY<MdRecord> md_data;

      {
	// Output program name and version number: 
	// The name (PACKAGE) and the major and minor version number
	// (VERSION) are set in configure.in. The configuration tools
	// place them in the file config.h, which is included in arts.h.
	// The subminor number is set in version.cc, which is linked with
	// arts.

	extern const string subversion;
  
	out1 << PACKAGE << " " << VERSION << "." << subversion << '\n';
      }

      // Initialize the wsv data:
      define_wsv_data();

      {
	// Quick test that the pointers stored in wsv_data can be
	// really used to access the workspace variables.
	// YES!! It works.
	// FIXME: Remove all this.
// 	WsvP   *wp = wsv_data[basename_].Pointer(); 
// 	string *s = *wp;
// 	*s = "test";
// 	cout << "workspace.basename = " << workspace.basename << '\n';
      }

      // Initialize the md data:
      define_md_data();

      {
	// Quick test:
// 	for (size_t i=0; i<md_data.size(); ++i)
// 	  {
// 	    cout << md_data[i].Name() << '\n';
// 	  }
      }

      // The list of methods to execute and their keyword data from
      // the control file. 
      ARRAY<MRecord> tasklist;

      // The text of the controlfile.
      SourceText text;
	
      // Read the control text from the control files:
      out3 << "\nReading control files:\n";
      for ( size_t i=0; i<parameters.controlfiles.size(); ++i )
	{
	  out3 << "- " << parameters.controlfiles[i] << '\n';
	  text.AppendFile(parameters.controlfiles[i]);
	}

      // Call the parser to parse the control text:
      parse_main(tasklist, text);

      // Call the executor:
      executor(workspace, tasklist);

    }
  catch (runtime_error x)
    {
      out0 << x.what() << '\n';
      exit(1);
    }

  out1 << "Goodbye.\n";
  return(0);
}
