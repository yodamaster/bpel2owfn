/*****************************************************************************\
  GNU BPEL2oWFN -- Translating BPEL Processes into Petri Net Models

  Copyright (C) 2009        Niels Lohmann
  Copyright (C) 2006, 2007  Niels Lohmann,
                            Christian Gierds, and
                            Martin Znamirowski
  Copyright (C) 2005        Niels Lohmann and
                            Christian Gierds

  GNU BPEL2oWFN is free software: you can redistribute it and/or modify it
  under the terms of the GNU Affero General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  GNU BPEL2oWFN is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
  details.

  You should have received a copy of the GNU Affero General Public License
  along with GNU BPEL2oWFN. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

/*!
 * \file    bpel2owfn.cc
 *
 * \brief   BPEL2oWFN's main
 *
 * \author  Niels Lohmann <nlohmann@informatik.hu-berlin.de>,
 *          Christian Gierds <gierds@informatik.hu-berlin.de>,
 *          Martin Znamirowski <znamirow@informatik.hu-berlin.de>,
 *          last changes of: \$Author: nielslohmann $
 *
 * \since   2005/10/18
 *
 * \date    \$Date: 2007/08/07 12:01:37 $
 *
 * \note    This file is part of the tool BPEL2oWFN and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universität zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 * \version \$Revision: 1.194 $
 */





/******************************************************************************
 * Headers
 *****************************************************************************/

#include <config.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>

#include "petrinet.h"           // Petri Net support
#include "cfg.h"        // Control Flow Graph
#include "debug.h"      // debugging help
#include "options.h"
#include "ast-config.h"
#include "ast-details.h"
#include "globals.h"
#include "cmdline.h"


using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::map;
using namespace PNapi;





/*!
 * \brief the command line options
 *
 * Stores all information on the command line options.
 */
gengetopt_args_info args_info;





/******************************************************************************
 * External functions
 *****************************************************************************/

extern int frontend_parse();            // from Bison
extern int frontend_chor_parse();       // from Bison
extern int frontend_wsdl_parse();       // from Bison
extern int frontend_debug;          // from Bison
extern int frontend_nerrs;          // from Bison
extern int frontend_lineno;         // from Bison
extern int frontend__flex_debug;        // from flex
extern FILE* frontend_in;           // from flex
extern void frontend_restart(FILE*);        // from flex

extern int frontend_owfn_debug;         // from Bison
extern int frontend_owfn_flex_debug;

extern FILE* frontend_owfn_in;
extern int frontend_owfn_error();
extern int frontend_owfn_parse();

extern int frontend_pnml_debug;         // from Bison

extern int frontend_pnml_parse();




/******************************************************************************
 * Global variables
 *****************************************************************************/

/// The Petri Nets
PetriNet PN = PetriNet();
PetriNet PN2 = PetriNet();

/******************************************************************************
 * program parts
 *****************************************************************************/


// analyzation of the commandline
void analyze_cl(int argc, char* argv[]) {
    // setting globals
    for (int i = 0; i < argc; i++) {
        globals::invocation += string(argv[i]);
        if (i != (argc - 1)) {
            globals::invocation += " ";
        }
    }

    /*
     * Reading command line arguments and triggering appropriate behaviour.
     * In case of false parameters call command line help function and exit.
     */
    parse_command_line(argc, argv);

}




// opening a file
void open_file(string file) {
    if (inputfiles.size() >= 1) {
        globals::filename = file;
        if (!(frontend_in = fopen(globals::filename.c_str(), "r"))) {
            cerr << "Could not open file for reading: " << globals::filename.c_str() << endl;
            exit(2);
        }
    }
}




// closing a file
void close_file(string file) {
    if (globals::filename != "<STDIN>" && frontend_in != NULL) {
        trace(TRACE_INFORMATION, " + Closing input file: " + globals::filename + "\n");
        fclose(frontend_in);
        frontend_in = NULL;
    }
}




