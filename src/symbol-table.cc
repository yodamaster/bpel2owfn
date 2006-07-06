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
 * \file    symbol-table.cc
 *
 * \brief   This file implements the classes and functions defined
 *          in symbol-table.h.
 *
 * \author
 *          - responsible: Dennis Reinert <reinert@informatik.hu-berlin.de>
 *          - last changes of: \$Author: nlohmann $  
 *          
 * \date
 * 
 * \note    This file is part of the tool BPEL2oWFN and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universit&auml;t zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/forschung/projekte/tools4bpel
 *          for details.
 *
 * \todo
 *          - readAttribute() have to throw a error messages -> e.g. K-COPY doesn't have attributes 
 */ 

#include "symbol-table.h"
#include "bpel-attributes.h"
#include "bpel-syntax.h"
#include "debug.h"
#include "helpers.h"
#include "options.h"
#include <map>
#include <assert.h>

extern SymbolTable symTab;
extern int yylineno;

map< string, string > channelShortNames;

/********************************************
 * implementation of SymbolTable CLASS
 ********************************************/

/*!
 * constructor
 */
SymbolTable::SymbolTable()
{
  this->symTab.clear();
  this->entryKey = 0;
  this->horizontal = "";
  this->smallHorizontal = "";
  this->vertical = "";
}

/*!
 * destructor
 */
SymbolTable::~SymbolTable() {}

/*!
 * increase the entryKey of the SymbolTable 
 */
unsigned int SymbolTable::nextKey()
{
  this->entryKey++;
  return this->entryKey;
}

/*!
 * 
 */
kc::integer SymbolTable::nextId()
{
  return kc::mkinteger(nextKey());
}

/*!
 * 
 */
void SymbolTable::printSymbolTable()
{
  traceST("printing SymbolTable ... \n");
  
  horizontal      = "+--------------------------------------------+";
  smallHorizontal = "---------------------";
  vertical        = "| ";
  
  /// iterator
  std::map<unsigned int, SymbolTableEntry*>::iterator symTabIterator;  
  symTabIterator = symTab.begin();
	
  /// iteration about symbol table 
  while(symTabIterator != symTab.end())
  {  
  	SymbolTableEntry* currentEntry = lookup((*symTabIterator).first);
    if ( currentEntry != NULL)
    {    
      traceSTwp(horizontal); traceSTwp("\n");
    
      printSymbolTableEntry(currentEntry);
      printSTElement(currentEntry);

      switch(currentEntry->elementId)
      {
	case K_COMPENSATE:		{ } break;
	case K_COMPENSATIONHANDLER:	{ } break;
	case K_CORRELATIONSET:		{ } break;
	case K_EVENTHANDLERS:		{ } break;
	case K_FAULTHANDLERS:		{ } break;
	case K_INVOKE:			{ } break;
	case K_LINK:			{ } break;
	case K_PARTNER:			{ } break;
	case K_PARTNERLINK:		{ } break;
	case K_PROCESS:
	{
	  printSTEnvelope(currentEntry);
	} break;
	case K_RECEIVE:			{ } break;
	case K_REPLY:			{ } break;
	case K_SCOPE:
	{
	  printSTEnvelope(currentEntry);
	} break;
	case K_TERMINATE:		{ } break;
	case K_VARIABLE:		{ } break;
	case K_WAIT:			{ } break;
	/* all other */
	default :				{ } break;
      }
      traceSTwp(horizontal); traceSTwp("\n"); 
      traceSTwp("\n");        
    }
    ++symTabIterator;
  }
} 

/*!
 * 
 */
void SymbolTable::printSymbolTableEntry(SymbolTableEntry* entry)
{
  traceSTwp(vertical); traceSTwp(translateToElementName(entry->elementId)); traceSTwp("\n");	
  traceSTwp(vertical); traceSTwp(smallHorizontal + "<SymbolTableEntry>"); traceSTwp("\n");	  
  traceSTwp(vertical); traceSTwp("entryKey=" + intToString(entry->entryKey)); traceSTwp("\n");
  traceSTwp(vertical); traceSTwp("elementId="+ intToString(entry->elementId)); traceSTwp("\n");
}

/*!
 * 
 */
void SymbolTable::printSTElement(SymbolTableEntry* entry)
{
  ///
  STElement* e = dynamic_cast <STElement*> (entry);

  traceSTwp(vertical); traceSTwp(smallHorizontal + "<STElement>"); traceSTwp("\n");	  
  traceSTwp(vertical); traceSTwp("activityLocation="); traceSTwp("\n");
  traceSTwp(vertical); traceSTwp("line=" + intToString(e->line)); traceSTwp("\n");
  traceSTwp(vertical); traceSTwp("mapOfAttributes=<STAttribute>"); traceSTwp("\n");
  
  printSTAttribute(e);
}  

/*!
 * 
 */
void SymbolTable::printSTAttribute(STElement* entry) {

  /// reference to attribute map
  std::map<string, STAttribute*> &  mapOfAttributes = entry->mapOfAttributes;	

  /// iterator for the embedded map
  std::map<std::string, STAttribute*>::iterator mapOfAttributesIterator;  
  mapOfAttributesIterator = mapOfAttributes.begin();

  /// 
  STAttribute* a;

  unsigned int i=1;

  traceSTwp(vertical); traceSTwp(smallHorizontal + "<STAttribute>"); traceSTwp("\n");	  

  while(mapOfAttributesIterator != mapOfAttributes.end())
  {  
    a = (*mapOfAttributesIterator).second;
    traceSTwp(vertical); traceSTwp("attribute" + intToString(i)); traceSTwp("\n");
    traceSTwp(vertical +"\t"); traceSTwp("line=" + intToString(a->line)); traceSTwp("\n");
    traceSTwp(vertical +"\t"); traceSTwp("name=" + a->name); traceSTwp("\n");
    traceSTwp(vertical +"\t"); traceSTwp("type=" + a->type); traceSTwp("\n");
    traceSTwp(vertical +"\t"); traceSTwp("value=" + a->value); traceSTwp("\n");
    mapOfAttributesIterator++; ++i;
  }
}  

/*!
 * 
 */
void SymbolTable::printSTEnvelope(SymbolTableEntry* entry) {
  ///
  STEnvelope* e = dynamic_cast <STEnvelope*> (entry);

  traceSTwp(vertical); traceSTwp(smallHorizontal + "<STEnvelope>"); traceSTwp("\n");	  
  traceSTwp(vertical); traceSTwp("hasEnventHandler=");
  if((e->hasEventHandler == true) || (e->hasEventHandler == false)) {
  	if(e->hasEventHandler == true) {
  	  traceSTwp("true");	
  	}
  	else {
  	  traceSTwp("false");	  		
  	}
  }
  else {
  	  traceSTwp("undefined");
  }  	  
  traceSTwp("\n");
}

/*!
 * \param astId
 * \param closeTag 
 */
string SymbolTable::getInformation(kc::integer astId, bool closeTag)
{
  string result = "<";
  
  if (closeTag)
    result += "/";
  
  result += translateToElementName((lookup(astId))->getElementId());
  result += " id=\"" + intToString(astId->value) + "\">";
  
  return result;
}

/*!
 * retranslate the internal parse id of BPEL element to the real BPEL string
 * \param elementId BPEL element-id
 */
string SymbolTable::translateToElementName(unsigned int elementId)
{
  //traceST("translateToElementName:\n");
  switch(elementId)
  {
    case K_ASSIGN:		return "assign";
    case K_CASE:		return "case";
    case K_CATCH:		return "catch";
    case K_CATCHALL:		return "catchAll";
    case K_COPY:		return "copy";
    case K_COMPENSATE:		return "compensate";
    case K_COMPENSATIONHANDLER:	return "compensationHandler";    
    case K_CORRELATION:		return "correlation";
    case K_CORRELATIONSET: 	return "correlationSet";
    case K_EMPTY:		return "empty";
    case K_EVENTHANDLERS:	return "eventHandlers";
    case K_FAULTHANDLERS:	return "faultHandlers";            
    case K_FROM:	    	return "from";
    case K_FLOW:		return "flow";
    case K_INVOKE:		return "invoke";
    case K_LINK:		return "link";
    case K_ONALARM:		return "onAlarm";
    case K_ONMESSAGE:		return "onMessage";
    case K_OTHERWISE:		return "otherwise";
    case K_PARTNER:		return "partner";
    case K_PARTNERLINK:		return "partnerLink";
    case K_PICK:		return "pick";
    case K_PROCESS:		return "process";
    case K_RECEIVE:		return "receive";
    case K_REPLY:		return "reply";
    case K_SCOPE:		return "scope";
    case K_SEQUENCE:		return "sequence";
    case K_SOURCE:		return "source";
    case K_SWITCH:		return "switch";
    case K_TARGET:		return "target";
    case K_TERMINATE:		return "terminate";
    case K_THROW:		return "throw";
    case K_TO:			return "to";    
    case K_VARIABLE:		return "variable";
    case K_WAIT:		return "wait";
    case K_WHILE:		return "while";
    default:			return "unknown";
  }
} 


/*!
 * create a new special BPEL element-object and insert these into symbol table
 * \param elementId  BPEL element-id (e.g. K_PROCESS)
 */
