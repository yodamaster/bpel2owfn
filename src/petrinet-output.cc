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
\*****************************************************************************/

/*!
 * \file    petrinet-output.cc
 *
 * \brief   output Functions for Petri nets (implementation)
 * 
 * \author  responsible: Niels Lohmann <nlohmann@informatik.hu-berlin.de>,
 *          last changes of: \$Author: nlohmann $
 *
 * \since   created: 2006-03-16
 *
 * \date    \$Date: 2006/11/22 15:29:51 $
 *
 * \note    This file is part of the tool BPEL2oWFN and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 * \version \$Revision: 1.51 $
 *
 * \ingroup petrinet
 */





/******************************************************************************
 * Headers
 *****************************************************************************/

#include "petrinet.h"		// to define member functions
#include "debug.h"		// debugging help
#include "helpers.h"		// helper functions
#include "bpel2owfn.h"
#include "options.h"
#include "ast-details.h"

#ifndef USING_BPEL2OWFN
#include "bpel2owfn_wrapper.h"
#endif



/******************************************************************************
 * Functions to print information of the net and its nodes
 *****************************************************************************/

/*!
 * \return the name of the node type
 */
string Node::nodeTypeName()
{
  return "";
}





/*!
 * \return the name of the node type
 */
string Place::nodeTypeName()
{
  return "place";
}





/*!
 * \return the name of the node type
 */
string Transition::nodeTypeName()
{
  return "transition";
}





/*!
 * dummy-implementation of virtual function
 */
string Node::nodeShortName()
{
  return "";
}





/*!
 * \return the (nice) name of the node for DOT output
 */
string Node::nodeName()
{
  string result = history[0];

  if (type == INTERNAL)
    result = result.substr(result.find_last_of(".")+1,result.length());

  result = prefix + result;
  return result;
}






/*!
 * \return the short name of the place, e.g. for LoLA output
 */
string Place::nodeShortName()
{
  if (type == INTERNAL)
    return ("p" + toString(id));
  else
    return history[0];
}





/*!
 * \return the short name of the transition, e.g. for LoLA output
 */
string Transition::nodeShortName()
{
  return ("t" + toString(id));
}





/*!
 * \return string containing information about the net
 */
string PetriNet::information()
{
  string result = "|P|=" + toString(P.size() + P_in.size() + P_out.size());
  result += ", |P_in|= " + toString(P_in.size());
  result += ", |P_out|= " + toString(P_out.size());
  result += ", |T|=" + toString(T.size());
  result += ", |F|=" + toString(F.size());
  return result;
}





/*!
 * Prints information about the generated Petri net. In particular, for each
 * place and transition all roles of the history are printed to understand
 * the net and maybe LoLA's witness pathes later.
 *
 * \todo put this to the nodes
 */
void PetriNet::printInformation()
{
  // the places
  (*output) << "PLACES:\nID\tTYPE\t\tROLES\n";

  // the internal places
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
  {
    (*output) << (*p)->nodeShortName() << "\tinternal";

    for (vector<string>::iterator role = (*p)->history.begin(); role != (*p)->history.end(); role++)
      if (role == (*p)->history.begin())
	(*output) << "\t" + (*p)->prefix + *role + "\n";
      else
	(*output) << "\t\t\t" + (*p)->prefix + *role + "\n";
  }

  // the input places
  for (set<Place *>::iterator p = P_in.begin(); p != P_in.end(); p++)
  {
    (*output) << (*p)->nodeShortName() << "\tinput   ";

    for (vector<string>::iterator role = (*p)->history.begin(); role != (*p)->history.end(); role++)
      if (role == (*p)->history.begin())
	(*output) << "\t" + (*p)->prefix + *role + "\n";
      else
	(*output) << "\t\t\t" + (*p)->prefix + *role + "\n";
  }

  // the output places
  for (set<Place *>::iterator p = P_out.begin(); p != P_out.end(); p++)
  {
    (*output) << (*p)->nodeShortName() << "\toutput  ";

    for (vector<string>::iterator role = (*p)->history.begin(); role != (*p)->history.end(); role++)
      if (role == (*p)->history.begin())
	(*output) << "\t" + (*p)->prefix + *role + "\n";
      else
	(*output) << "\t\t\t" + (*p)->prefix + *role + "\n";
  }

  // the transitions
  (*output) << "\nTRANSITIONS:\n";
  (*output) << "ID\tROLES\n";

  for (set<Transition *>::iterator t = T.begin(); t != T.end(); t++)
  {
    (*output) << (*t)->nodeShortName() + "\t";

    for (vector<string>::iterator role = (*t)->history.begin(); role != (*t)->history.end(); role++)
      if (role == (*t)->history.begin())
	(*output) << (*t)->prefix + *role + "\n";
      else
	(*output) << "\t" + (*t)->prefix + *role + "\n";
  }
}





