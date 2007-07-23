/*****************************************************************************\
 * Copyright 2006, 2007 Peter Massuthe, Dennis Reinert, Daniela Weinberg     *
 *                      Niels Lohmann, Christian Gierds                      *
 *                                                                           *
 * This file is part of PNML2oWFN.                                           *
 *                                                                           *
 * PNML2oWFN is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or (at your    *
 * option) any later version.                                                *
 *                                                                           *
 * PNML2oWFN is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for  *
 * more details.                                                             *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with PNML2oWFN; if not, write to the Free Software Foundation, Inc., 51   *
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.                      *
\****************************************************************************/

%{
/*!
 * \file syntax_pnml.cc
 *
 * \brief EPNML 1.1 grammar (implementation)
 *
 * This file defines and implements the grammar of EPNML 1.1 using standard 
 * BNF-rules to describe the originally XML-based syntax as it is specified in
 * the EPNML 1.1 specification. All terminals are passed from the lexer
 * (implemented in \ref pnml-lexic.cc).
 * 
 * \author  
 *          - responsible: Dennis Reinert <reinert@informatik.hu-berlin.de>
 *          - last changes of: \$Author: znamirow $
 *          
 * \date 
 *          - created: 2006/09/10
 *          - last changed: \$Date: 2007/07/23 14:01:51 $
 * 
 * \note    This file is part of the tool PNML2oWFN and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 * \note    This file was created using GNU Bison reading file pnml-syntax.yy.
 *          See http://www.gnu.org/software/bison/bison.html for details
 *
 * \version \$Revision: 1.2 $
 * 
 */
%}

%{	
/*!
 * \file syntax_pnml.h
 * \brief EPNML 1.1 grammar (interface)
 *
 * See \ref syntax_pnml.cc for more information.
 */
%}

%{

// options for Bison
#define YYDEBUG 1
#define YYERROR_VERBOSE 1  // for verbose error messages

// to avoid the message "parser stack overflow"
#define YYMAXDEPTH 1000000
#define YYINITDEPTH 10000


#include <stdio.h>
#include "globals.h"
#include "petrinet.h"
#include "helpers.h"
#include "ast-config.h"
#include "debug.h"
#include "ast-details.h"
#include <limits.h>
#include <string>
using namespace std;
using namespace PNapi;

extern PetriNet PN;

/******************************************************************************
 * External variables
 *****************************************************************************/

// from flex
extern int frontend_pnml_lex();
extern int frontend_pnml_lineno;
extern char* frontend_pnml_text;

extern int frontend_pnml_error(const char *);

string id = "";
communication_type type = INTERNAL;
string source = "";
string target = "";
bool isMarked = false;
Place* p = NULL;

%}

/*---------------------------------------------------------------------------*/
 /* more bison options */
%name-prefix="frontend_pnml_"
// Bison generates a list of all used tokens in file "syntax_pnml.h" (for flex)
%token_table
%defines
%yacc


/* data type definition for return value of flex or bison action */
%union {
    kc::casestring yt_casestring;
}

/* the terminal symbols (tokens) */
%token X_OPEN X_SLASH X_CLOSE X_NEXT X_EQUALS QUOTE
%token P_NET P_PLACE P_GRAPHICS P_NAME P_DESCRIPTION P_TRANSITION P_PAGE
%token P_POSITION P_TEXT P_INITIALMARKING P_DIMENSION P_PNML
%token P_ARC P_INSCRIPTION P_OFFSET P_REFERENCEPLACE P_TYPE P_TRANSFORMATION
%token P_TOOLSPECIFIC
%token <yt_casestring> X_NAME
%token <yt_casestring> X_STRING
%token <yt_casestring> X_TEXT

%type <yt_casestring> tText

/* the start symbol of the grammar */
%start tPnml

/*----------------------- Grammar rules -------------------------------------*/

%%

/******************************************************************************
  PNML
******************************************************************************/

tPnml: 	
		X_OPEN P_PNML X_NEXT 
		 tNet 		
		X_NEXT X_SLASH P_PNML X_CLOSE
;

/******************************************************************************
  NET
******************************************************************************/

tNet:
  P_NET attributes X_NEXT netSubElement_List X_SLASH P_NET
| P_NET attributes X_SLASH
;

/*---------------------------------------------------------------------------*/

netSubElement_List:
  netSubElement X_NEXT
| netSubElement X_NEXT netSubElement_List
;

netSubElement:
  tTransition	
| tPlace  		
| tArc  		
| tPage  		
| tGraphics 	
| tName  		
| tDescription	
| tExtension
;

/*---------------------------------------------------------------------------*/

/******************************************************************************
  TRANSITION
******************************************************************************/