// finishing AST
void finish_AST() {
    // apply first set of rewrite rules
    trace(TRACE_INFORMATION, "Rewriting...\n");
    globals::AST = globals::AST->rewrite(kc::invoke);
    globals::AST = globals::AST->rewrite(kc::implicit);
    trace(TRACE_INFORMATION, "Rewriting complete...\n");

    // postprocess and annotate the AST
    trace(TRACE_INFORMATION, "Postprocessing...\n");
    globals::AST->unparse(kc::printer, kc::postprocessing);
    trace(TRACE_INFORMATION, "Postprocessing complete...\n");

    // apply second set of rewrite rules
    trace(TRACE_INFORMATION, "Rewriting 2...\n");
    globals::AST = globals::AST->rewrite(kc::newNames);
    trace(TRACE_INFORMATION, "Rewriting 2 complete...\n");

    // an experiment
    //      cout << "digraph G{" << endl;
    //      globals::ASTEmap[1]->output();
    //      cout << "}" << endl;
}




// output of every single processed file
void single_output(set< string >::iterator file) {
    // print the AST?
    if (modus == M_AST) {
        trace(TRACE_INFORMATION, "-> Printing AST ...\n");
        if (formats[F_DOT]) {
            string dot_filename = globals::output_filename + "." + suffixes[F_DOT];
            FILE* dotfile = fopen(dot_filename.c_str(), "w+");
            globals::AST->fprintdot(dotfile, "", "", "", true, true, true);
            fclose(dotfile);
#ifdef HAVE_DOT
            string systemcall = "dot -q -Tpng -o\"" + globals::output_filename + ".png\" \"" + globals::output_filename + "." + suffixes[F_DOT] + "\"";
            trace(TRACE_INFORMATION, "Invoking dot with the following options:\n");
            trace(TRACE_INFORMATION, systemcall + "\n\n");
            system(systemcall.c_str());
#endif
        } else {
            globals::AST->print();
        }
    }

    // print the Visualization?
    if (modus == M_VIS) {
        trace(TRACE_INFORMATION, "-> Printing the process visualization ...\n");
        if (formats[F_DOT]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_DOT]);
            }
            trace(TRACE_INFORMATION, "-> Printing it in dot format ...\n");
            globals::AST->unparse(kc::printer, kc::visualization);
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
#ifdef HAVE_DOT
            string systemcall = "dot -q -Tpng -o\"" + globals::output_filename + ".png\" \"" + globals::output_filename + "." + suffixes[F_DOT] + "\"";
            trace(TRACE_INFORMATION, "Invoking dot with the following options:\n");
            trace(TRACE_INFORMATION, systemcall + "\n\n");
            system(systemcall.c_str());
#endif
        } else {
            globals::AST->print();
        }
    }

    if (modus == M_PRETTY) {
        if (formats[F_XML]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_XML]);
            }
            trace(TRACE_INFORMATION, "-> Printing \"pretty\" XML ...\n");
            globals::AST->unparse(kc::printer, kc::xml);
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }
    }

    // generate and process the control flow graph?
    if (modus == M_CFG) {
        processCFG();
    }

    // print information about the process
    show_process_information();




    if (modus == M_PETRINET || modus == M_CHOREOGRAPHY) {
        // choose Petri net patterns
        if (globals::parameters[P_COMMUNICATIONONLY] == true) {
            globals::AST->unparse(kc::pseudoPrinter, kc::petrinetsmall);
        } else {
            globals::AST->unparse(kc::pseudoPrinter, kc::petrinetnew);
        }

        // calculate maximum occurences
        PN.calculate_max_occurrences();

        // structurally reducte the Petri net
        if (globals::reduction_level > 0) {
            trace(TRACE_INFORMATION, "-> Structurally simplifying Petri Net ...\n");
            PN.reduce(globals::reduction_level);
        }

        if (modus == M_CHOREOGRAPHY) {
            // case 1: no instance of this process is needed
            if (globals::instances_of_current_process == 0) {
                // add a prefix and compose PN to PN2
                PN.addPrefix(globals::ASTEmap[1]->attributes["name"] + ".");
                PN2.compose(PN);
            } else { // case 2: one or more instances of this process are needed
                for (int instance = 1; instance <= globals::instances_of_current_process; instance++) {
                    std::cerr << "instance " << instance << "/" << globals::instances_of_current_process << " " << PN2.information() << std::endl;

                    PetriNet PN3 = PN;
                    PN3.addPrefix(globals::ASTEmap[1]->attributes["name"] + "_" + toString(instance) + ".");
                    PN3.add_interface_suffix(".instance_" + toString(instance));
                    PN2.compose(PN3);
                }
            }

            // reset data structures
            PN = PetriNet();
            globals::AST = NULL;
        }
    }


    /*

      // generate a Petri net (w/o BPEL4Chor)?
      if ((globals::choreography_filename == "") &&
        (modus == M_PETRINET || modus == M_CHOREOGRAPHY))
      {
        trace(TRACE_INFORMATION, "-> Unparsing AST to Petri net ...\n");
        // choose Petri net patterns
        if (globals::parameters[P_COMMUNICATIONONLY] == true)
          globals::AST->unparse(kc::pseudoPrinter, kc::petrinetsmall);
        else
          globals::AST->unparse(kc::pseudoPrinter, kc::petrinetnew);

        // calculate maximum occurences
        PN.calculate_max_occurrences();
        if (modus == M_CHOREOGRAPHY)
        {
          unsigned int pos = file->rfind(".bpel", file->length());
          unsigned int pos2 = file->rfind("/", file->length());
          string prefix = "";
          if (pos == (file->length() - 5))
          {
            prefix = file->substr(pos2 + 1, pos - pos2 - 1) + "_";
          }

          if (globals::reduction_level > 0)
          {
        trace(TRACE_INFORMATION, "-> Structurally simplifying Petri Net ...\n");
        PN.reduce(globals::reduction_level);
          }

          PN.addPrefix(prefix);
          PN2.compose(PN);
          PN = PetriNet();

          globals::AST = NULL;
          }
        }*/
}