/******************************************************************************
 * DOT output of the net
 *****************************************************************************/


/*!
 * DOT-output of the arc.
*/
string Arc::dotOut()
{
  string result = " ";
  if (source->nodeType == PLACE)
    result += "p" + toString(source->id) + " -> t" + toString(target->id);
  else
    result += "t" + toString(source->id) + " -> p" + toString(target->id);
  result += ";\n";

  return result;
}





/*!
 * DOT-output of the transition. Transitions are colored corresponding to their
 * initial role.
*/
string Transition::dotOut()
{
  string result;
#ifdef USING_BPEL2OWFN
  result += " t" + toString(id) + "\t[label=\"" + nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
  result += " t" + toString(id) + "\t[label=\"" + nodeName();
#endif
 
  if (parameters[P_COMMUNICATIONONLY])
  {
    result += "\\n";
    result += nodeName();
  }

  result += "\"";

  if (type == IN)
    result += " style=filled fillcolor=orange";
  if (type == OUT)
    result += " style=filled fillcolor=yellow";
  if (type == INOUT)
    result += " style=filled fillcolor=gold";
  
  result += "];\n";
  return result;
}





/*!
 * DOT-output of the place. Places are colored corresponding to their initial
 * role.
*/
string Place::dotOut()
{
  string result;
#ifdef USING_BPEL2OWFN
  result += " p" + toString(id) + "\t[label=\"" + nodeShortName() + "\"" + " URL=\"http://www.google.de\"";
#endif
#ifndef USING_BPEL2OWFN
  result += " p" + toString(id) + "\t[label=\"" + nodeName() + "\"" + " URL=\"http://www.google.de\"";
#endif

  if (type == IN)
    result += " style=filled fillcolor=orange shape=ellipse";
  if (type == OUT)
    result += " style=filled fillcolor=yellow shape=ellipse";
  if (type == INOUT)
    result += " style=filled fillcolor=gold shape=ellipse";

  if (historyContains("1.internal.initial")
	   || historyContains("1.internal.final"))
    result += " style=filled fillcolor=gray";
  else if (firstMemberIs("!link."))
    result += " style=filled fillcolor=red";
  else if (firstMemberIs("link."))
    result += " style=filled fillcolor=green";
  else if (firstMemberIs("variable."))
    result += " style=filled fillcolor=cyan";
  else if (historyContains("1.internal.clock"))
    result += " style=filled fillcolor=seagreen";

  result += "];\n";
  return result;
}





/*!
 * Creates a DOT output(see http://www.graphviz.org) of the Petri net. It uses
 * the digraph-statement and adds labels to transitions, places and arcs if
 * neccessary. It also distinguishes the three arc types of #arc_type.
 */