unsigned int SymbolTable::insert(unsigned int elementId)
{
  traceST("insert(" + translateToElementName(elementId) + ")\n");
  traceST(intToString(this->entryKey) + "\n");
  traceST(intToString(yylineno) + "\n");
  switch(elementId)
  {
//    case K_CASE:                {symTab[this->nextKey()] = new STCaseBranch(elementId, this->entryKey);} break;
    case K_COMPENSATE:          {symTab[this->nextKey()] = new STCompensate(elementId, this->entryKey);} break;
    case K_COMPENSATIONHANDLER: {symTab[this->nextKey()] = new STCompensationHandler(elementId, this->entryKey);} break;
    case K_CORRELATIONSET:      {symTab[this->nextKey()] = new STCorrelationSet(elementId, this->entryKey);} break;
    case K_EVENTHANDLERS:       {symTab[this->nextKey()] = new STEventHandlers(elementId, this->entryKey);} break;
    case K_FAULTHANDLERS:       {symTab[this->nextKey()] = new STFaultHandlers(elementId, this->entryKey);} break;
    case K_INVOKE:              {symTab[this->nextKey()] = new STInvoke(elementId, this->entryKey);} break;
    case K_LINK:                {symTab[this->nextKey()] = new STLink(elementId, this->entryKey);} break;
    case K_PARTNER:             {symTab[this->nextKey()] = new STPartner(elementId, this->entryKey);} break;
    case K_PARTNERLINK:         {symTab[this->nextKey()] = new STPartnerLink(elementId, this->entryKey);} break;
    case K_PROCESS:             {symTab[this->nextKey()] = new STProcess(elementId, this->entryKey);} break;
    case K_RECEIVE:             {symTab[this->nextKey()] = new STReceive(elementId, this->entryKey);} break;
    case K_REPLY:               {symTab[this->nextKey()] = new STReply(elementId, this->entryKey);} break;
    case K_SCOPE:               {symTab[this->nextKey()] = new STScope(elementId, this->entryKey);} break;
    case K_TERMINATE:           {symTab[this->nextKey()] = new STTerminate(elementId, this->entryKey);} break;
    case K_VARIABLE:            {symTab[this->nextKey()] = new STVariable(elementId, this->entryKey);} break;
    case K_WAIT:                {symTab[this->nextKey()] = new STWait(elementId, this->entryKey);} break;
    case K_CATCH:               {symTab[this->nextKey()] = new STCatch(elementId, this->entryKey);} break;
//    case K_ONALARM:             {symTab[this->nextKey()] = new STCaseBranch(elementId, this->entryKey);} break;
    case K_ONMESSAGE:           {symTab[this->nextKey()] = new STOnMessage(elementId, this->entryKey);} break;
//    case K_OTHERWISE:           {symTab[this->nextKey()] = new STCaseBranch(elementId, this->entryKey);} break;
    case K_FROM:                {symTab[this->nextKey()] = new STFromTo(elementId, this->entryKey);} break;
    case K_TO:                  {symTab[this->nextKey()] = new STFromTo(elementId, this->entryKey);} break;
    case K_THROW:               {symTab[this->nextKey()] = new STThrow(elementId, this->entryKey);} break;
    case K_FLOW:		{symTab[this->nextKey()] = new STFlow(elementId, this->entryKey);} break;
    case K_SOURCE:              {symTab[this->nextKey()] = new STSourceTarget(elementId, this->entryKey);} break;
    case K_TARGET:              {symTab[this->nextKey()] = new STSourceTarget(elementId, this->entryKey);} break;
    /* all other */
    default :                   {symTab[this->nextKey()] = new STActivity(elementId, this->entryKey);} break;
  }
  
  return this->entryKey;
}





/*!
 * return symbol table entry
 * \param entryKey  symbol table entry key
 */
SymbolTableEntry* SymbolTable::lookup(kc::integer entryKey)
{
  return lookup(entryKey->value);
}






/*!
 * return symbol table entry
 * \param entryKey  symbol table entry key
 */
SymbolTableEntry* SymbolTable::lookup(unsigned int entryKey)
{
//  traceST("lookup(entryKey=" + intToString((dynamic_cast <SymbolTableEntry*> (symTab[entryKey]))->entryKey) + ")\n");
  SymbolTableEntry* result = dynamic_cast <SymbolTableEntry*> (symTab[entryKey]);
  assert(result != NULL);

  return result;
//  return (dynamic_cast <SymbolTableEntry*> (symTab[entryKey])); 
}






/*!
 * return
 */
unsigned int SymbolTable::getCurrentEntryKey()
{
  return this->entryKey;
}

/*!
 * create a new attribute object and return a pointer of these
 * \param name  name of attribute
 * \param value value of attribute
 */
STAttribute* SymbolTable::newAttribute(kc::casestring name, kc::casestring value)
{
  return (new STAttribute(name->name, value->name));
}

/*!
 * yes-no domain check
 * \param attributeName
 * \param attributeValue 
 */
void SymbolTable::checkAttributeValueYesNo(string attributeName, string attributeValue)
{
  if((attributeValue != (string)"yes") && (attributeValue != (string)"no"))
  {
    printErrorMsg("wrong value of " + attributeName + " attribute", (dynamic_cast< STElement *>(lookup(entryKey)))->line );
  }
}

/*!
 * returns valid or unvalid depending on attribute value
 * \param attributeName
 * \param attributeValue 
 */
bool SymbolTable::isValidAttributeValue(string attributeName, string attributeValue)
{
  /// check all attributes with yes or no domain
  if(attributeName == A__ABSTRACT_PROCESS)
  {
    checkAttributeValueYesNo(attributeName, attributeValue);
  }
  else if(attributeName == A__CREATE_INSTANCE)
  {
    checkAttributeValueYesNo(attributeName, attributeValue);
  }
  else if(attributeName == A__ENABLE_INSTANCE_COMPENSATION)
  {
    checkAttributeValueYesNo(attributeName, attributeValue);
  }
  else if(attributeName == A__INITIATE)
  {
    checkAttributeValueYesNo(attributeName, attributeValue);
  }
  else if(attributeName == A__SUPPRESS_JOIN_FAILURE)
  {
    checkAttributeValueYesNo(attributeName, attributeValue);
  }
  else if(attributeName == A__VARIABLE_ACCESS_SERIALIZABLE)
  {
    checkAttributeValueYesNo(attributeName, attributeValue);
  }
  
  return true;
}

/*!
 * this function checks whether the submitted name of attribute already element
 * of the attribute-name-set of the current BPEL-element is.
 * \param attributeName name of BPEL-element attribute
 */
