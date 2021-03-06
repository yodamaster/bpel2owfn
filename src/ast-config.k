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

%{
/*!
 * \file    ast-config.cc
 *
 * \brief   Configuration of the AST
 *
 * \author  Niels Lohmann <nlohmann@informatik.hu-berlin.de>,
 *          last changes of: \$Author: nielslohmann $
 * 
 * \since   2006-24-03
 *
 * \date    \$Date: 2007/06/28 07:38:14 $
 * 
 * \note    This file is part of the tool GNU BPEL2oWFN and was created during
 *          the project "Tools4BPEL" at the Humboldt-Universität zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 * \note    This file was created using Kimwitu++ reading file bpel-abstract.k.
 *          See http://site.informatik.hu-berlin.de/kimwitu++ for details.
 *
 * \version \$Revision: 1.55 $
 *
 * \todo    Annotate the global mappings which function clear them.
 */
%}


/******************************************************************************
 * THE HEADER FOR ALL KIMWITU++ USERS (ast-config.h)
 *****************************************************************************/

%{ HEADER

/*!
 * \file ast-config.h
 * \brief Configuration of the AST (interface)
 *
 * See \ref ast-config.cc for more information.
 */

#include <config.h>
#include <climits>

#include "ast-system-k.h"	// phylum definitions
#include "ast-system-rk.h"	// rewrite rules
#include "ast-system-unpk.h"	// unparse rules
#include "ast-system-yystype.h" // data types for tokens and non-terminals

extern string forEach_suffix;
%}





/******************************************************************************
  UNPARSING FEATURES
******************************************************************************/

// the unparse views

%uview petrinetsmall;
%uview petrinetnew;
%uview xml;
%uview cfg;
%uview postprocessing;
%uview evaluation;
%uview visualization;


// the header file

%{ KC_UNPARSE
#include <config.h>
#include <iostream>
#include <cmath>
#include <map>
#include <set>
#include <climits>
#include <string>

#include "petrinet-patterns-basic.h"
#include "petrinet-patterns-structured.h"
#include "petrinet-patterns-scopes.h"
#include "petrinet-patterns-handlers.h"
#include "petrinet-patterns-links.h"
#include "extension-wsdl.h"			// for class WSDL
#include "options.h"
#include "cfg.h"
#include "helpers.h"
#include "ast-details.h"
#include "ast-tools.h"
#include "globals.h"
#include "extension-chor.h"

using std::endl;
using std::cerr;

extern PetriNet PN;				// introduced in main.c
extern CFGBlock * CFG;				// introduced in cfg.cc
%}


// All the includes, variables and structs defined here can be used in the
// printers below.
%{
#include <iostream>
#include "options.h" // for #output
%}


// the printer functions

/// "fake" printer-function
void pseudoPrinter(const char *s, uview v) { }


/// printer-function for output on stdout
void printer(const char *s, uview v)
{
  *output << s;
}





/******************************************************************************
  PHYLUM DECLARATIONS
******************************************************************************/

// all class declarations generated from phylum definitions; included by all
// implicitly generated files

%{ KC_TYPES_HEADER
#include <string>
#include <iostream>
#include <cstring>
#include <map>

#include "debug.h"	// for (SAerror, trace)
#include "helpers.h"	// for (toString)

/*!
 * \brief  a preprocessor directive for the quite generic constructors
 *
 *         This preprocessor defines a constructor for ASTE objects. It is
 *         invoked by the constructors of the AST phyla. The latter constructor
 *         can be invoked explictly by the BPEL parser (defined in in
 *         frontend-parser.yy), or implicitly during rewriting.
 *
 *         Each AST phylum has an integer value defined which is interpreted as
 *         the identifier of this phylum. If this value is non-zero, it is
 *         directly passed to the ASTE constructor and used in mapping
 *         globals::ASTEmap to reference to the newly created object. If,
 *         however, the identifier has the value 0, the value of the global
 *         variable globals::ASTEid is used as identifier. The latter case is
 *         used if an AST phylum is added by an unparser an thus cannot be
 *         linked to any information of the parser such as an attribute map.
 *
 * \param  token  The token name (e.g. "K_PROCESS" for the <process>) which is
 *                passed to the ASTE constructor.
 */
#define CTOR(token) { \
  ENTER("[AST]"); \
  assert(integer_1 != NULL); \
  bool id_was_zero = (integer_1->value == 0); \
  if (integer_1->value == 0) \
  { \
    id = globals::ASTEid; \
    integer_1 = mkinteger(globals::ASTEid++); \
  } \
  else { \
    id = integer_1->value; \
  } \
  if (globals::ASTEmap[id] == NULL) \
  { \
    globals::ASTEmap[id] = new ASTE(id, token); \
  } \
  assert(globals::ASTEmap[id] != NULL); \
  globals::ASTEmap[id]->phylum = this; \
  if (id_was_zero) \
    globals::ASTEmap[id]->isUserDefined = globals::parsing; \
  LEAVE("[AST]"); \
}





// use to get ANSI C
#undef kc_strcasecmp
#define kc_strcasecmp strcmp

%}