void PetriNet::dotOut()
{
  trace(TRACE_DEBUG, "[PN]\tCreating DOT-output.\n");

  (*output) << "digraph N {" << endl;
  (*output) << " graph [fontname=\"Helvetica\", label=\"";

  if (parameters[P_SIMPLIFY])
    (*output) << "structural simplified ";

  (*output) << "Petri net generated from " << filename << "\"];" << endl;
  (*output) <<
    " node [fontname=\"Helvetica\" fontsize=10 fixedsize];" << endl;
  (*output) << " edge [fontname=\"Helvetica\" fontsize=10];" <<
    endl << endl;

  // list the places
  (*output) << endl << " node [shape=circle];" << endl;
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
    (*output) << (*p)->dotOut();

  if (!hasNoInterface)
  {
    for (set<Place *>::iterator p = P_in.begin(); p != P_in.end(); p++)
      (*output) << (*p)->dotOut();
    for (set<Place *>::iterator p = P_out.begin(); p != P_out.end(); p++)
      (*output) << (*p)->dotOut();
  }

  // list the transitions
  (*output) << endl << " node [shape=box regular=true];" << endl;
  for (set<Transition *>::iterator t = T.begin(); t != T.end(); t++)
    (*output) << (*t)->dotOut();

  // list the arcs
  for (set<Arc *>::iterator f = F.begin(); f != F.end(); f++)
    (*output) << (*f)->dotOut();

  (*output) << "}" << endl;
}





/******************************************************************************
 * Petri net file formats
 *****************************************************************************/

/*!
 * Outputs the net in PNML (Petri Net Markup Language).
 */
void PetriNet::pnmlOut()
{
  trace(TRACE_DEBUG, "[PN]\tCreating PNML-output.\n");

  // remove interface since we do not create an open workflow net
  removeInterface();

  (*output) << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl << endl;
  (*output) << "<!--" << endl;
  (*output) << "  Petri net created by " << PACKAGE_STRING << " reading file " << filename << "." << endl;
  (*output) << "  See http://www.informatik.hu-berlin.de/top/tools4bpel/bpel2owfn" << endl;
  (*output) << "  for more details." << endl;
  (*output) << "-->" << endl << endl;

  (*output) << "<pnml>" << endl;
  (*output) << "  <net id=\"bpel-net\" type=\"\">" << endl << endl;

  // places(only internal)
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
  {
#ifdef USING_BPEL2OWFN
    (*output) << "    <place id=\"" << (*p)->nodeShortName() << "\">" << endl;
#endif
#ifndef USING_BPEL2OWFN
    (*output) << "    <place id=\"" << (*p)->nodeName() << "\">" << endl;
#endif
    (*output) << "      <name>" << endl;
    (*output) << "        <text>" << (*p)->history[0] << "</text>" << endl;
    (*output) << "      </name>" << endl;
    if ((*p)->tokens > 0)
    {
      (*output) << "      <initialMarking>" << endl;
      (*output) << "        <text>" << (*p)->tokens << "</text>" << endl;
      (*output) << "      </initialMarking>" << endl;
    }
    (*output) << "    </place>" << endl << endl;
  }

  // transitions
  for (set<Transition *>::iterator t = T.begin(); t != T.end(); t++)
  {
#ifdef USING_BPEL2OWFN
    (*output) << "    <transition id=\"" << (*t)->nodeShortName() << "\">" << endl;
#endif
#ifndef USING_BPEL2OWFN
    (*output) << "    <transition id=\"" << (*t)->nodeName() << "\">" << endl;
#endif
    (*output) << "      <name>" << endl;
    (*output) << "        <text>" << (*t)->history[0] << "</text>" << endl;
    (*output) << "      </name>" << endl;
    (*output) << "    </transition>" << endl << endl;
  }
  (*output) << endl;

  // arcs
  int arcNumber = 1;
  for (set<Arc *>::iterator f = F.begin(); f != F.end(); f++, arcNumber++)
  {
    (*output) << "    <arc id=\"a" << arcNumber << "\" ";
#ifdef USING_BPEL2OWFN
    (*output) << "source=\"" << (*f)->source->nodeShortName() << "\" ";
    (*output) << "target=\"" << (*f)->target->nodeShortName() << "\" />" << endl;
#endif
#ifndef USING_BPEL2OWFN
    (*output) << "source=\"" << (*f)->source->nodeName() << "\" ";
    (*output) << "target=\"" << (*f)->target->nodeName() << "\" />" << endl;
#endif
  }
  (*output) << endl;
  (*output) << "  </net>" << endl;
  (*output) << "</pnml>" << endl;
}