bool SymbolTable::isDuplicate(unsigned int entryKey, STAttribute* attribute)
{
  /// pointer of attribute map
  std::map<string, STAttribute*>& mapOfAttributes = ((dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes);	

  /// iterator
  std::map<std::string, STAttribute*>::iterator mapOfAttributesIterator;  
  mapOfAttributesIterator = mapOfAttributes.begin();

  /// iteration loop over all attributes of the desired symbol table entry       
  while(mapOfAttributesIterator != mapOfAttributes.end())
  {  
    if(attribute->name.compare((*mapOfAttributesIterator).first) == 0)
    {
      return true;
    }
    ++mapOfAttributesIterator;
  }
  return false;
}


/*!
 * to add an attribute object to an existend symbol table entry,
 * but before it makes a duplicate and domain check
 * \param entryKey  symbol table entry Key
 * \param attribute attribute object
 */
void SymbolTable::addAttribute(unsigned int entryKey, STAttribute* attribute)
{
  traceST("addAttribute(entryKey=" + intToString(entryKey) + ", [name=" + attribute->name + ", value=" + attribute->value + "])\n");	
  
  // duplicate check
  if(isDuplicate(entryKey, attribute))
  {
    //traceST("DUPLIKAT GEFUNDEN\n" + string(attribute->name) + "=" + string(attribute->value) + "\n");
    printErrorMsg("attribute <" + (string)attribute->name + "> already exist", attribute->line );
  }
  // domain check
  else if(isValidAttributeValue(attribute->name, attribute->value))  
  {
   /// add attribute
   switch((dynamic_cast <SymbolTableEntry*> (symTab[entryKey]))->elementId)
   {
    case K_COPY: { /* no attributes */ break; }

    case K_COMPENSATE:
    {
//     traceST("cast to STCompensate\n");
      (dynamic_cast <STCompensate*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_COMPENSATIONHANDLER: { /* no attributes */ break; }

    case K_CORRELATIONSET:
    {
//     traceST("cast to STCorrelationSet\n");
      (dynamic_cast <STCorrelationSet*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_EVENTHANDLERS: { /* no attributes */ break; }

    case K_FAULTHANDLERS: { /* no attributes */ break; }
    
    case K_INVOKE:
    {
//      traceST("cast to STInvoke\n");
      (dynamic_cast <STInvoke*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }
    
    case K_LINK:
    {
//      traceST("cast to STLink\n");
      (dynamic_cast <STLink*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_PARTNER:
    {
//      traceST("cast to STPartner\n");
      (dynamic_cast <STPartner*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_PARTNERLINK:
    {
//      traceST("cast to STPartnerLink\n");
      (dynamic_cast <STPartnerLink*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_PROCESS:
    {
//      traceST("cast to STProcess\n");
      (dynamic_cast <STProcess*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_RECEIVE:
    {
//      traceST("cast to STReceive\n");
      (dynamic_cast <STReceive*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_REPLY:
    {
//      traceST("cast to STReply\n");
      (dynamic_cast <STReply*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }
        
    case K_SCOPE:
    {
//      traceST("cast to STScope\n");
      (dynamic_cast <STScope*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }    

    case K_TERMINATE:
    {
//      traceST("cast to STTerminate\n");
      (dynamic_cast <STTerminate*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }
    
    case K_VARIABLE:
    {
//      traceST("cast to STProcess\n");
      (dynamic_cast <STVariable*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_WAIT:
    {
//      traceST("cast to STWait\n");
      (dynamic_cast <STWait*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_CATCH:
    {
//      traceST("cast to STCatch\n");
      (dynamic_cast <STCatch*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_ONMESSAGE:
    {
//      traceST("cast to STOnMessage\n");
      (dynamic_cast <STOnMessage*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_FROM:
    {
//      traceST("cast to STFromTo\n");
      (dynamic_cast <STFromTo*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_TO:
    {
//      traceST("cast to STFromTo\n");
      (dynamic_cast <STFromTo*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_THROW:
    {
//      traceST("cast to STThrow\n");
      (dynamic_cast <STThrow*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_SOURCE:
    {
//      traceST("cast to STSourceTarget\n");
      (dynamic_cast <STSourceTarget*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    case K_TARGET:
    {
//      traceST("cast to STSourceTarget\n");
      (dynamic_cast <STSourceTarget*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }

    default:
    { /* cast to Activity */
//      traceST("cast to STActivity\n");    	
      (dynamic_cast <STActivity*> (symTab[entryKey]))->mapOfAttributes[attribute->name] = attribute;
      break;
    }
   } // end switch
  } // end if
}

/*!
 * wrapper for readAttributeValue(unsigned int, string)
 * \param entryKey  symbol table entry key
 * \param name      name of attribute
 */
string SymbolTable::readAttributeValue(kc::integer entryKey, string name)
{
  return readAttributeValue(entryKey->value, name);	
}

/*!
 * return value of an attribute object
 * \param entryKey  symbol table entry key
 * \param name      name of attribute 
 */
string SymbolTable::readAttributeValue(unsigned int entryKey, string name)
{
  return (readAttribute(entryKey, name)->value);	
}

/*!
 * wrapper for readAttribute(unsigned int, string)
 * \param entryKey  symbol table entry key
 * \param name      name of attribute
 */
STAttribute* SymbolTable::readAttribute(kc::integer entryKey, string name)
{
  return readAttribute(entryKey->value, name);
}

/*!
 * return a pointer of an attribute object,
 * doesn't exist an desired attribute then to create a new attribute with default value or empty string
 * \param entryKey  symbol table entry key
 * \param name      name of attribute 
 */
STAttribute* SymbolTable::readAttribute(unsigned int entryKey, string name)
{
  traceST("readAttribute(entryKey=" + intToString(entryKey) + ", name=" + name + ")\n");
  
  ///
  switch((dynamic_cast <SymbolTableEntry*> (symTab[entryKey]))->elementId)
  {
    case K_COMPENSATE:
    {
//      traceST("cast to STCompensate\n");
      STAttribute* attribute = (dynamic_cast <STCompensate*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STCompensate*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }

    case K_CORRELATION:
    {
//      traceST("cast to STActivity Correlation\n");
      STAttribute* attribute = (dynamic_cast <STActivity*> (symTab[entryKey]))->mapOfAttributes[name];
	  
	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STActivity*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }
	  
	  /// if attribute value empty then set default value
      if(attribute->value.empty())
      {
      	if(attribute->name == A__INITIATE)
      	{
      	  attribute->value = DV__INITIATE;
      	}
      }
      return attribute;
    }

    case K_CORRELATIONSET:
    {
//      traceST("cast to STActivity Correlation\n");
      STAttribute* attribute = (dynamic_cast <STCorrelationSet*> (symTab[entryKey]))->mapOfAttributes[name];
	  
	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STCorrelationSet*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }
	  
      return attribute;
    }

    case K_INVOKE:
    {
//      traceST("cast to STInvoke\n");
      STAttribute* attribute = (dynamic_cast <STInvoke*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STInvoke*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }

    case K_LINK:
    {
//      traceST("cast to STLink\n");
      STAttribute* attribute = (dynamic_cast <STLink*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STLink*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }

    case K_PARTNER:
    {
//      traceST("cast to STPartner\n");
      STAttribute* attribute = (dynamic_cast <STPartner*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STPartner*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }

    case K_PARTNERLINK:
    {
//      traceST("cast to STPartnerLink\n");
      STAttribute* attribute = (dynamic_cast <STPartnerLink*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STPartnerLink*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }

    case K_PICK:
    {
//      traceST("cast to STActivity Pick\n");
      STAttribute* attribute = (dynamic_cast <STActivity*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STActivity*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }
	  
	  /// if attribute value empty then set default value
      if(attribute->value.empty())
      {
      	if(attribute->name == A__CREATE_INSTANCE)
      	{
      	  attribute->value = DV__CREATE_INSTANCE;
      	}
      }
      return attribute;
    }

    case K_PROCESS:
    {
//      traceST("cast to STProcess\n");
      STAttribute* attribute = (dynamic_cast <STProcess*> (symTab[entryKey]))->mapOfAttributes[name];
	  
	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STProcess*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }
	  
	  /// if attribute value empty then set default value
      if(attribute->value.empty())
      {
      	if(attribute->name == A__QUERY_LANGUAGE)
      	{
      	  attribute->value = DV__QUERY_LANGUAGE;
      	}
      	else if(attribute->name == A__EXPRESSION_LANGUAGE)
      	{
      	  attribute->value = DV__EXPRESSION_LANGUAGE;
      	}
      	else if(attribute->name == A__SUPPRESS_JOIN_FAILURE)
      	{
      	  attribute->value = DV__SUPPRESS_JOIN_FAILURE;
      	}
      	else if(attribute->name == A__ENABLE_INSTANCE_COMPENSATION)
      	{
          attribute->value = DV__ENABLE_INSTANCE_COMPENSATION;
      	}
      	else if(attribute->name == A__ABSTRACT_PROCESS)
      	{
      	  attribute->value = DV__ABSTRACT_PROCESS;
      	}
      	else if(attribute->name == A__XMLNS)
      	{ 
      	  attribute->value = DV__XMLNS;
      	}	      	
      }

      return attribute;
    }

    case K_RECEIVE:
    {
//      traceST("cast to STReceive\n");
      STAttribute* attribute = (dynamic_cast <STReceive*> (symTab[entryKey]))->mapOfAttributes[name];
	  
	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STReceive*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }
	  
	  /// if attribute value empty then set default value
      if(attribute->value.empty())
      {
      	if(attribute->name == A__CREATE_INSTANCE)
      	{
      	  attribute->value = DV__CREATE_INSTANCE;
      	}
      }
      return attribute;
    }
    
    case K_REPLY:
    {
//      traceST("cast to STReply\n");
      STAttribute* attribute = (dynamic_cast <STReply*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
   
    case K_SCOPE:
    {
//      traceST("cast to STScope\n");
      STAttribute* attribute = (dynamic_cast <STScope*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }
	  
	  /// if attribute value empty then set default value
      if(attribute->value.empty())
      {
      	if(attribute->name == A__VARIABLE_ACCESS_SERIALIZABLE)
      	{
      	  attribute->value = DV__VARIABLE_ACCESS_SERIALIZABLE;
      	}
      }
      return attribute;
    }    

    case K_SOURCE:
    {
//      traceST("cast to STActivity Source\n");
      STAttribute* attribute = (dynamic_cast <STSourceTarget*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }
	  
	  /// if attribute value empty then set default value
      if(attribute->value.empty())
      {
      	if(attribute->name == A__TRANSITION_CONDITION)
      	{
      	  attribute->value = DV__TRANSITION_CONDITION;
      	}
      }
      return attribute;
    }

    case K_TERMINATE:
    {
//      traceST("cast to STTerminate\n");
      STAttribute* attribute = (dynamic_cast <STTerminate*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }

    case K_VARIABLE:
    {
//      traceST("cast to STPartnerLink\n");
      STAttribute* attribute = (dynamic_cast <STVariable*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    case K_WAIT:
    {
//      traceST("cast to STWait\n");
      STAttribute* attribute = (dynamic_cast <STWait*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    case K_CATCH:
    {
//      traceST("cast to STCatch\n");
      STAttribute* attribute = (dynamic_cast <STCatch*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    case K_ONMESSAGE:
    {
//      traceST("cast to STOnMessage\n");
      STAttribute* attribute = (dynamic_cast <STOnMessage*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    case K_FROM:
    {
//      traceST("cast to STFromTo\n");
      STAttribute* attribute = (dynamic_cast <STFromTo*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    case K_TO:
    {
//      traceST("cast to STFromTo\n");
      STAttribute* attribute = (dynamic_cast <STFromTo*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    case K_THROW:
    {
//      traceST("cast to STThrow\n");
      STAttribute* attribute = (dynamic_cast <STThrow*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    case K_TARGET:
    {
//      traceST("cast to STSourceTarget\n");
      STAttribute* attribute = (dynamic_cast <STSourceTarget*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
    
    default:
    { /* cast to Activity */
//      traceST("cast to STActivity\n");    	
      STAttribute* attribute = (dynamic_cast <STActivity*> (symTab[entryKey]))->mapOfAttributes[name];

	  if(attribute == NULL)
	  {
	    attribute = new STAttribute(name,"");
	    (dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes[name] = attribute;
	  }

      return attribute;
    }
  }
} 

/*!
 * print formatted symbol table error message
 * \param errorMsg  message
 */
void SymbolTable::printErrorMsg(string errorMsg, int line)
{
  trace("[SymbolTable]: " + errorMsg + "\n");
  if (line != 0)
  {
    trace("               See line " + intToString(line) + "\n");
  }
  error();
}

/*!
 * trace method for the ST class
 * \param traceMsg  message
 */
void SymbolTable::traceST(string traceMsg)
{
  trace(TRACE_VERY_DEBUG, "[ST] " + traceMsg);
}

/*!
 * trace method for the ST class without prefix [ST]
 * \param traceMsg  message
 */
void SymbolTable::traceSTwp(string traceMsg)
{
  trace(TRACE_VERY_DEBUG, traceMsg);
}

/*!
 * wrapper for checkAttributes(unsigned int entryKey, kc::casestring bpelElementValue)
 * \param entryKey  symbol table entry key
 * \param bpelElementValue  content of the BPEL-XML-element (e.g. <from> ... content ... </from>)
 */
void SymbolTable::checkAttributes(kc::integer entryKey, kc::casestring bpelElementValue)
{
  checkAttributes(entryKey->value, bpelElementValue);
}


/*!
 * wrapper for checkAttributes(unsigned int entryKey)
 * \param entryKey  symbol table entry key
 */
void SymbolTable::checkAttributes(kc::integer entryKey)
{
  checkAttributes(entryKey->value);
}

/*!
 * checked the presence of valid and necessary attribute combinations of BPEL-elements
 * \param entryKey  symbol table entry key
 * \param bpelElementValue  content of the BPEL-XML-element (e.g. <from> ... content ... </from>)
 */
void SymbolTable::checkAttributes(unsigned int entryKey, kc::casestring bpelElementValue)
{
  traceST("checkAttributes(" + intToString(entryKey) + ", " + string(bpelElementValue->name) + ")\n");
  /// pointer of attribute map
  std::map<string, STAttribute*>& mapOfAttributes = ((dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes);	

  //special case: <from> ... literal value ... </from>
  
  if((dynamic_cast <SymbolTableEntry*> (symTab[entryKey]))->elementId == K_FROM)
  {
    std::string lit = bpelElementValue->name;

    /* without attributes */
    if(mapOfAttributes.size() == 0)
    { /* literal value is empty */
      if(lit.empty())
      {
        printErrorMsg("from clause is wrong", (dynamic_cast< STElement *>(lookup(entryKey)))->line );
      }
    }
    /* with attributes */
    else
    {  /* literal value is empty */
      if(lit.empty())
      {
        /* element with attributes and without literal value */
        checkAttributes(entryKey);
      }
      else
      {
        printErrorMsg("from clause is wrong", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
    }
  }
  else
  {  
    checkAttributes(entryKey);      
  }
}

/*!
 * checked the presence of valid and necessary attribute combinations of BPEL-elements
 * \param entryKey  symbol table entry key
 */
void SymbolTable::checkAttributes(unsigned int entryKey)
{
  traceST("checkAttributes(" + intToString(entryKey) + ")" + "\n");
  /// pointer of attribute map
  std::map<string, STAttribute*>& mapOfAttributes = ((dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes);	

  /// iterator for the embedded map
  std::map<std::string, STAttribute*>::iterator mapOfAttributesIterator;  
  
  switch((dynamic_cast <SymbolTableEntry*> (symTab[entryKey]))->elementId)
  {
    case K_ASSIGN:
    {
      traceST("ASSIGN\n"); 
      checkAttributeSJF(entryKey);
    }
    break;

    case K_CASE:
    {
      traceST("CASE\n");

      bool conditionFlag;
      conditionFlag = false;
        
      mapOfAttributesIterator = mapOfAttributes.begin();
        
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__CONDITION)
        {
          conditionFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!conditionFlag)
      {
        printErrorMsg("attribute " + A__CONDITION + "=\"" + T__BOOLEAN_EXPR + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );          
      }        
      
    }
    break;

    case K_CATCH:
    {
      traceST("CATCH\n");
      /* todo */
    }
    break;  

    case K_CATCHALL:
    {
      traceST("CATCHALL\n");
      /* no attributes */
    }
    break;  

    case K_COMPENSATE:
    {
      traceST("COMPENSATE\n");
      checkAttributeSJF(entryKey);
    }
    break;

    case K_COMPENSATIONHANDLER:
    {
      traceST("COMPENSATIONHANDLER\n");
      /* no attributes */
    }
    break;  
 
    case K_COPY:
    {
      traceST("Copy\n");
      /* no attributes */
    }
    break;  

    case K_CORRELATION:
    {
      traceST("CORRELATION\n");
      
      bool setFlag;
      setFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__SET)
        {
          setFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!setFlag)
      {
        printErrorMsg("attribute " + A__SET + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
    }
    break;    

    case K_CORRELATIONSET:
    {
      traceST("CORRELATIONSET\n");
      
      bool nameFlag, propertiesFlag;
      nameFlag = propertiesFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__NAME)
        {
          nameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PROPERTIES)
        {
          propertiesFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!nameFlag)
      {
        printErrorMsg("attribute " + A__NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!propertiesFlag)
      {
        printErrorMsg("attribute " + A__PROPERTIES + "=\"" + T__QNAME_LIST + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      
    }
    break;    

    case K_EMPTY:
    {
      traceST("EMPTY\n");
      checkAttributeSJF(entryKey);
    }
    break;  

    case K_FROM:
    {
      traceST("FROM\n");
        
      bool validFrom = false;
      unsigned int attributeCompareCounter;
              
      // array to tag found attributes
      // to store the number of found attributes -> COUNTER field
      unsigned int foundAttributes[] =
      {  /*variable, part, query, partnerLk, endpointRef, property, expression, opaque, COUNTER*/
             0,      0,     0,       0,          0,          0,         0,        0,      0
      };

      unsigned int CSIZE = 7; // number of valid combination of attributes
      unsigned int ASIZE = 9; // number of attributes + COUNTER
      unsigned int COUNTER = 8; // position of counter within the arrays 

      // matrix for valid combination of attributes
      unsigned int validAttributeCombination[][9] =
      {  /*variable, part, query, partnerLk, endpointRef, property, expression, opaque, COUNTER*/
        {   1,      0,     0,       0,          0,          0,         0,        0,      1    },
        {   1,      1,     0,       0,          0,          0,         0,        0,      2    },
        {   1,      1,     1,       0,          0,          0,         0,        0,      3    },
        {   0,      0,     0,       1,          1,          0,         0,        0,      2    },
        {   1,      0,     0,       0,          0,          1,         0,        0,      2    },
        {   0,      0,     0,       0,          0,          0,         1,        0,      1    },
        {   0,      0,     0,       0,          0,          0,         0,        1,      1    }
      };
         
      mapOfAttributesIterator = mapOfAttributes.begin();
        
        ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__VARIABLE)
        {
          foundAttributes[0] = 1;
          foundAttributes[COUNTER]++;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PART)
        {
          foundAttributes[1] = 1;
          foundAttributes[COUNTER]++;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__QUERY)
        {
          foundAttributes[2] = 1;
          foundAttributes[COUNTER]++;            
          traceST((*mapOfAttributesIterator).first + "\n");
        }          
        else if(((*mapOfAttributesIterator).first) == A__PARTNER_LINK)
        {
          foundAttributes[3] = 1;
          foundAttributes[COUNTER]++;            
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__ENDPOINT_REFERENCE)
        {
          foundAttributes[4] = 1;
          foundAttributes[COUNTER]++;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PROPERTY)
        {
          foundAttributes[5] = 1;
          foundAttributes[COUNTER]++;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__EXPRESSION)
        {
          foundAttributes[6] = 1;
          foundAttributes[COUNTER]++;            
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__OPAQUE)
        {
          foundAttributes[7] = 1;
          foundAttributes[COUNTER]++;            
          traceST((*mapOfAttributesIterator).first + "\n");
        }                                        
        ++mapOfAttributesIterator;
      }
                
      // iteration about valid attribute combination
      for(unsigned int c = 0; c < CSIZE; c++)
      {
        // under or too much found attributes ... next combination
        if(validAttributeCombination[c][COUNTER] != foundAttributes[COUNTER]) continue;
 
        attributeCompareCounter = 0;
          
        // iteration about attributes
        for(unsigned int a = 0; a < ASIZE-1; a++)
        {  // compare scanned combination of attributes with allowed combination of attributes
          if((validAttributeCombination[c][a] == 1) && (foundAttributes[a] == 1))
          {
            attributeCompareCounter++;
          }
        }

        if(validAttributeCombination[c][COUNTER] == attributeCompareCounter)
        {
          validFrom = true;
          break;  
        }
          
      }
        
      if(!validFrom)
      {
        printErrorMsg("attribute combination within the from clause is wrong", (dynamic_cast< STElement *>(lookup(entryKey)))->line );          
      }        
    }
    break;    
 
    case K_EVENTHANDLERS:
    {
      traceST("EVENTHANDLERS\n");
      /* no attributes */
    }
    break;  
 
    case K_FAULTHANDLERS:
    {
      traceST("EVENTHANDLERS\n");
      /* no attributes */
    }
    break;  

    case K_FLOW:
    {
      traceST("EVENTHANDLERS\n");
      checkAttributeSJF(entryKey);
    }
    break;  
 
    case K_INVOKE:
    {
      traceST("INVOKE\n");

      bool partnerLinkFlag, portTypeFlag, operationFlag, inputVariableFlag, outputVariableFlag;
      partnerLinkFlag = portTypeFlag = operationFlag = inputVariableFlag = outputVariableFlag =false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
        ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__PARTNER_LINK)
        {
          partnerLinkFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PORT_TYPE)
        {
          portTypeFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__OPERATION)
        {
          operationFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }        
        else if(((*mapOfAttributesIterator).first) == A__INPUT_VARIABLE)
        {
          inputVariableFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        /* optional */
        /*
        else if(((*mapOfAttributesIterator).first) == A__OUTPUT_VARIABLE)
        {
          outputVariableFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }*/
        ++mapOfAttributesIterator;
      }
      
      if(!partnerLinkFlag)
      {
      printErrorMsg("attribute " + A__PARTNER_LINK + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );      
      }
      else if(!portTypeFlag)
      {
        printErrorMsg("attribute " + A__PORT_TYPE + "=\"" + T__QNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!operationFlag)
      {
        printErrorMsg("attribute " + A__OPERATION + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }            
      else if(!inputVariableFlag)
      {
        printErrorMsg("attribute " + A__INPUT_VARIABLE + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      /* optional */
      /*
      else if(!outputVariableFlag)
      {
        printErrorMsg("attribute " + A__OUTPUT_VARIABLE + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }*/
      checkAttributeSJF(entryKey);
    }
    break;  

    case K_LINK:
    {
      traceST("LINK\n");

      bool nameFlag;
      nameFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__NAME)
        {
          nameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!nameFlag)
      {
        printErrorMsg("attribute " + A__NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }

    }
    break;  

    case K_ONALARM:
    {
      traceST("ONALARM\n");
      
      bool forFlag, untilFlag;
      forFlag = untilFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__FOR)
        {
          forFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__UNTIL)
        {
          untilFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }

      /// only one attribute is allowed
      if((forFlag && !untilFlag) || (untilFlag && !forFlag))
      {  
        /* all okay */
      }
      else
      {
        // no attribute
        if(!forFlag && !untilFlag)
        {
          printErrorMsg("attribute " + A__FOR + "=\"" + T__DURATION_EXPR + "\"  or " + A__UNTIL + "=\"" + T__DEADLINE_EXPR + " is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );          
        }
        // to much attributes, only one is allowed
        else
        {
          printErrorMsg("both attributes " + A__FOR + "=\"" + T__DURATION_EXPR + "\"  and " + A__UNTIL + "=\"" + T__DEADLINE_EXPR + " are not allowed", (dynamic_cast< STElement *>(lookup(entryKey)))->line );                  
        }
      }
    }
    break;    

    case K_ONMESSAGE:
    {
      traceST("ONMESSAGE\n");
      
      bool partnerLinkFlag, portTypeFlag, operationFlag, variableFlag;
      partnerLinkFlag = portTypeFlag = operationFlag = variableFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__PARTNER_LINK)
        {
          partnerLinkFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PORT_TYPE)
        {
          portTypeFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__OPERATION)
        {
          operationFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }        
        else if(((*mapOfAttributesIterator).first) == A__VARIABLE)
        {
          variableFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }        
        ++mapOfAttributesIterator;
      }
      
      if(!partnerLinkFlag)
      {
        printErrorMsg("attribute " + A__PARTNER_LINK + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!portTypeFlag)
      {
        printErrorMsg("attribute " + A__PORT_TYPE + "=\"" + T__QNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!operationFlag)
      {
        printErrorMsg("attribute " + A__OPERATION + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!variableFlag)
      {
        printErrorMsg("attribute " + A__VARIABLE + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
    }
    break;

    case K_PARTNER:
    {
      traceST("PARTNER\n");
      
      bool nameFlag;
      nameFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__NAME)
        {
          nameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!nameFlag)
      {
        printErrorMsg("attribute " + A__NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
    }
    break;    

    case K_PARTNERLINK:
    {
      traceST("PARTNERLINK\n");
      
      /* attention case differentation
       * case 1:
	   *     <partnerLinks>?
	   *       <partnerLink name="ncname" partnerLinkType="qname"
	   *                    myRole="ncname"? partnerRole="ncname"?>+
	   *       </partnerLink>
	   *     </partnerLinks>
       * case 2:
	   *     <partners>?
	   *       <partner name="ncname">+
	   *         <partnerLink name="ncname"/>+
	   *       </partner>
	   *     </partners> 
       * 
       */
      
      bool nameFlag, partnerLinkTypeFlag;
      nameFlag = partnerLinkTypeFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__NAME)
        {
          nameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PARTNER_LINK_TYPE)
        {
          partnerLinkTypeFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      // important for both cases
      if(!nameFlag)
      {
        printErrorMsg("attribute " + A__NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      // important for case 1
/*      else if((((STPartnerLink*)symTab[entryKey])->isInPartners == false) && !partnerLinkTypeFlag)
      {
        printErrorMsg("attribute " + A__PARTNER_LINK_TYPE + "=\"" + T__QNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
  */    
    }
    break;    
      
    case K_PICK:
    {
      traceST("PICK\n");
      checkAttributeSJF(entryKey);
    }
    break;      

    case K_PROCESS:
    {
      traceST("PROCESS\n");
      
      bool nameFlag, targetNamespaceFlag;
      nameFlag = targetNamespaceFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__NAME)
        {
          nameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__TARGET_NAMESPACE)
        {
          targetNamespaceFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!nameFlag)
      {
        printErrorMsg("attribute " + A__NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!targetNamespaceFlag)
      {
        printErrorMsg("attribute " + A__TARGET_NAMESPACE + "=\"" + T__ANYURI + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      checkAttributeSJF(entryKey);
    }
    break;    

    case K_RECEIVE:
    {
      traceST("RECEIVE\n");      

      bool partnerLinkFlag, portTypeFlag, operationFlag, variableFlag;
      partnerLinkFlag = portTypeFlag = operationFlag = variableFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__PARTNER_LINK)
        {
          partnerLinkFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PORT_TYPE)
        {
          portTypeFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__OPERATION)
        {
          operationFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }        
        else if(((*mapOfAttributesIterator).first) == A__VARIABLE)
        {
          variableFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }        
        ++mapOfAttributesIterator;
      }
      
      if(!partnerLinkFlag)
      {
        printErrorMsg("attribute " + A__PARTNER_LINK + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!portTypeFlag)
      {
        printErrorMsg("attribute " + A__PORT_TYPE + "=\"" + T__QNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!operationFlag)
      {
        printErrorMsg("attribute " + A__OPERATION + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!variableFlag)
      {
        printErrorMsg("attribute " + A__VARIABLE + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      checkAttributeSJF(entryKey);
    }
    break;    

    case K_REPLY:
    {
      traceST("REPLY\n");

      bool partnerLinkFlag, portTypeFlag, operationFlag;
      partnerLinkFlag = portTypeFlag = operationFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__PARTNER_LINK)
        {
          partnerLinkFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PORT_TYPE)
        {
          portTypeFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__OPERATION)
        {
          operationFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }        
        ++mapOfAttributesIterator;
      }
      
      if(!partnerLinkFlag)
      {
        printErrorMsg("attribute " + A__PARTNER_LINK + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!portTypeFlag)
      {
        printErrorMsg("attribute " + A__PORT_TYPE + "=\"" + T__QNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      else if(!operationFlag)
      {
        printErrorMsg("attribute " + A__OPERATION + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      checkAttributeSJF(entryKey);
    }
    break;    

    case K_SCOPE:
    {
      traceST("SCOPE\n");
      checkAttributeSJF(entryKey);
    }
    break;

    case K_SEQUENCE:
    {
      traceST("SEQUENCE\n");
      checkAttributeSJF(entryKey);
    }
    break;      

    case K_SOURCE:
    {
      traceST("SOURCE\n");

      bool linkNameFlag;
      linkNameFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__LINK_NAME)
        {
          linkNameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!linkNameFlag)
      {
        printErrorMsg("attribute " + A__LINK_NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }

    }
    break;  

    case K_SWITCH:
    {
      traceST("SWITCH\n");
      checkAttributeSJF(entryKey);
    }
    break;      

    case K_TARGET:
    {
      traceST("TARGET\n");

      bool linkNameFlag;
      linkNameFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__LINK_NAME)
        {
          linkNameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!linkNameFlag)
      {
        printErrorMsg("attribute " + A__LINK_NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }

    }
    break;  

    case K_TERMINATE:
    {
      traceST("TERMINATE\n");
      checkAttributeSJF(entryKey);
    }
    break;    

    case K_THROW:
    {
      traceST("THROW\n");

      bool faultNameFlag;
      faultNameFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__FAULT_NAME)
        {
          faultNameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!faultNameFlag)
      {
        printErrorMsg("attribute " + A__FAULT_NAME + "=\"" + T__QNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      checkAttributeSJF(entryKey);
    }
    break;    

    case K_TO:
    {
      traceST("TO\n");
      
      bool validTo = false;
      unsigned int attributeCompareCounter;
            
      // array to tag found attributes
      // to store the number of found attributes -> COUNTER field
      unsigned int foundAttributes[] =
      {  /*variable, part, query, partnerLk, property, COUNTER*/
           0,      0,     0,       0,        0,      0
      };

      unsigned int CSIZE = 5; // number of valid combination of attributes
      unsigned int ASIZE = 6; // number of attributes + COUNTER
      unsigned int COUNTER = 5; // position of counter within the arrays 

      // matrix for valid combination of attributes
      unsigned int validAttributeCombination[][6] =
      {  /*variable, part, query, partnerLk, property, COUNTER*/
         {   1,      0,     0,       0,      0,      1    },
         {   1,      1,     0,       0,      0,      2    },
         {   1,      1,     1,       0,      0,      3    },
         {   0,      0,     0,       1,      0,      1    },
         {   1,      0,     0,       0,      1,      2    }
      };
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__VARIABLE)
        {
          foundAttributes[0] = 1;
          foundAttributes[COUNTER]++;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PART)
        {
          foundAttributes[1] = 1;
          foundAttributes[COUNTER]++;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__QUERY)
        {
          foundAttributes[2] = 1;
          foundAttributes[COUNTER]++;          
          traceST((*mapOfAttributesIterator).first + "\n");
        }        
        else if(((*mapOfAttributesIterator).first) == A__PARTNER_LINK)
        {
          foundAttributes[3] = 1;
          foundAttributes[COUNTER]++;          
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__PROPERTY)
        {
          foundAttributes[4] = 1;
          foundAttributes[COUNTER]++;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
            
      // iteration about valid attribute combination
      for(unsigned int c = 0; c < CSIZE; c++)
      {
        // under or too much found attributes ... next combination
        if(validAttributeCombination[c][COUNTER] != foundAttributes[COUNTER]) continue;
  
        attributeCompareCounter = 0;
        
        // iteration about attributes
        for(unsigned int a = 0; a < ASIZE-1; a++)
        {  // compare scanned combination of attributes with allowed combination of attributes
          if((validAttributeCombination[c][a] == 1) && (foundAttributes[a] == 1))
          {
            attributeCompareCounter++;
          }
        }

        if(validAttributeCombination[c][COUNTER] == attributeCompareCounter)
        {
          validTo = true;
          break;  
        }
        
      }
      
      if(!validTo)
      {
        printErrorMsg("attribute combination within the to clause is wrong", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }      
      }
      break;    

    case K_VARIABLE:
    {
      traceST("VARIABLE\n");

      bool nameFlag;
      nameFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__NAME)
        {
          nameFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!nameFlag)
      {
        printErrorMsg("attribute " + A__NAME + "=\"" + T__NCNAME + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }

    }
    break;  

    case K_WAIT:
    {
      traceST("WAIT\n");
      
      bool forFlag, untilFlag;
      forFlag = untilFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__FOR)
        {
          forFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        else if(((*mapOfAttributesIterator).first) == A__UNTIL)
        {
          untilFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }

      /// only one attribute is allowed
      if((forFlag && !untilFlag) || (untilFlag && !forFlag))
      {  
        /* all okay */
      }
      else
      {
        // no attribute
        if(!forFlag && !untilFlag)
        {
          printErrorMsg("attribute " + A__FOR + "=\"" + T__DURATION_EXPR + "\"  or " + A__UNTIL + "=\"" + T__DEADLINE_EXPR + " is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );          
        }
        // to much attributes, only one is allowed
        else
        {
          printErrorMsg("both attributes " + A__FOR + "=\"" + T__DURATION_EXPR + "\"  and " + A__UNTIL + "=\"" + T__DEADLINE_EXPR + " are not allowed", (dynamic_cast< STElement *>(lookup(entryKey)))->line );                  
        }
      }
      checkAttributeSJF(entryKey);
    }
    break;    

    case K_WHILE:
    {
      traceST("WHILE\n");

      bool conditionFlag;
      conditionFlag = false;
       
      mapOfAttributesIterator = mapOfAttributes.begin();
      
      ///
      while(mapOfAttributesIterator != mapOfAttributes.end())
      {  
        if(((*mapOfAttributesIterator).first) == A__CONDITION)
        {
          conditionFlag = true;
          traceST((*mapOfAttributesIterator).first + "\n");
        }
        ++mapOfAttributesIterator;
      }
      
      if(!conditionFlag)
      {
        printErrorMsg("attribute " + A__CONDITION + "=\"" + T__BOOLEAN_EXPR + "\" is missing", (dynamic_cast< STElement *>(lookup(entryKey)))->line );        
      }
      checkAttributeSJF(entryKey);  
    }
    break;      
  }
}

/*!
 * checked the presence of suppressJoinFailure (SJF) attribute
 * in the set of all attributes of the current ST entry and
 * create a new entry at absence of the SJF attribute with the
 * SJF value of the parent BPEL element
 * 
 * \param entryKey  symbol table entry key
 */
void SymbolTable::checkAttributeSJF(unsigned int entryKey)
{
  traceST("checkAttributeSJF(" + intToString(entryKey) + ")\n");

  /// pointer of attribute map
  std::map<string, STAttribute*>& mapOfAttributes = ((dynamic_cast <STElement*> (symTab[entryKey]))->mapOfAttributes);	
  /// iterator for the embedded map
  std::map<std::string, STAttribute*>::iterator mapOfAttributesIterator;  
  mapOfAttributesIterator = mapOfAttributes.begin();

  bool suppressJoinFailureFlag = false;
      
  /// iteration about the set of attributes
  while(mapOfAttributesIterator != mapOfAttributes.end())
  {  
    if(((*mapOfAttributesIterator).first) == A__SUPPRESS_JOIN_FAILURE)
    {
      suppressJoinFailureFlag = true;
      traceST((*mapOfAttributesIterator).first + "\n");
    }
      ++mapOfAttributesIterator;
  }
  
  // attribute doesn't exist
  if(!suppressJoinFailureFlag)
  {  // create a new entry (within readAttribute())
     if(readAttributeValue(entryKey, "suppressJoinFailure").empty())
     {  
       // SJF value of the parent BPEL element
       readAttribute(entryKey, "suppressJoinFailure")->value = this->suppressJoinFailureStack.top();
     }
  }

  // to store current SJF value for all BPEL child elements
  this->suppressJoinFailureStack.push(readAttributeValue(entryKey, "suppressJoinFailure"));

  traceST("SJF\t" + this->suppressJoinFailureStack.top());	
} 

/*!
 * to clear the value of the current BPEL element of the
 * suppressJoinFailure auxiliary storage 
 */
void SymbolTable::popSJFStack()
{
  this->suppressJoinFailureStack.pop();
}


/********************************************
 * implementation of SymbolTableEntry CLASS
 ********************************************/
 
/*!
 * constructor
 */
SymbolTableEntry::SymbolTableEntry(unsigned int elementId)
{
  setElementId(elementId);
}


/*!
 * constructor
 */
SymbolTableEntry::SymbolTableEntry(unsigned int elementId, unsigned int entryKey)
{
  setElementId(elementId);
  setEntryKey(entryKey);
}


/*!
 * constructor
 */
SymbolTableEntry::SymbolTableEntry() {}


/*!
 * destructor
 */
SymbolTableEntry::~SymbolTableEntry() {}

/*!
 * 
 */
void SymbolTableEntry::setElementId(unsigned int id)
{
  this->elementId = id;
}

/*!
 * 
 */
void SymbolTableEntry::setEntryKey(unsigned int key)
{
  this->entryKey = key;
}
 
/*!
 * 
 */
unsigned int SymbolTableEntry::getElementId()
{
  return this->elementId;
}

/*!
 * 
 */
unsigned int SymbolTableEntry::getEntryKey()
{
  return this->entryKey;
}

/********************************************
 * implementation of STActivity CLASS
 ********************************************/

/*!
 * constructor
 */
STActivity::STActivity(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * constructor
 */
STActivity::STActivity() {}

/*!
 * destructor
 */
STActivity::~STActivity() {}

 
/********************************************
 * implementation of STAttribute CLASS
 ********************************************/

/*!
 * constructor
 */
STAttribute::STAttribute(string name, string value)
{
  this->name = name;
  this->value = value;
  if(value.empty()) {
  	this->line = 0;
  }
  else {
  	this->line = yylineno;
  }
}

/*!
 * destructor
 */
STAttribute::~STAttribute() {}

/********************************************
 * implementation of STCommunicationActivity CLASS
 ********************************************/

/*!
 * constructor
 */
STCommunicationActivity::STCommunicationActivity(unsigned int elementId, unsigned int entryKey)
 :STActivity(elementId, entryKey) {}

/*!
 * destructor
 */
STCommunicationActivity::~STCommunicationActivity() {}

/********************************************
 * implementation of STCompensate CLASS
 ********************************************/

/*!
 * constructor
 */
STCompensate::STCompensate(unsigned int elementId, unsigned int entryKey)
 :STActivity(elementId, entryKey) {}

/*!
 * destructor
 */
STCompensate::~STCompensate() {}
 
/********************************************
 * implementation of STCompensationHandler CLASS
 ********************************************/

/*!
 * constructor
 */
STCompensationHandler::STCompensationHandler(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * destructor
 */
STCompensationHandler::~STCompensationHandler() {}
 
/********************************************
 * implementation of STCorrelationSet CLASS
 ********************************************/

/*!
 * constructor
 */
STCorrelationSet::STCorrelationSet(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * destructor
 */
STCorrelationSet::~STCorrelationSet() {}

/********************************************
 * implementation of Element CLASS
 ********************************************/
 

/*!
 * Collects all source links having an id between firstId and lastId. This
 * collection is used for dead path elimination.
 */
void STElement::processLinks(unsigned int firstId, unsigned int lastId)
{
  if (!parameters[P_NEWLINKS])
    return;

  for (unsigned int id = firstId+1; id <= lastId; id++)
  { 
    if  (symTab.lookup(id) != NULL){

      if (typeid(*(symTab.lookup(id))) == typeid(STSourceTarget)) 
    {
      STSourceTarget* sourceLink = dynamic_cast<STSourceTarget *> (symTab.lookup(id));
      if (sourceLink != NULL && sourceLink->isSource)
      {
	enclosedSourceLinks.insert(sourceLink->link);
      }
    }
    }
  }
}

/*!
 * constructor
 */
STElement::STElement() {
  line = yylineno;
}

/*!
 * destructor
 */
STElement::~STElement() {}
 
/********************************************
 * implementation of STEnvelope CLASS
 ********************************************/

/*!
 * destructor
 */
STEnvelope::~STEnvelope() {}
 
/********************************************
 * implementation of EventHandlers CLASS
 ********************************************/

/*!
 * constructor
 */
STEventHandlers::STEventHandlers(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * destructor
 */
STEventHandlers::~STEventHandlers() {}

/********************************************
 * implementation of FaultHandlers CLASS
 ********************************************/

/*!
 * constructor
 */
STFaultHandlers::STFaultHandlers(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * destructor
 */
STFaultHandlers::~STFaultHandlers() {}

/********************************************
 * implementation of Invoke CLASS
 ********************************************/

/*!
 * constructor
 */
STInvoke::STInvoke(unsigned int elementId, unsigned int entryKey)
 :STCommunicationActivity(elementId, entryKey) {}

/*!
 * destructor
 */
STInvoke::~STInvoke() {}

/********************************************
 * implementation of Link CLASS
 ********************************************/

/*!
 * constructor
 */
STLink::STLink(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) 
{
  sourceId = 0;
  targetId = 0;
  parentId = 0;
}

/*!
 * destructor
 */
STLink::~STLink() {}

/********************************************
 * implementation of Partner CLASS
 ********************************************/

/*!
 * constructor
 */
STPartner::STPartner(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * destructor
 */
STPartner::~STPartner() {}

 
/********************************************
 * implementation of PartnerLink CLASS
 ********************************************/

/*!
 * constructor
 */
STPartnerLink::STPartnerLink(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * destructor
 */
STPartnerLink::~STPartnerLink() {}

/********************************************
 * implementation of Process CLASS
 ********************************************/

/*!
 * constructor
 */
STProcess::STProcess(unsigned int elementId, unsigned int entryKey)
 :STScope(elementId, entryKey) 
// :SymbolTableEntry(elementId, entryKey) 
{
  parentScopeId = 0;
}

/*!
 * destructor
 */
STProcess::~STProcess() {}

/*!
 * adds a PartnerLink to the Process
 */
void STProcess::addPartnerLink(STPartnerLink* pl)
{
  trace(TRACE_VERY_DEBUG, "[ST] Trying to add PartnerLink with name \"" + symTab.readAttributeValue(pl->entryKey, "name") + "\"\n");
  if (! partnerLinks.empty())
  {
    for (list<STPartnerLink *>::iterator iter = partnerLinks.begin(); iter != partnerLinks.end(); iter++)
    {
      if (symTab.readAttributeValue((*iter)->entryKey, "name") == symTab.readAttributeValue(pl->entryKey, "name"))
      {
	trace("\n");
        trace("ERROR: Two PartnerLinks with same name.\n");
	trace("       Name of double PartnerLink is \"" 
		       + symTab.readAttributeValue(pl->entryKey, "name") + "\"\n");
	trace("       See lines " + intToString(symTab.readAttribute((*iter)->entryKey, "name")->line) 
	      + " and " + intToString(symTab.readAttribute(pl->entryKey, "name")->line) + ".\n\n");
	error();
      }
    }
  }

  partnerLinks.push_back(pl);
}

/*!
 * checks if a PartnerLink is declared in the Process
 */
STPartnerLink * STProcess::checkPartnerLink(std::string name)
{
  if (name == "")
  {
    return NULL;
  }

  trace(TRACE_VERY_DEBUG, "[ST] Checking for PartnerLink with name \"" + name + "\"\n");
  list<STPartnerLink *>::iterator iter;
  if (! partnerLinks.empty())
  {
    for (iter = partnerLinks.begin(); iter != partnerLinks.end(); iter++)
    {
      if (symTab.readAttributeValue((*iter)->entryKey, "name") == name)
      {
	return (*iter);
      }
    }
  }
  if (partnerLinks.empty() || iter == partnerLinks.end())
  {
    trace("\n");
    trace("ERROR: Name of undefined PartnerLink is \"" 
	           + name + "\"\n\n");
    error();
  }
}

/// adds a Channel to the Process
std::string STProcess::addChannel(std::string channelName, bool isInChannel)
{
  trace(TRACE_VERY_DEBUG, "[CS] Adding channel " + channelName + "\n"); 
  if (isInChannel)
  {
    trace(TRACE_VERY_DEBUG, "[CS]  --> incoming channel\n");
    inChannels.insert(channelName);
  }
  else
  {
    trace(TRACE_VERY_DEBUG, "[CS]  --> outgoinig channel\n");
    outChannels.insert(channelName);
  }
  // return unique name
  return channelName;

}



/********************************************
 * implementation of Receive CLASS
 ********************************************/

/*!
 * constructor
 */
STReceive::STReceive(unsigned int elementId, unsigned int entryKey)
 :STCommunicationActivity(elementId, entryKey) {}

/*!
 * destructor
 */
STReceive::~STReceive() {}

/********************************************
 * implementation of Reply CLASS
 ********************************************/

/*!
 * constructor
 */
STReply::STReply(unsigned int elementId, unsigned int entryKey)
 :STCommunicationActivity(elementId, entryKey) {}

/*!
 * destructor
 */
STReply::~STReply() {}

/********************************************
 * implementation of Scope CLASS
 ********************************************/

/*!
 * add a new Variable with scope ID and name
 */
std::string STScope::addVariable(STVariable * variable) 
{
  trace(TRACE_VERY_DEBUG, "[ST] Adding variable " + variable->mapOfAttributes["name"]->value + "\n");
  if (! variables.empty())
  {
    for (list<STVariable *>::iterator iter = variables.begin();
	    iter != variables.end(); 
	    iter++)
    {
      if ((*iter)->mapOfAttributes["name"]->value == variable->mapOfAttributes["name"]->value)
      {
	trace("\n");
	trace("ERROR: Two Variables with same name.\n");
	trace("       Name of double Variable is \"" 
		     + variable->mapOfAttributes["name"]->value
		     + "\".\n");
	trace("       See lines " + intToString(symTab.readAttribute((*iter)->entryKey, "name")->line) 
	      + " and " + intToString(symTab.readAttribute(variable->entryKey, "name")->line) + ".\n\n");
	error();
      }
    }
  }
    
  variables.push_back(variable);
  std::string uniqueId = intToString(entryKey) + "." + variable->mapOfAttributes["name"]->value;
  symTab.variables.push_back(uniqueId);
  return uniqueId;
}

/// checks for a variable with a given name and returns pointer to the object
STVariable * STScope::checkVariable(std::string name, int line, STScope * callingScope, bool isFaultVariable)
{
  if (name == "")
  {
    return NULL;
  }
  trace(TRACE_VERY_DEBUG, "[ST] Checking variable " + name + "\n");
  if (! variables.empty())
  {
    trace(TRACE_VERY_DEBUG, "[ST] Looking in Scope\n");
    for (list<STVariable *>::iterator iter = variables.begin();
	    iter != variables.end(); 
	    iter++)
    {
      if ((*iter)->mapOfAttributes["name"]->value == name)
      {
	return (*iter);
      }
    }
  }
  if (parentScopeId != 0)
  {
    trace(TRACE_VERY_DEBUG, "[ST] Looking in parent Scope " + intToString(parentScopeId) +"\n");
    return (dynamic_cast <STScope*> (symTab.lookup(parentScopeId)))->checkVariable(name, line, callingScope, isFaultVariable);
  }
  else if (! isFaultVariable)
  {
    trace("\n");
    trace("ERROR: Undefined Variable found.\n");
    trace("       Name of undefined Variable is \"" 
		   + name + "\".\n");
    trace("       See line " + intToString(line) + ".\n\n");
    error();
  }

  // create FaultVariable
  trace(TRACE_VERY_DEBUG, "[ST] Adding fault variable " + name + "\n");
  unsigned int key = symTab.insert(K_VARIABLE);
  STVariable * stVariable = NULL;
  stVariable = dynamic_cast<STVariable*> (symTab.lookup(key));
  if (stVariable == NULL)
  {
    throw Exception(CHECK_SYMBOLS_CAST_ERROR, "Could not cast correctly", pos(__FILE__, __LINE__, __FUNCTION__));
  }
  // add attributes
  symTab.addAttribute(key, symTab.newAttribute(kc::mkcasestring("name"), kc::mkcasestring(name.c_str())));
  callingScope->addVariable(stVariable);
  
  return stVariable;
}

/*!
 * adds a Link to the enclosedLinks list
 */
void STScope::addLink(STLink * link)
{
  trace(TRACE_VERY_DEBUG, "[ST] Adding " + link->name
 	                + " to list of inner links for Scope " 
			+ intToString(entryKey) + "\n");
  enclosedLinks.push_back(link);
  if (parentScopeId != 0)
  {
    (dynamic_cast<STScope *> (symTab.lookup(parentScopeId)))->addLink(link);
  }
}

/*!
 * constructor
 */
STScope::STScope(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) 
{
}

/*!
 * destructor
 */
STScope::~STScope() {}

/********************************************
 * implementation of Terminate CLASS
 ********************************************/

/*!
 * constructor
 */
STTerminate::STTerminate(unsigned int elementId, unsigned int entryKey)
 :STActivity(elementId, entryKey) {}

/*!
 * destructor
 */
STTerminate::~STTerminate() {}
 
/********************************************
 * implementation of Variable CLASS
 ********************************************/

/*!
 * constructor
 */
STVariable::STVariable(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) {}

/*!
 * destructor
 */
STVariable::~STVariable() {}

/********************************************
 * implementation of Wait CLASS
 ********************************************/

/*!
 * constructor
 */
STWait::STWait(unsigned int elementId, unsigned int entryKey)
 :STActivity(elementId, entryKey) {}

/*!
 * destructor
 */
STWait::~STWait() {}


/********************************************
 * implementation of Catch CLASS
 ********************************************/

/*!
 * constructor
 */
STCatch::STCatch(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) 
{
  faultVariable = NULL;
}

/*!
 * destructor
 */
STCatch::~STCatch() {}


/********************************************
 * implementation of OnMessage CLASS
 ********************************************/

/*!
 * constructor
 */
STOnMessage::STOnMessage(unsigned int elementId, unsigned int entryKey)
 :STCommunicationActivity(elementId, entryKey) 
{
  variable = NULL;
}

/*!
 * destructor
 */
STOnMessage::~STOnMessage() {}


/********************************************
 * implementation of FromTo CLASS
 ********************************************/

/*!
 * constructor
 */
STFromTo::STFromTo(unsigned int elementId, unsigned int entryKey)
 :STCommunicationActivity(elementId, entryKey) 
{
  variable = NULL;
  partnerLink = NULL;
}

/*!
 * destructor
 */
STFromTo::~STFromTo() {}


/********************************************
 * implementation of Throw CLASS
 ********************************************/

/*!
 * constructor
 */
STThrow::STThrow(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) 
{
  faultVariable = NULL;
}

/*!
 * destructor
 */
STThrow::~STThrow() {}


/********************************************
 * implementation of Flow CLASS
 ********************************************/

/*!
 * constructor
 */
STFlow::STFlow(unsigned int elementId, unsigned int entryKey)
 :STActivity(elementId, entryKey) 
{
  parentFlowId = 0;
}

/*!
 * constructor
 */
STFlow::STFlow()
{
  parentFlowId = 0;
}

/*!
 * destructor
 */
STFlow::~STFlow() {}

/*!
 * adds a Link to the Flow
 */
std::string STFlow::addLink(STLink* link)
{
  trace(TRACE_VERY_DEBUG, "[ST] Trying to add Link with name \"" + symTab.readAttributeValue(link->entryKey, "name") + "\"\n");

  if (! links.empty())
  {
    for (list<STLink *>::iterator iter = links.begin(); iter != links.end(); iter++)
    {
      if (symTab.readAttributeValue((*iter)->entryKey, "name") == symTab.readAttributeValue(link->entryKey, "name"))
      {
	trace("\n");
        trace("ERROR: Two Links with same name.\n");
	trace("       Name of double Link is \"" + 
			symTab.readAttributeValue(link->entryKey,"name") + "\".\n");
	trace("       See lines " + intToString(symTab.readAttribute((*iter)->entryKey, "name")->line)
	      + " and " + intToString(symTab.readAttribute(link->entryKey, "name")->line) + ".\n\n");
	error();
      }
    }
  }
  
  links.push_back(link);
  return intToString(entryKey) + "." + symTab.readAttributeValue(link->entryKey, "name");
}

/*!
 * checks if a Link is declared in the Flow
 */
STLink * STFlow::checkLink(std::string name, unsigned int id, bool isSource)
{
  if (name == "")
  {
    return NULL;
  }

  trace(TRACE_VERY_DEBUG, "[ST] Checking for Link with name \"" + name + "\"\n");

  if (! links.empty())
  {
    trace(TRACE_VERY_DEBUG, "[ST] Looking in Flow\n");
    for (list<STLink *>::iterator iter = links.begin();
	    iter != links.end(); 
	    iter++)
    {
      if (symTab.readAttributeValue((*iter)->entryKey, "name") == name)
      {
	if (isSource)
	{
	  if ((*iter)->sourceId == 0)
	  {
	    (*iter)->sourceId = id;
	  }	    
	  else
	  {
            trace("ERROR: Link \"" + name 
			   + "\" was already used as source\n\n");
	    error();
	  }
	}
	else
	{
	  if ((*iter)->targetId == 0)
	  {
	    /// \todo: change source and target ID to real ID!
	    (*iter)->targetId = id;
	  }	    
	  else
	  {
            trace("ERROR: Link \"" + name 
			   + "\" was already used as target\n\n");
	  }
	}
	return (*iter);
      }
    }
  }
  if (parentFlowId != 0)
  {
    trace(TRACE_VERY_DEBUG, "[ST] Looking in parent Flow " + intToString(parentFlowId) +"\n");
    return (dynamic_cast <STFlow*> (symTab.lookup(parentFlowId)))->checkLink(name, id, isSource);
  }
  else
  {
    trace("ERROR: Name of undefined Link is \"" 
		   + name + "\"\n\n");
    error();
  }

  return NULL;
}

/*!
 * checks if all the Flow's Links are used exactly one time as a Source and Target (resp.)
 */
void STFlow::checkLinkUsage()
{
  bool problems = false;
  if (!links.empty())
  {
    for (list<STLink *>::iterator iter = links.begin(); iter != links.end(); iter++)
    {
      if (!((*iter)->sourceId > 0 && (*iter)->targetId > 0))
      {
	problems = true;
        if ( (*iter)->sourceId > 0 )
	{
	  trace("\n");
          trace("ERROR: The Link \"" + symTab.readAttributeValue((*iter)->entryKey, "name") + "\" defined in line " 
		  + intToString(symTab.readAttribute((*iter)->entryKey, "name")->line) + "\n");
	  trace("       was used as source (line " + 
	        intToString(symTab.readAttribute((*iter)->sourceId, "linkName")->line) + "), but never as a target!\n\n");
	}
	else if ( (*iter)->targetId > 0 )
	{
	  trace("\n");
          trace("ERROR: The Link \"" + symTab.readAttributeValue((*iter)->entryKey, "name") + "\" defined in line " 
		  + intToString(symTab.readAttribute((*iter)->entryKey, "name")->line) + "\n");
	  trace("       was used as target (line " + 
	        intToString(symTab.readAttribute((*iter)->targetId, "linkName")->line) + "), but never as a source!\n\n");
	}
	else
	{
	  trace("\n");
          trace("ERROR: The Link \"" + symTab.readAttributeValue((*iter)->entryKey, "name") + "\" defined in line " 
		  + intToString(symTab.readAttribute((*iter)->entryKey, "name")->line) + "\n");
	  trace("       has never been used!\n\n");
	}
      }
      if (problems)
      {
	error ();
      }
    }
  }
}




  
/********************************************
 * implementation of SourceTarget CLASS
 ********************************************/

/*!
 * constructor
 */
STSourceTarget::STSourceTarget(unsigned int elementId, unsigned int entryKey)
 :SymbolTableEntry(elementId, entryKey) 
{
  link = NULL;
}

/*!
 * destructor
 */
STSourceTarget::~STSourceTarget() {}




std::string channelName(std::string myportType, std::string myoperation, std::string mypartnerLink)
{
  string longName = mypartnerLink + "." + myportType + "." + myoperation;
  string result = "";
  if ( channelShortNames[myoperation] == "" || channelShortNames[myoperation] == longName )
  {
    channelShortNames[myoperation] = longName;
    result = myoperation;
  }
  else
  {
    int i = 1;
    while (( channelShortNames[myoperation + "_" + intToString(i++)] != "" ));
    channelShortNames[myoperation + "_" + intToString(i)] = longName;
    result = myoperation + "_" + intToString(i);
  }
  return result;
  // return mypartnerLink + "." + myportType + "." + myoperation;
}

