//==============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 3.0 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//  
//  This software was developed by Applied Research Laboratories at the
//  University of Texas at Austin.
//  Copyright 2004-2020, The Board of Regents of The University of Texas System
//
//==============================================================================

//==============================================================================
//
//  This software was developed by Applied Research Laboratories at the
//  University of Texas at Austin, under contract to an agency or agencies
//  within the U.S. Department of Defense. The U.S. Government retains all
//  rights to use, duplicate, distribute, disclose, or release this software.
//
//  Pursuant to DoD Directive 523024 
//
//  DISTRIBUTION STATEMENT A: This software has been approved for public 
//                            release, distribution is unlimited.
//
//==============================================================================


// CommandLine.cpp  Command line options and argument processing.
//
// NB LOG(DEBUG) (only) appears here, but --debug is not going to turn it on;
// for it to work, LOGlevel = ConfigureLOG:Level("DEBUG"); must be set explicitly
// before calling these routines; then debug will be set here and LOG(DEBUG) will work
// NB default for debug is -1

// TD what if a vector option is given a duplicate value? ignore - let caller handle
// TD does Validate test two targets are equal? do we care?
// TD consider adding '[or -<shortopt>]' to syntax description
// TD is there a bug if --debugN appears twice?
// TD other bool types
// typeBool:       --opt   => target = true;
// typeBoolValue:  --opt T => target = true;
//             and --opt F => target = false;
// typeBoolToggle: --opt   => target = !target;

// system
#include <fstream>
#include <cmath>
// gpstk
#include "Exception.hpp"
#include "StringUtils.hpp"
#include "stl_helpers.hpp"
// geomatics
#include "logstream.hpp"
#include "expandtilde.hpp"

#include "CommandLine.hpp"

using namespace std;
using namespace gpstk;
using namespace StringUtils;

// -----------------------------------------------------------------------------------
// the main entry point
/**
 * @throw Exception
 */