tTransition:
  P_TRANSITION attributes	X_NEXT transitionSubElement_List X_SLASH P_TRANSITION
    {
    	PN.newTransition(id);
    }
| P_TRANSITION attributes X_SLASH
    {
    	PN.newTransition(id);
    }
;

/*---------------------------------------------------------------------------*/

transitionSubElement_List:
  transitionSubElement X_NEXT
| transitionSubElement X_NEXT transitionSubElement_List
;

transitionSubElement:
  tGraphics  		
| tName  			  
| tDescription  	
| tTransformation	
| tType				

/*---------------------------------------------------------------------------*/

/******************************************************************************
  PAGE
******************************************************************************/

tPage:
  P_PAGE attributes X_NEXT pageSubElement_List X_SLASH P_PAGE
| P_PAGE attributes X_SLASH
;

/*---------------------------------------------------------------------------*/

pageSubElement_List:
  pageSubElement X_NEXT
| pageSubElement X_NEXT pageSubElement_List
;

pageSubElement:
  netSubElement_List
| tType					
| tReferenceplace		

/*---------------------------------------------------------------------------*/

/******************************************************************************
  REFERENCEPLACE
******************************************************************************/

tReferenceplace:
  P_REFERENCEPLACE attributes X_NEXT referenceplaceSubElement_List { trace(TRACE_VERY_DEBUG, "???"); } X_SLASH P_REFERENCEPLACE
| P_REFERENCEPLACE attributes X_SLASH
;

/*---------------------------------------------------------------------------*/

referenceplaceSubElement_List:
  referenceplaceSubElement X_NEXT
| referenceplaceSubElement X_NEXT referenceplaceSubElement_List
;

referenceplaceSubElement:
  tGraphics  	
| tName  			
| tDescription  

/*---------------------------------------------------------------------------*/


/******************************************************************************
  PLACE
******************************************************************************/

tPlace:
  P_PLACE attributes
    {
      type = INTERNAL;
    }
  X_NEXT placeSubElement_List X_SLASH P_PLACE
    {
   	  p = PN.newPlace(id,type);
    	if (isMarked == true)
    	{
    	  p->mark();
    	  isMarked = false;
      }   
    }
| P_PLACE attributes X_SLASH
    {
    	p = PN.newPlace(id,INTERNAL);
    	if (isMarked == true)
    	{
    	  p->mark();
    	  isMarked = false;
      }   
    }
;

/*---------------------------------------------------------------------------*/

placeSubElement_List:
  placeSubElement X_NEXT
| placeSubElement X_NEXT placeSubElement_List
;

placeSubElement:
  tGraphics  		
| tName  			
| tDescription  	
| tInitialmarking	
| tType

/*---------------------------------------------------------------------------*/

/******************************************************************************
  TRANSFORMATION
******************************************************************************/

tTransformation:
  P_TRANSFORMATION X_NEXT tText X_NEXT X_SLASH P_TRANSFORMATION
;

/******************************************************************************
  INITIALMARKING
******************************************************************************/

tInitialmarking:
  P_INITIALMARKING X_NEXT tText X_NEXT X_SLASH P_INITIALMARKING
    {
    	isMarked = 1;
    }
;

/******************************************************************************
  TYPE
******************************************************************************/

tType:
  P_TYPE X_NEXT tText X_NEXT X_SLASH P_TYPE
    {
      if((strip_namespace($3->name)) == ">input<")
      {
        type = IN;
      } 
      else if ((strip_namespace($3->name)) == ">output<")
      {
        type = OUT;
      }
      else if ((strip_namespace($3->name)) == ">inout<")
      {
        type = INOUT;
      }
      else
      {
        type = INTERNAL;
      }
    }
;

/******************************************************************************
  TEXT
******************************************************************************/

tText:
  P_TEXT X_TEXT X_SLASH P_TEXT
    {
    	$$ = $2;
    }
;

/******************************************************************************
  ARC
******************************************************************************/

tArc:
  P_ARC attributes
  	{
		PNapi::Node* sourceNode;
		PNapi::Node* targetNode;
	
		if(PN.findPlace(source) != NULL) {
			sourceNode = PN.findPlace(source);
		}
		else if(PN.findTransition(source) != NULL) {
			sourceNode = PN.findTransition(source);
		}

		if(PN.findPlace(target) != NULL) {
			targetNode = PN.findPlace(target);
		}
		else if(PN.findTransition(target) != NULL) {
			targetNode = PN.findTransition(target);
		}
	
		PN.newArc(sourceNode, targetNode);    	
	}
    X_NEXT arcSubElement_List X_SLASH P_ARC