/*!
 * Outputs the net in low-level PEP notation.
 */
void PetriNet::pepOut()
{
  trace(TRACE_DEBUG, "[PN]\tCreating PEP-output.\n");

  // remove interface since we do not create an open workflow net
  removeInterface();

  // header
  (*output) << "PEP" << endl << "PTNet" << endl << "FORMAT_N" << endl;

  // places(only internal)
  (*output) << "PL" << endl;
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
  {
    (*output) << (*p)->id << "\"" << (*p)->nodeShortName() << "\"80@40";
    if ((*p)->tokens > 0)
      (*output) << "M" << (*p)->tokens;
    (*output) << "k1" << endl;
  }

  // transitions
  (*output) << "TR" << endl;
  for (set<Transition *>::iterator t = T.begin(); t != T.end(); t++)
    (*output) << (*t)->id << "\"" << (*t)->nodeShortName() << "\"80@40" << endl;

  // arcs from transitions to places
  (*output) << "TP" << endl;
  for (set<Arc *>::iterator f = F.begin(); f != F.end(); f++)
    if (((*f)->source->nodeType) == TRANSITION)
      (*output) << (*f)->source->id << "<" << (*f)->target->id << endl;

  // arcs from places to transitions
  (*output) << "PT" << endl;
  for (set<Arc *>::iterator f = F.begin(); f != F.end(); f++)
    if (((*f)->source->nodeType) == PLACE)
      (*output) << (*f)->source->id << ">" << (*f)->target->id << endl;
}





/*!
 * Outputs the net in APNN (Abstract Petri Net Notation).
 */
void PetriNet::apnnOut()
{
  trace(TRACE_DEBUG, "[PN]\tCreating APNN-output.\n");

  // remove interface since we do not create an open workflow net
  removeInterface();

  (*output) << "\\beginnet{" << filename << "}" << endl << endl;

  // places(only internal)
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
  {
    (*output) << "  \\place{" << (*p)->nodeShortName() << "}{";
    if ((*p)->tokens > 0)
      (*output) << "\\init{" << (*p)->tokens << "}";
    (*output) << "}" << endl;
  }
  (*output) << endl;

  // transitions
  for (set<Transition *>::iterator t = T.begin(); t != T.end(); t++)
  {
    (*output) << "  \\transition{" << (*t)->nodeShortName() << "}{}" << endl;
  }
  (*output) << endl;

  // arcs
  int arcNumber = 1;
  for (set<Arc *>::iterator f = F.begin(); f != F.end(); f++, arcNumber++)
  {
    (*output) << "  \\arc{a" << arcNumber << "}{ ";
    (*output) << "\\from{" << (*f)->source->nodeShortName() << "} ";
    (*output) << "\\to{" << (*f)->target->nodeShortName() << "} }" << endl;
  }
  (*output) << endl;

  (*output) << "\\endnet" << endl;
}





/*!
 * Outputs the net in LoLA-format.
 */