// Final Output of the result
void final_output() {
    if (modus == M_CHOREOGRAPHY) {
        PN = PN2;
    }


    if (modus == M_PETRINET || modus == M_CHOREOGRAPHY) {
        if (globals::reduction_level > 0) {
            trace(TRACE_INFORMATION, "-> Structurally simplifying Petri Net ...\n");
            PN.reduce(globals::reduction_level);
        }


        // Removes the deadlock from the final place in case of a deadlocktest
        if (globals::parameters[P_DEADLOCKTEST]) {
            PN.loop_final_state();
        }


        // now the net will not change any more, thus the nodes are re-enumerated
        // and the maximal occurrences of the nodes are calculated.
        PN.reenumerate();
        //  PN.calculate_max_occurrences();
        cerr << PN.information() << endl;


        // create oWFN output ?
        if (formats[F_OWFN]) {
            if (globals::output_filename != "") {
                if (options[O_NET] && globals::net_mode == OWFN) {
                    output = openOutput(globals::output_filename + "Gen." + suffixes[F_OWFN]);
                    trace(TRACE_ALWAYS, "Output filename for owfn2owfn is [OLDNAME]Gen.owfn \n");
                } else {
                    output = openOutput(globals::output_filename + "." + suffixes[F_OWFN]);
                }
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for oWFN ...\n");
            PN.set_format(FORMAT_OWFN);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }


        // create LoLA output ?
        if (formats[F_LOLA]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_LOLA]);
            }
            if (modus == M_CHOREOGRAPHY) {
                PN.makeChannelsInternal();
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for LoLA ...\n");
            PN.set_format(FORMAT_LOLA);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }

            if (modus == M_CHOREOGRAPHY || modus == M_PETRINET) {
                if (globals::output_filename != "") {
                    output = openOutput(globals::output_filename + ".task");
                }
                string comment = "{ AG EF (";
                string formula = "FORMULA\n  ALLPATH ALWAYS EXPATH EVENTUALLY (";
                string andStr = "";

                set< Place* > finalPlaces = PN.getFinalPlaces();
                for (set< Place* >::iterator place = finalPlaces.begin(); place != finalPlaces.end(); place++) {
                    comment += andStr + (*place)->nodeFullName();
                    formula += andStr + (*place)->nodeShortName() + " > 0";
                    andStr = " AND ";
                }
                comment += ") }";
                formula += ")";
                (*output) << comment << endl << endl;
                (*output) << formula << endl << endl;
                if (globals::output_filename != "") {
                    closeOutput(output);
                    output = NULL;
                }
            }
        }


        // create PNML output ?
        if (formats[F_PNML]) {
            if (globals::output_filename != "") {
                if (options[O_NET] && globals::net_mode == PNML) {
                    output = openOutput(globals::output_filename + "Gen." + suffixes[F_PNML]);
                    trace(TRACE_ALWAYS, "Output filename for pnml2pnml is [OLDNAME]Gen.pnml \n");
                } else {
                    output = openOutput(globals::output_filename + "." + suffixes[F_PNML]);
                }
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for PNML ...\n");
            PN.set_format(FORMAT_PNML);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }


        // create I/O-annotated LoLA output ?
        if (formats[F_IOLOLA]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_IOLOLA]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for I/O-annotated LoLA ...\n");
            PN.set_format(FORMAT_IOLOLA);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }


        // create PEP output ?
        if (formats[F_PEP]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_PEP]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for PEP ...\n");
            PN.set_format(FORMAT_PEP);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }


        // create INA output ?
        if (formats[F_INA]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_INA]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for INA ...\n");
            PN.set_format(FORMAT_INA);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }

        // create SPIN output ?
        if (formats[F_SPIN]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_SPIN]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for SPIN ...\n");
            PN.set_format(FORMAT_SPIN);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }

        // create APNN output ?
        if (formats[F_APNN]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_APNN]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for APNN ...\n");
            PN.set_format(FORMAT_APNN);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }


        // create TPN output ?
        if (formats[F_TPN]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_TPN]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for TPN ...\n");
            PN.set_format(FORMAT_TPN);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }


        // create dot output ?
        if (formats[F_DOT]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_DOT]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net for dot ...\n");
            // if parameter "nointerface" is set, set dot format with parameter "false"
            // not to draw an interface
            PN.set_format(FORMAT_DOT, !globals::parameters[P_NOINTERFACE]);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;

