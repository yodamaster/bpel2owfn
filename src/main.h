/*****************************************************************************\
 * Copyright 2005, 2006 Niels Lohmann, Christian Gierds, Dennis Reinert      *
 *                                                                           *
 * This file is part of BPEL2oWFN.                                           *
 *                                                                           *
 * BPEL2oWFN is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or (at your    *
 * option) any later version.                                                *
 *                                                                           *
 * BPEL2oWFN is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for  *
 * more details.                                                             *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with BPEL2oWFN; if not, write to the Free Software Foundation, Inc., 51   *
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.                      *
\****************************************************************************/

/*!
 * \file main.h
 *
 * \brief Main file header
 *
 * \author  
 *          - responsible: Christian Gierds <gierds@informatik.hu-berlin.de>
 *          - last changes of: \$Author: gierds $
 *          
 * \date
 *          - created: 2005/11/09
 *          - last changed: \$Date: 2006/03/31 14:56:53 $
 * 
 * \note    This file is part of the tool BPEL2oWFN and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/forschung/projekte/tools4bpel
 *          for details.
 *
 * \version \$Revision: 1.19 $
 *          - 2005-11-09 (gierds) Initial version.
 *          - 2005-11-15 (gierds) Added Exception class.
 *          - 2005-11-16 (gierds) Use of error() and cleanup() as defined in helpers.cc
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>


#include "bpel-kc-k.h"          // generated by kimwitu++
#include "petrinet.h"           // Petri Net support
#include "cfg.h"		// Control Flow Graph
#include "debug.h"		// debugging help
#include "exception.h"		// exception handling
#include "check-symbols.h"	// scope handling and checking

/* defined by Bison */
extern int yyparse();
extern int yydebug;
extern kc::tProcess TheProcess;


/* defined by flex */
extern int yy_flex_debug; ///< if set to 1 the lexer trace is printed
extern FILE *yyin;        ///< pointer to the input file


// from check-symbols
extern SymbolManager symMan;


#endif