void PetriNet::lolaOut()
{
  trace(TRACE_DEBUG, "[PN]\tCreating LoLA-output.\n");

  // remove interface since we do not create an open workflow net
  removeInterface();

  (*output) << "{ Petri net created by " << PACKAGE_STRING << " reading " << filename << " }" << endl << endl;

  
  // places (only internal)
  (*output) << "PLACE" << endl;
  unsigned int count = 1;
  for (set<Place *>::iterator p = P.begin(); p != P.end(); count++, p++)
  {
    (*output) << "  " << (*p)->nodeShortName();
    
    if (count < P.size())
      (*output) << "," << endl;
  }
  (*output) << endl << ";" << endl << endl << endl;


  // initial marking
  (*output) << "MARKING" << endl;
  count = 1;
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
  {
    if ((*p)->tokens > 0)
    {
      if (count++ != 1)
	(*output) << "," << endl;
      
      (*output) << "  " << (*p)->nodeShortName() << ":\t" << (*p)->tokens;
    }
  }
  (*output) << endl << ";" << endl << endl << endl;


  // transitions
  for (set<Transition *>::iterator t = T.begin(); t != T.end(); t++)
  {
    (*output) << "TRANSITION " << (*t)->nodeShortName();
/*   
    switch ((*t)->type)
    {
      case (INTERNAL): break;
      case (OUT): (*output) << "_o"; break;
      case (IN): (*output) << "_i"; break;
      case (INOUT): (*output) << "_io"; break;
    }
*/
    (*output) << endl;
    set<Node *> consume = preset(*t);
    set<Node *> produce = postset(*t);
    
    (*output) << "CONSUME" << endl;
    count = 1;
    for (set<Node *>::iterator pre = consume.begin(); pre != consume.end(); count++, pre++)
    {
      (*output) << "  " << (*pre)->nodeShortName() << ":\t1";
      
      if (count < consume.size())
	(*output) << "," << endl;
    }
    (*output) << ";" << endl;
    
    (*output) << "PRODUCE" << endl;
    count = 1;
    for (set<Node *>::iterator post = produce.begin(); post != produce.end(); count++, post++)
    {
      (*output) << "  " << (*post)->nodeShortName() << ":\t1";
      
      if (count < produce.size())
	(*output) << "," << endl;
    }
    
    (*output) << ";" << endl << endl;
  }  
  (*output) << "{ END OF FILE }" << endl;
}





/******************************************************************************
 * oWFN file format
 *****************************************************************************/

/*!
 * Outputs the net in oWFN-format.
 */