| P_ARC attributes X_SLASH
  	{
		PNapi::Node* sourceNode;
		PNapi::Node* targetNode;
	
		if(PN.findPlace(source) != NULL) {
			sourceNode = PN.findPlace(source);
		}
		else if(PN.findTransition(source) != NULL) {
			sourceNode = PN.findTransition(source);
		}

		if(PN.findPlace(target) != NULL) {
			targetNode = PN.findPlace(target);
		}
		else if(PN.findTransition(target) != NULL) {
			targetNode = PN.findTransition(target);
		}
	
		PN.newArc(sourceNode, targetNode);    	
	}
;

/*---------------------------------------------------------------------------*/

arcSubElement_List:
  arcSubElement X_NEXT
| arcSubElement X_NEXT arcSubElement_List
;

arcSubElement:
  tGraphics  		
| tName			  	  
| tDescription  
| tInscription	
| tType				
;

/*---------------------------------------------------------------------------*/

/******************************************************************************
  NAME
******************************************************************************/

tName:
  P_NAME X_NEXT ndi_SubElement_List X_SLASH P_NAME
;

/******************************************************************************
  DESCRIPTION
******************************************************************************/

tDescription:
  P_DESCRIPTION X_NEXT ndi_SubElement_List X_SLASH P_DESCRIPTION
;


/******************************************************************************
  INSCRIPTION
******************************************************************************/

tInscription:
  P_INSCRIPTION X_NEXT ndi_SubElement_List X_SLASH P_INSCRIPTION
;

/*---------------------------------------------------------------------------*/
/* Name, Description and Inscription have the same subelements */

ndi_SubElement_List:
  ndi_SubElement X_NEXT
| ndi_SubElement X_NEXT ndi_SubElement_List
;

ndi_SubElement:
  tGraphics 
| tText			  

/*---------------------------------------------------------------------------*/

/******************************************************************************
  GRAPHICS
******************************************************************************/

tGraphics:
  P_GRAPHICS X_NEXT graphicsSubElements X_SLASH P_GRAPHICS
;

/*---------------------------------------------------------------------------*/

graphicsSubElements:

  tPosition
| tOffset
| tDimension

/*---------------------------------------------------------------------------*/

/******************************************************************************
  POSITION
******************************************************************************/

tPosition:
  P_POSITION attributes X_SLASH X_NEXT graphicsSubElements
| P_POSITION attributes X_SLASH X_NEXT
;

/******************************************************************************
  OFFSET
******************************************************************************/

tOffset:
  P_OFFSET attributes X_SLASH X_NEXT graphicsSubElements
| P_OFFSET attributes X_SLASH X_NEXT
;

/******************************************************************************
  DIMENSION
******************************************************************************/

tDimension:
  P_DIMENSION attributes X_SLASH X_NEXT graphicsSubElements
| P_DIMENSION attributes X_SLASH X_NEXT
;

/*---------------------------------------------------------------------------*/

attributes:

| X_NAME X_EQUALS X_STRING attributes
  { 	
      if((strip_namespace($1->name)) == "id")
      {
        id = (strip_namespace($3->name)).substr(1,(strip_namespace($3->name)).length()-2);
      } 
      else if ((strip_namespace($1->name)) == "source")
      {
        source = (strip_namespace($3->name)).substr(1,(strip_namespace($3->name)).length()-2);
      }
      else if ((strip_namespace($1->name)) == "target")
      {
        target = (strip_namespace($3->name)).substr(1,(strip_namespace($3->name)).length()-2);
      }
      else
      {
      }
  }
;

/******************************************************************************
  EXTENSIONS OF PNML within NET
******************************************************************************/

tExtension:
  tToolSpecific
;

tToolSpecific:
  P_TOOLSPECIFIC attributes X_NEXT toolSpecificSubElement_List X_SLASH P_TOOLSPECIFIC
| P_TOOLSPECIFIC attributes X_SLASH
;

/*---------------------------------------------------------------------------*/

toolSpecificSubElement_List:
  X_NAME attributesIgnore X_NEXT toolSpecificSubElement_List X_SLASH X_NAME X_NEXT toolSpecificSubElement_List
| X_NAME attributesIgnore X_SLASH X_NEXT toolSpecificSubElement_List
|	/* empty */
| X_NAME attributesIgnore X_NEXT P_TEXT X_TEXT X_SLASH P_TEXT X_NEXT X_SLASH X_NAME X_NEXT toolSpecificSubElement_List
  { 
    if (PN.findPlace((strip_namespace($5->name)).substr(1,(strip_namespace($5->name)).length()-3)) != NULL)
      PN.findPlace((strip_namespace($5->name)).substr(1,(strip_namespace($5->name)).length()-3))->isFinal = true;
  }
;

attributesIgnore:
  /* empty */
| X_NAME X_EQUALS X_STRING attributesIgnore
;