int CommandLine::ProcessCommandLine(int argc, char** argv, string PrgmDesc,
                              string& Usage, string& Errors, vector<string>& Unrecog)
{
try {
   int i,j;
   string option,word;

   // if caller has set LOGlevel to DEBUG already, set debug here
   if(ConfigureLOG::ReportingLevel() >= ConfigureLOG::Level("DEBUG")) {
      debug = ConfigureLOG::ReportingLevel()-4;
      LOG(DEBUG) << "CommandLine sets debug to "
         << ConfigureLOG::ToString(ConfigureLOG::ReportingLevel());
   }

   // preliminaries ------------------------------------------------------
   helponly = foundErrors = false;
   Usage = string();
   Errors = string();
   Unrecog.clear();

   if(syntaxPageBuilt == 0) {
      word = string(argv[0]);
      string::size_type pos = word.find_last_of("/\\");
      if(pos != string::npos) word = word.substr(pos+1);
      syntaxPage = "Usage: " + word + " [option] ...";
      syntaxPageBuilt = 1;
   }
   if(syntaxPageBuilt == 1) {
      syntaxPage += "\n" + PrgmDesc + "\n";
      syntaxPageBuilt = 2;
   }

   // validate the command line ------------------------------------------
   if(!ValidateCommandLine(word)) {              // word is a dummy here
      Errors = word;
      return -3;
   }

   // build the syntax page ----------------------------------------------
   BuildSyntaxPage();

   // define usage by getting the syntax page
   Usage = SyntaxPage();

   // pre-processing -----------------------------------------------------
   // no args means help
   if(argc==1 && requireOneArg) helponly = true;

   // PreProcessArgs pulls out help, debug, verbose, file, deprecated and ignore args
   vector<string> Args;
   for(j=1; j<argc; j++) PreProcessArgs(argv[j],Args,Errors);
   LOG(DEBUG) << "Return from CommandLine::PreProcessArgs: help is "
      << (help ? "T":"F") << " and helponly is " << (helponly ? "T":"F");

   // PreProcessArgs may have removed all cmd line args....
   if((requireOneArg && Args.size() == 0) || helponly) help = true;

   if(debug >= 0) {
      ostringstream oss;
      oss << "CommandLine argument list passed to parser:" << endl;
      for(size_t k=0; k<Args.size(); k++)
         oss << " arg[" << k << "] = " << Args[k] << endl;

      word = oss.str();
      stripTrailing(word,'\n');
      LOG(DEBUG) << word;
   }

   // parse the command line ----------------------------------------------
   // fill values for each option, and Errors and Unrecog
   Parse(Args, Errors, Unrecog);

   // post-process: pull out the parsed input --------------------------------
   Postprocess(Errors, Unrecog);

   // help exit
   if(help) return 1;

   // error exit
   if(!Errors.empty()) {
      foundErrors = true;
      return -1;
   }

   // normal exit
   return 0;

}  // end try
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
// public
/**
 * @throw Exception
 */
void CommandLine::DumpConfiguration(ostream& os, string tag)
{
try {
   size_t i,j;
   double d;
   string str;
   RinexSatID sat;
   vector<string> values;
   string::size_type pos;

   // loop over all options, output of the form:
   // Description (--option) : value
   for(i=0; i<options.size(); i++) {
      if(!tag.empty()) os << tag << " ";
      if(options[i].predesc.size()) {
         str = options[i].predesc;
         if(str[0] == '#' || str[0] == '\n') ;     // do nothing
         else str = string("   ") + str;
         os << str << endl;
         if(!tag.empty()) os << tag << " ";
      }

      str = options[i].desc;
      os << "   " << str << " (--" << options[i].longOpt << ") : ";

      str = string();
      switch(options[i].type) {
         case typeBool:
            os << (*((bool *)(options[i].p_output)) ? "true" : "false");
            break;
         case typeInt:
            os << *((int *)(options[i].p_output));
            break;
         case typeVectorInt: {
               vector<int> ints(*((vector<int> *)(options[i].p_output)));
               if(ints.size() == 0)
                  os << "<none>";
               else for(j=0; j<ints.size(); j++)
                  os << ints[j] << (j==ints.size()-1 ? "" : ",");
            }
            break;
         case typeDouble:
            d = *((double *)(options[i].p_output));
            if(d == 0 || fabs(d) >= 0.1)
               os << fixed << setprecision(2) << d;
            else if(fabs(d) >= 0.01)
               os << fixed << setprecision(3) << d;
            else
               os << scientific << setprecision(2) << d;
            break;
         case typeString:
            if((*((string *)(options[i].p_output))).empty())
               os << "<none>";
            else
               str = *((string *)(options[i].p_output));
            break;
         case typeVectorString:
            values = *((vector<string> *)(options[i].p_output));
            if(values.size() == 0)
               os << "<none>";
            else for(j=0; j<values.size(); j++)
               str += values[j] + (j==values.size()-1 ? "" : ",");
            break;
         case typeSat:              // RinexSatID
            sat = *(RinexSatID *)(options[i].p_output);
            if(sat.id == -1)
               os << "<none>";
            else
               os << sat.toString();
            break;
         case typeVectorSat: {      // vector<RinexSatID>
               vector<RinexSatID> sats(*((vector<RinexSatID> *)(options[i].p_output)));
               if(sats.size() == 0) os << "<none>";
               else
                  for(j=0; j<sats.size(); j++)
                     str += sats[j].toString() + (j==sats.size()-1 ? "" : ",");
            }
            break;
         case typeUndefined:
         case typeCount:
            break;
      }  // end switch(type)

      if(!str.empty()) {
         pos = str.find_first_of(" \t");
         if(pos != string::npos) str = string("\"") + str + string("\"");
         os << str;
      }

      os << endl;

   }  // end loop over options

   // add verbose, debug and help
   os << "   Print extended output, including cmdline summary (--verbose) : "
            << (verbose?"true":"false") << endl;
   os << "   Print debug output at level DEBUGn [n=0-7] (--debug<n>) : "
            << debug << endl;
   os << "   Print this syntax page and quit (--help) : "
            << (help?"true":"false") << endl;

}  // end try
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// the rest are private
// -----------------------------------------------------------------------------------
/**
 * @throw Exception
 */
bool CommandLine::ValidateCommandLine(string& msg)
{
try {
   bool isValid(true);
   size_t i,j;
   string tag("Error (CommandLine): option ");
   ostringstream oss;

   LOG(DEBUG) << "CommandLine::ValidateCommandLine()";
   // are any valid options also being ignored?
   for(i=0; i<options.size(); i++) {
      string opt("--" + options[i].longOpt);
      //LOG(DEBUG) << "Is " << opt << " valid?";
      if(vectorindex(ignore_opts_with_arg,opt) != -1) {
         isValid = false;
         oss << tag << options[i].longOpt
            << " (with arg) is both valid and to be ignored.\n";
      }
      //LOG(DEBUG) << " ignore_opts_with_arg ok";
      if(vectorindex(ignore_opts_without_arg,opt) != -1) {
         isValid = false;
         oss << tag << options[i].longOpt
            << " (w/o arg) is both valid and to be ignored.\n";
      }
      //LOG(DEBUG) << " ignore_opts_without_arg ok";
      if(vectorindex(ignore_on_opts,opt) != -1) {
         isValid = false;
         oss << tag << options[i].longOpt
            << " is both valid and an 'ignore on' option.\n";
      }
      //LOG(DEBUG) << " ignore_on_opts ok";
      if(vectorindex(ignore_off_opts,opt) != -1) {
         isValid = false;
         oss << tag << options[i].longOpt
            << " is both valid and an 'ignore off' option.\n";
      }
      //LOG(DEBUG) << " ignore_off_opts ok";

      for(j=0; j<i; j++) {
         if(options[i].longOpt == options[j].longOpt) {
            isValid = false;
            oss << tag << "'" << options[i].longOpt << "' is repeated.\n";
         }
         if(options[i].shortOpt != 0 && options[i].shortOpt == options[j].shortOpt) {
            isValid = false;
            oss << tag << "'" << options[i].longOpt
               << "' short form is already used in option '" << options[j].longOpt
               << "'.\n";
         }
      }
   }

   // deprecated options
   map<string,string>::const_iterator it;
   for(it=deprec_opts.begin(); it != deprec_opts.end(); ++it) {
      LOG(DEBUG) << "Test deprec option " << it->first << " -> " << it->second;
      bool found(false);
      for(i=0; i<options.size(); i++) {
         if(it->second == string("--") + options[i].longOpt ||
            it->second == string("-") + options[i].shortOpt)
               { found=true; break; }
      }
      if(!found) {
         isValid = false;
         oss << tag << it->second << ", the replacement for deprecated option "
            << it->first << ", is not found.\n";
      }
   }
   //LOG(DEBUG) << " deprec_opts ok";

   msg = oss.str();
   if(!msg.empty()) { LOG(DEBUG) << "ValidateCommandLine finds " << msg; }

   return isValid;

}  // end try
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
/**
 * @throw Exception
 */
void CommandLine::BuildSyntaxPage(void)
{
try {
   size_t i,j,k;
   double d;
   string option,deflt;

   // first determine the max size of the string '--option <arg>'
   optionsize = 0;                        // member data
   for(i=0; i<options.size(); i++) {
      j = options[i].longOpt.length() + options[i].arg.length() + 7;
      if(j > optionsize) optionsize = j;
   }

   // build the command line ----------------------------------------
   for(i=0; i<options.size(); i++) {
      // ignore invalid types
      if(options[i].type < typeBool || options[i].type >= typeCount)
         continue;    // throw?

      // get the padded string ' --option <arg> '
      option = " --" + options[i].longOpt;
      if(options[i].arg.length()) option += " <" + options[i].arg + "> ";
      option = leftJustify(option,optionsize);
      
      // get the default string
      switch(options[i].type) {
         case typeBool:
            deflt = ( *(bool *)(options[i].p_output) ? "do" : "don't");
            break;
         case typeInt:
            deflt = asString(*(int *)(options[i].p_output));
            break;
         case typeVectorInt: {
               vector<int> *iptr = (vector<int> *)(options[i].p_output);
               deflt = string();
               for(j=0; j<iptr->size(); j++)
                  deflt += (j>0 ? ",":"") + asString((*iptr)[j]);
            }
            break;
         case typeDouble:
            d = *((double *)(options[i].p_output));
            if(d == 0 || fabs(d) >= 0.1)
               deflt = asString(d,2);
            else if(fabs(d) >= 0.01)
               deflt = asString(d,3);
            else
            {
               deflt = floatFormat(d, FFLead::NonZero, 3, 2);
            }
            break;
         case typeString:
            deflt = *(string *)(options[i].p_output);
            break;
         case typeVectorString: {
               vector<string> *sptr = (vector<string> *)(options[i].p_output);
               deflt = string("");
               for(j=0; j<sptr->size(); j++)
                  deflt += (j>0 ? ",":"") + (*sptr)[j];
            }
            break;
         case typeSat: {
               RinexSatID *gptr = (RinexSatID *)(options[i].p_output);
               if(gptr->id != -1) deflt = (*gptr).toString();
               else deflt = string("");
            }
            break;
         case typeVectorSat: {
               vector<RinexSatID> *gptr = (vector<RinexSatID> *)(options[i].p_output);
               deflt = string("");
               for(k=0,j=0; j<gptr->size(); j++)
                  if((*gptr)[j].id > 0)
                     deflt += (k++>0 ? ",":"") + (*gptr)[j].toString();
            }
            break;
         case typeUndefined:
         case typeCount:
            break;
      }

      // build the syntax line
      options[i].syntax = options[i].predesc
                        + (options[i].predesc.size() ? "\n" : "")
                        + option                                    // --opt <arg>
                        + options[i].desc                           // desc
            + (options[i].repeat ? " [repeatable]" : "")            // add 'repeat'
            + " (" + deflt + ")";                                   // add default
   }

}  // end try
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
// re-entrant!
/**
 * @throw Exception
 */
void CommandLine::PreProcessArgs(const char *in_arg, vector<string>& Args,
   string& Errors)
{
try {
   static bool found_cfg_file=false;
   static bool ignore_opts=false;              // ignore args for now...
   static bool ignore_once=false;              // ignore one argument
   int i,k;
   string msg,sarg(in_arg);

   if(sarg == string()) return;
   LOG(DEBUG) << "CommandLine::PreProcess arg " << sarg;

   if(ignore_once) { ignore_once = false; return; }

   // ignore these
   for(i=0; i<ignore_opts_without_arg.size(); i++) {     // note that -- is included
      if(sarg == ignore_opts_without_arg[i]) {
         LOG(DEBUG) << "CommandLine::PreProcess: ignoring option " << sarg;
         return;
      }
   }
   // ignore these and the follow arg
   for(i=0; i<ignore_opts_with_arg.size(); i++) {        // note that -- is included
      if(sarg == ignore_opts_with_arg[i]) {
         LOG(DEBUG) << "CommandLine::PreProcess: ignoring option " << sarg
            << " and its argument";
         ignore_once = true;
         return;
      }
   }
   // handle 'ignore on' args
   for(i=0; i<ignore_on_opts.size(); i++) {              // note that -- is included
      if(sarg == ignore_on_opts[i]) {
         LOG(DEBUG) << "CommandLine::PreProcess: start ignoring options: " << sarg;
         ignore_opts = true;
         return;
      }
   }
   // handle 'ignore off' args
   for(i=0; i<ignore_off_opts.size(); i++) {             // note that -- is included
      if(sarg == ignore_off_opts[i]) {
         LOG(DEBUG) << "CommandLine::PreProcess: stop ignoring options: " << sarg;
         ignore_opts = false;
         return;
      }
   }
   // it ignoring is 'on'
   if(ignore_opts) {
      LOG(DEBUG) << "CommandLine::PreProcess: ignoring option " << sarg;
      return;
   }

   // config file
   if(sarg == "--file" || sarg == "-f")         // strip out before found_cfg_file
      found_cfg_file = true;

   // NB second test (in_arg[0]...) is for -f <file> embedded in another cfg file *1*
   else if(found_cfg_file || (in_arg[0]=='-' && in_arg[1]=='f')) {
      string filename(in_arg);
      if(!found_cfg_file) filename.erase(0,2); else found_cfg_file = false;
      LOG(DEBUG) << "CommandLine::PreProcess found a file of options: " << filename;
      ifstream infile(filename.c_str());
      if(!infile) {
         LOG(ERROR) << "Error: could not open options file " << filename;
         return;
      }

      bool again_cfg_file=false;
      string buffer,word;
      while(1) {
         getline(infile,buffer);
         stripTrailing(buffer,'\r');
         //LOG(DEBUG) << "Line in file " << filename << " is " << buffer;

         // process the buffer before checking eof or bad b/c there can be
         // a line at EOF that has no CRLF...
         while(!buffer.empty()) {
            word = firstWord(buffer);
            if(again_cfg_file) {
               word = "-f" + word;           // see above *1*
               again_cfg_file = false;
               PreProcessArgs(word.c_str(),Args,Errors);
            }
            else if(word[0] == '#') {        // skip to end of line
               buffer = "";
            }
            else if(word == "--file" || word == "-f")
               again_cfg_file = true;
            else if(word[0] == '"') {
               word = stripFirstWord(buffer,'"');
               buffer = "dummy " + buffer;   // to be stripped below
               PreProcessArgs(word.c_str(),Args,Errors);
            }
            else
               PreProcessArgs(word.c_str(),Args,Errors);

            word = stripFirstWord(buffer);
         }
         if(infile.eof() || !infile.good()) break;
      }  // end loop over file

      ignore_opts = false;
   }

   // other args to strip out
   else if(sarg==string("-h") || sarg==string("--help")) {
      help = true;
      LOG(DEBUG) << "CommandLine::PreProcess found help option";
   }
   else if(sarg.substr(0,2)==string("-d") || sarg.substr(0,7)==string("--debug")) {
      string level("DEBUG");
      if(sarg==string("-d") || sarg==string("--debug"))
         debug = 0;
      else
         debug = asInt(sarg[1]=='d' ? sarg.substr(2) : sarg.substr(7));

      if(debug >= 0 && debug <= 7) {
         if(debug > 0) level += asString(debug);
         ConfigureLOG::ReportingLevel() = ConfigureLOG::Level(level.c_str());
         LOG(DEBUG) << "CommandLine found debug option at level " << debug
            << ", logging level "
            << ConfigureLOG::ToString(ConfigureLOG::ReportingLevel());
         verbose = true;            // NB debug implies verbose
      }
   }
   else if(sarg==string("-v") || sarg==string("--verbose")) {
      verbose = true;
      // do NOT overwrite debug setting else serious subtle problems
      if(debug == -1)
         ConfigureLOG::ReportingLevel() = ConfigureLOG::Level("VERBOSE");
      LOG(DEBUG) << "CommandLine::PreProcess found the verbose option";
   }

   // deprecated options
   // note that -- is included in both key and value
   else if(deprec_opts.find(sarg) != deprec_opts.end())
      Args.push_back(deprec_opts[sarg]);

   // regular argument
   else {
      LOG(DEBUG) << "CommandLine::PreProcess found regular arg " << sarg;
      Args.push_back(sarg);
   }
}
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
/**
 * @throw Exception
 */
void expand_args(vector<string>& oldvalues, vector<string>& newvalues, string& msg)
{
try {
   string arg;

   for(size_t k=0; k<oldvalues.size(); k++) {      // first split values on ','
      while(!(arg = stripFirstWord(oldvalues[k],',')).empty()) {
         if(arg.substr(0,1) == "@") {              // list file
            // remove the '@'
            string filename(arg.substr(1));

            // expand ~ in the filename
            expand_filename(filename);

            // open the file, read it and get new values (expandtilde.cpp)
            if(!expand_list_file(filename,newvalues)) {
               msg += "  Error - Argument list file " + filename
                     + " could not be opened.";
               continue;
            }
            else LOG(DEBUG) << "Opened arg list file " << filename;
         }
         else                                     // just a value
            newvalues.push_back(arg);

      }  // end while
   }  // end for
}  // end try
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
// fill values for each option, and Errors and Unrecog
/**
 * @throw Exception
 */
void CommandLine::Parse(vector<string>& Args, string& Errors, vector<string>& Unrecog)
{
try {
   size_t i,j;
   string arg,val;

   // parse args, saving values -----------------------
   i = 0;
   while(i < Args.size()) {
      arg = Args[i];                               // arg
      // does it match an option?
      for(j=0; j<options.size(); ++j) {
         if((arg.size() == 2 && arg[0] == '-' && arg[1] == options[j].shortOpt) ||
               (arg.substr(0,2) == "--" && arg.substr(2) == options[j].longOpt))
         {                                         // match
            if(options[j].type > typeBool) {       // find value
               if(i == Args.size()-1) {
                  Errors += "Error - option " + arg + " without value.\n";
                  break;
               }
               else if(Args[i+1].substr(0,2) == "--") {
                  Errors += "Error - option " + arg + " without value.\n";
                  break;
               }
               else val = Args[++i];               // value
            }
            else val = string("T");                // bool 'value'

            LOG(DEBUG) << "CommandLine::Parse found arg[" << i << "] "
               << arg << " = " << val;
            options[j].values.push_back(val);      // save it

            break;
         }  // end match
      }  // end loop over options

      if(j == options.size()) {
         Unrecog.push_back(Args[i]);
         LOG(DEBUG) << "CommandLine::Parse found unrecognized arg[" << i << "] "
            << Args[i];
      }

      // next arg
      ++i;

   }  // end loop over args

   // were all required options found? -----------------------
   // were any non-repeatable options repeated? --------------
   for(j=0; j<options.size(); ++j) {

      // do this here, after PreProcArguments() and before the code below
      // define callers special targets, using values found in PreProcessArgs
      if(help && options[j].longOpt == string("help")) {
         LOG(DEBUG) << "CommandLine::Parse found help option and help";
         *((bool *)(options[j].p_output)) = help;
         options[j].values.push_back(string("T"));
      }
      else if(verbose && options[j].longOpt == string("verbose")) {
         LOG(DEBUG) << "CommandLine::Parse found verbose option and verbose";
         *((bool *)(options[j].p_output)) = verbose;
         options[j].values.push_back(string("T"));
      }
      else if(debug > -1 && options[j].longOpt == string("debug")) {
         LOG(DEBUG) << "CommandLine::Parse found debug option and debug = " << debug;
         *((int *)(options[j].p_output)) = debug;
         options[j].values.push_back(asString(debug));
      }

      // required options that are not found
      if(options[j].required && options[j].values.size() == 0)
         Errors += "Required option " + options[j].longOpt + " is not found.\n";

      // non-repeatable options that are repeated
      if(!options[j].repeat && options[j].values.size() > 1)
         Errors += "Not-repeatable option "+options[j].longOpt+" was repeated.\n";
   }
   
}
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
/**
 * @throw Exception
 */
string CommandLine::SyntaxPage(void)
{
try {
   if(syntaxPageBuilt == 2) {
      if(options.size() > 0)
         for(int i=0; i<options.size(); i++)
            if(options[i].doc)
               syntaxPage += options[i].syntax + "\n";

      // add verbose, debug and help, which are always there...
      syntaxPage += //"# (automatic options)\n" +
                    leftJustify(" --verbose",optionsize)
                  + "Print extended output, including cmdline summary (don't)\n";
      syntaxPage += leftJustify(" --debug<n>",optionsize)
                  + "Print debug output at LOGlevel n [n=0-7] (-1)\n";
      syntaxPage += leftJustify(" --help",optionsize)
                  + "Print this syntax page and quit (don't)";

      syntaxPageBuilt = 3;
   }

   return syntaxPage;
}
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
/**
 * @throw Exception
 */
void CommandLine::Postprocess(string& Errors, vector<string>& Unrecog)
{
try {
   size_t i,k;
   RinexSatID sat;
   string msg,errStr;
   vector<string> values;
   ostringstream oss;

   // loop over options, parse out @file and value,value, and assign targets
   for(i=0; i<options.size(); i++) {

      // was the option even found on the command line?
      if(options[i].values.size() == 0) continue;
      LOG(DEBUG) << "CommandLine::Postprocess parse " << options[i].longOpt
         << " of type " << options[i].type << (options[i].doc ? "":" undocumented");
      
      // bool is special b/c values are ignored
      if(options[i].type == typeBool) {
         *((bool *)(options[i].p_output)) = ( options[i].toggle ? 
                  ! (*((bool *)(options[i].p_output))) :    true );
         continue;
      }

      // get the array of values (strings)
      values = options[i].values;

      // parse out 'value,value' and '@file.lst'
      if(options[i].expand && (options[i].type == typeVectorString ||
                               options[i].type == typeVectorSat ||
                               options[i].type == typeVectorInt))
      {
         vector<string> nvalues;
         expand_args(values, nvalues, errStr);
         if(!errStr.empty())
            oss << errStr << " for option --" << options[i].longOpt << "\n";
         values = nvalues;
      }

      switch(options[i].type) {
         case typeBool:                 // bool -- done above
            break;

         case typeInt:
            if(!isDigitString(values[0]))
               oss << "Error: non-integer value for --" << options[i].longOpt
                  << ": " << values[0] << endl;
            else
               *((int *)(options[i].p_output)) = asInt(values[0]);
            break;

         case typeVectorInt:
            for(k=0; k<values.size(); k++) {
               if(!isDigitString(values[k]))
                  oss << "Error: non-integer value for --" << options[i].longOpt
                     << ": " << values[k] << endl;
               else
                  ((vector<int> *)(options[i].p_output))->push_back(asInt(values[k]));
            }
            break;

         case typeDouble:
            if(!isScientificString(values[0]))
               oss << "Error: invalid value for --" << options[i].longOpt
                  << ": " << values[0] << endl;
            else
               *((double *)(options[i].p_output))=asDouble(values[0]);
            break;

         case typeString:
            *((string *)(options[i].p_output)) = values[0];
            break;

         case typeVectorString:
            for(k=0; k<values.size(); k++)
               ((vector<string> *)(options[i].p_output))->push_back(values[k]);
            break;

         case typeSat:
            sat = RinexSatID(values[0]);
            *(RinexSatID *)(options[i].p_output) = sat;
            break;

         case typeVectorSat:
            for(k=0; k<values.size(); k++) {
               sat = RinexSatID(values[k]);
               ((vector<RinexSatID> *)(options[i].p_output))->push_back(sat);
            }
            break;
         case typeUndefined:
         case typeCount:
            break;
      }

   }  // end loop over options

   // NO! ResCor has commas in REdit cmds.
   // parse out @file and value,value from unrecognized args
   //vector<string> nvalues;
   //expand_args(Unrecog, nvalues, errStr);
   //if(!errStr.empty()) Errors += errStr + " (an unrecognized arg)\n";
   //Unrecog = nvalues;

   Errors += oss.str();

}  // end try
catch(Exception& e) { GPSTK_RETHROW(e); }
catch(std::exception& e) {
   Exception E("std except: "+string(e.what())); GPSTK_THROW(E);
}
catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