void PetriNet::owfnOut()
{
  extern map<unsigned int, ASTE*> ASTEmap;
  extern string invocation;

  trace(TRACE_DEBUG, "[PN]\tCreating oWFN-output.\n");

  (*output) << "{" << endl;
  (*output) << "  generated by: " << PACKAGE_STRING << " (see http://www.informatik.hu-berlin.de/top/tools4bpel/" << PACKAGE << ")" << endl;
#ifdef USING_BPEL2OWFN
  (*output) << "  input file:   `" << filename << "' (process `" << ASTEmap[1]->attributes["name"] << "')" << endl;
#endif
#ifndef USING_BPEL2OWFN
  (*output) << "  input file:   `" << filename << "'" << endl;
#endif
  (*output) << "  invocation:   `" << invocation << "'" << endl;
  (*output) << "  net size:     " << information() << endl;
  (*output) << "}" << endl << endl;

  // places
  (*output) << "PLACE" << endl;

  // internal places
  (*output) << "  INTERNAL" << endl;
  (*output) << "    ";

  unsigned int count = 1;
  for (set<Place *>::iterator p = P.begin(); p != P.end(); count++, p++)
  {
#ifdef USING_BPEL2OWFN
    (*output) << (*p)->nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
    (*output) << (*p)->nodeName();
#endif
    if (count < P.size())
      (*output) << ", ";
  }
  (*output) << ";" << endl << endl;


  // input places
  (*output) << "  INPUT" << endl;
  count = 1;
  for (set<Place *>::iterator p = P_in.begin(); p != P_in.end(); count++, p++)
  {
#ifdef USING_BPEL2OWFN
    (*output) << "    " << (*p)->nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
    (*output) << "    " << (*p)->nodeName();
#endif

    if (count < P_in.size())
      (*output) << "," << endl;
  }
  (*output) << ";" << endl << endl;


  // output places
  (*output) << "  OUTPUT" << endl;
  count = 1;
  for (set<Place *>::iterator p = P_out.begin(); p != P_out.end(); count++, p++)
  {
#ifdef USING_BPEL2OWFN
    (*output) << "    " << (*p)->nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
    (*output) << "    " << (*p)->nodeName();
#endif
    
    if (count < P_out.size())
      (*output) << "," << endl;
  }
  (*output) << ";" << endl << endl << endl;
  

  // initial marking
  (*output) << "INITIALMARKING" << endl;
  count = 1;
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
  {
    if ((*p)->tokens > 0)
    {
      if (count++ != 1)
	(*output) << "," << endl;

#ifdef USING_BPEL2OWFN
      (*output) << " " << (*p)->nodeShortName() << ":\t" << (*p)->tokens;
#endif
#ifndef USING_BPEL2OWFN
      (*output) << " " << (*p)->nodeName() << ":\t" << (*p)->tokens;
#endif

      if ((*p)->historyContains("1.internal.initial"))
	(*output) << " {initial place}";
      if ((*p)->historyContains("1.internal.final"))
	(*output) << " {final place}";
    }
  }
  (*output) << ";" << endl << endl;  


  // final marking
  (*output) << "FINALMARKING" << endl;
  count = 1;
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
  {
    if ((*p)->historyContains((*p)->prefix + "1.internal.final") )
    {
      if (count++ != 1)
	(*output) << ",";

#ifdef USING_BPEL2OWFN
      (*output) << "  " << (*p)->nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
      (*output) << "  " << (*p)->nodeName();
#endif

      if ((*p)->historyContains("1.internal.initial"))
      	(*output) << " {initial place}";
      if ((*p)->historyContains("1.internal.final"))
  	(*output) << " {final place}";
    }
  }
  (*output) << ";" << endl << endl << endl;


  // transitions
  for (set<Transition *>::iterator t = T.begin(); t != T.end(); t++)
  {
#ifdef USING_BPEL2OWFN
    (*output) << "TRANSITION " << (*t)->nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
    (*output) << "TRANSITION " << (*t)->nodeName();
#endif
    switch( (*t)->type )
    {
      case (INTERNAL):	(*output) << endl; break;
      case(IN):		(*output) << " { input }" << endl; break;
      case(OUT):	(*output) << " { output }" << endl; break;
      case(INOUT):	(*output) << " { input/output }" << endl; break;
    }

    set<Node *> consume = preset(*t);
    set<Node *> produce = postset(*t);
    
    (*output) << "  CONSUME ";
    count = 1;
    for (set<Node *>::iterator pre = consume.begin(); pre != consume.end(); count++, pre++)
    {
#ifdef USING_BPEL2OWFN
      (*output) << (*pre)->nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
      (*output) << (*pre)->nodeName();
#endif
      if (arc_weight(*pre, *t) != 1)
	(*output) << ":" << arc_weight(*pre, *t);

      if (count < consume.size())
	(*output) << ", ";
    }
    (*output) << ";" << endl;
    
    (*output) << "  PRODUCE ";

    count = 1;
    for (set<Node *>::iterator post = produce.begin(); post != produce.end(); count++, post++)
    {
#ifdef USING_BPEL2OWFN
      (*output) << (*post)->nodeShortName();
#endif
#ifndef USING_BPEL2OWFN
      (*output) << (*post)->nodeName();
#endif
      if (arc_weight(*t, *post) != 1)
	(*output) << ":" << arc_weight(*t, *post);
      
      if (count < produce.size())
	(*output) << ", ";
    }
    
    (*output) << ";" << endl << endl;
  }  
  (*output) << endl << "{ END OF FILE `" << output_filename << ".owfn' }" << endl;
}