#ifdef HAVE_DOT
                string systemcall = "dot -q -Tpng -o\"" + globals::output_filename + ".png\" \"" + globals::output_filename + "." + suffixes[F_DOT] + "\"";
                trace(TRACE_INFORMATION, "Invoking dot with the following options:\n");
                trace(TRACE_INFORMATION, systemcall + "\n\n");
                system(systemcall.c_str());
#endif
            }
        }


        // create info file ?
        if (formats[F_INFO]) {
            if (globals::output_filename != "") {
                output = openOutput(globals::output_filename + "." + suffixes[F_INFO]);
            }
            trace(TRACE_INFORMATION, "-> Printing Petri net information ...\n");
            PN.set_format(FORMAT_INFO);
            (*output) << PN;
            if (globals::output_filename != "") {
                closeOutput(output);
                output = NULL;
            }
        }
    }
}





/*!
 * \brief use free choice conflict clusters to decompose the net
 *
 * This function determines the free choice conflict clusters of the Petri net
 * created so far (assumes -m petrinet) and creates a net for each combination
 * of conflict resolution. The filenames are suffixed with increasing numbers.
 *
 */
void decompostion() {
    PetriNet original(PN);
    string original_filename = globals::output_filename;


    vector< vector<string> > clusters = PN.getFreeChoiceClusters();
    vector<unsigned int> current_index;
    vector<unsigned int> max_index;
    unsigned int decompositions = 1;

    for (unsigned int i = 0; i < clusters.size(); i++) {
        current_index.push_back(0);
        max_index.push_back(clusters[i].size());

        decompositions *= clusters[i].size();
    }

    assert(current_index.size() == clusters.size());
    assert(max_index.size() == clusters.size());

    cerr << decompositions << " decompositions of this net" << endl;

    for (unsigned int i = 0; i < decompositions; i++) {
        cerr << "decomposition " << i + 1 << " of " << decompositions << endl;

        // get next index
        for (unsigned int j = 0; j < current_index.size(); j++) {
            if (current_index[j] < max_index[j] - 1) {
                current_index[j]++;
                break;
            } else {
                current_index[j] = 0;
            }
        }


        PN = original;

        for (unsigned int j = 0; j < clusters.size(); j++) {
            for (unsigned int k = 0; k < clusters[j].size(); k++) {
                if (k == current_index[j]) {
                    PN.removeTransition(PN.findTransition(clusters[j][k]));
                }
            }
        }

        globals::output_filename = original_filename + toString(i);
        final_output();
    }
}





/******************************************************************************
 * main() function
 *****************************************************************************/

/*!
 * \brief entry point of BPEL2oWFN
 *
 * Controls the behaviour of input and output.
 *
 * \param argc  number of command line arguments
 * \param argv  array with command line arguments
 *
 * \returns 0 if everything went well
 * \returns 1 if an error occurred
 *
 * \todo The opening and closing of files is awkward.
 *
 * \bug  Piping does not always work...
 */
int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--bug") {
        printf("\n\n");
        printf("Please email the following information to %s:\n", PACKAGE_BUGREPORT);
        printf("- tool:              %s\n", PACKAGE_NAME);
        printf("- version:           %s\n", PACKAGE_VERSION);
        printf("- compilation date:  %s\n", __DATE__);
        printf("- compiler version:  %s\n", __VERSION__);
        printf("- platform:          %s\n", BUILDSYSTEM);
        printf("\n\n");
        return EXIT_SUCCESS;
    }


    //---------------------------------------------------------------------------
    // process command line options

    struct cmdline_parser_params* params;

    // set default values
    cmdline_parser_init(&args_info);

    // initialize the parameters structure
    params = cmdline_parser_params_create();

    // call the cmdline parser
    if (cmdline_parser(argc, argv, &args_info) != 0) {
        exit(EXIT_FAILURE);
    }
    //---------------------------------------------------------------------------


    // initilization of variables
    PetriNet PN2 = PetriNet();


    // analyzation of the commandline
    analyze_cl(argc, argv);



    /*
     * Choreography: parse a BPEL4Chor file
     */
    if (globals::choreography_filename != "") {
        open_file(globals::choreography_filename);

        show_process_information_header();

        // invoke Bison BPEL4Chor parser
        trace(TRACE_INFORMATION, "Parsing " + globals::choreography_filename + " ...\n");
        int parse_result = frontend_chor_parse();
        trace(TRACE_INFORMATION, "Parsing of " + globals::choreography_filename + " complete.\n");

        close_file(globals::choreography_filename);

        if (parse_result != 0) {
            genericError(122, "", toString(frontend_lineno), ERRORLEVER_WARNING);
        }
    }



    /*
     * parse an optional WSDL file
     */
    if (globals::wsdl_filename != "") {
        open_file(globals::wsdl_filename);

        show_process_information_header();

        // invoke Bison BPEL4WSDL parser
        trace(TRACE_INFORMATION, "Parsing " + globals::wsdl_filename + " ...\n");
        int parse_result = frontend_wsdl_parse();
        trace(TRACE_INFORMATION, "Parsing of " + globals::wsdl_filename + " complete.\n");

        close_file(globals::wsdl_filename);

        if (parse_result != 0) {
            genericError(123, "", toString(frontend_lineno), ERRORLEVER_WARNING);
        }

        show_wsdl_information();
    }

    // parsing all inputfiles
    set< string >::iterator file = inputfiles.begin();
    do {
        if (file != inputfiles.end()) {
            open_file(*file);

            // reset the parser
            frontend_restart(frontend_in);
        }

        show_process_information_header();


        // invoke BPEL Bison parser
        trace(TRACE_INFORMATION, "Parsing " + globals::filename + " ...\n");
        frontend_parse();
        trace(TRACE_INFORMATION, "Parsing of " + globals::filename + " complete.\n");

        globals::parsing = false;

        if (file != inputfiles.end()) {
            close_file(*file);
        }

        if (frontend_nerrs == 0) {
            finish_AST();
            single_output(file);
        } else { /* parse error */
            if (globals::AST == NULL) {
                genericError(104, "", toString(frontend_lineno), ERRORLEVEL_CRITICAL);

                cleanup();
                return 3;
            } else {
                genericError(105, "", toString(frontend_lineno), ERRORLEVEL_NOTICE);

                finish_AST();
                single_output(file);
            }
        }


        // in case we are reading from standard input, we are done now
        if (globals::filename == "<STDIN>") {
            break;
        }


        file++;

    } while (modus == M_CHOREOGRAPHY && file != inputfiles.end());

    trace(TRACE_INFORMATION, "All files have been parsed.\n");


    // decomposition
    if (globals::parameters[P_DECOMP]) {
        decompostion();
    }


    final_output();

    if (debug_level != TRACE_ERROR) {
        cerr << "==============================================================================" << endl << endl;
    }

    // everything went fine
    return EXIT_SUCCESS;
}





/*!
 * \defgroup frontend Front End
 * \defgroup patterns Petri Net Patterns
 */
