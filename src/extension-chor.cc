/*****************************************************************************\
 * Copyright 2007  Niels Lohmann                                             *
 *                                                                           *
 * This file is part of GNU BPEL2oWFN.                                       *
 *                                                                           *
 * GNU BPEL2oWFN is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or (at your    *
 * option) any later version.                                                *
 *                                                                           *
 * GNU BPEL2oWFN is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  *
 * Public License for more details.                                          *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with GNU BPEL2oWFN; see file COPYING. if not, write to the Free Software  *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA. *
\*****************************************************************************/

/*!
 * \file    extension-chor.cc
 *
 * \brief   BPEL4Chor extension
 *  
 * \author  Niels Lohmann <nlohmann@informatik.hu-berlin.de>,
 *          last changes of: \$Author: nielslohmann $
 *
 * \since   2007/04/30
 *
 * \date    \$Date: 2007/05/08 16:11:41 $
 * 
 * \note    This file is part of the tool BPEL2oWFN and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 * \version \$Revision: 1.13 $
 */





#include <iostream>

#include "extension-chor.h"
#include "debug.h"
#include "helpers.h"
#include "globals.h"
#include "ast-details.h"


extern int frontend_lineno;





BPEL4Chor_participantSet::BPEL4Chor_participantSet(map<string, string> &attribute_map)
{
  // read attributes
  name = attribute_map["name"];
  forEach_name = attribute_map["forEach"];
  participantType_name = attribute_map["type"];
  count = toUInt(attribute_map["count"]);

  // if participantType is not found, display an error
  if (participantType_name == "")
    genericError(135, "BPEL4Chor <participantType> `" + participantType_name +
	"' referenced by <participantSet> `" + name + "' not defined before",
	toString(frontend_lineno), ERRORLEVER_WARNING);

  // warn if no count is given
  if (count == UINT_MAX)
    std::cerr << "no count given" << std::endl;
}





BPEL4Chor_messageLink::BPEL4Chor_messageLink(map<string, string> &attribute_map)
{
  name = attribute_map["name"];
  sender = attribute_map["sender"];
  receiver = attribute_map["receiver"];
  messageName = attribute_map["messageName"];

  id = (attribute_map["name"] != "") ?
    attribute_map["name"] :
    attribute_map["messageName"];

  sendActivity = (attribute_map["sendActivity"] != "") ?
    attribute_map["sendActivity"] :
    attribute_map["sendActivities"];

  receiveActivity = (attribute_map["receiveActivity"] != "") ?
    attribute_map["receiveActivity"] :
    attribute_map["receiveActivities"];

  participantSet = NULL;
  for (map<string, BPEL4Chor_participantSet*>::const_iterator participantSet_it = globals::BPEL4ChorInfo.participantSets.begin();
      participantSet_it != globals::BPEL4ChorInfo.participantSets.end(); participantSet_it++)
  {
    if (participantSet_it->second->iterator_participant_names.find(receiver) != participantSet_it->second->iterator_participant_names.end() ||
	participantSet_it->second->iterator_participant_names.find(sender) != participantSet_it->second->iterator_participant_names.end())
      participantSet = participantSet_it->second;
  }
}





/*!
 * \param  attribute_map  an attribute mapping provided by the BPEL4Chor parser
 */
void BPEL4Chor::add_participantType(map<string, string> &attribute_map)
{
  string participantType_name = attribute_map["name"];
  string participantBehaviorDescription_name = attribute_map["participantBehaviorDescription"];

  // if participant type was already defined before, display a warning
  if (participantTypes[participantType_name] != "")
    genericError(134, participantType_name, toString(frontend_lineno), ERRORLEVER_WARNING);

  participantTypes[participantType_name] = participantBehaviorDescription_name;
}





/*!
 * \param  attribute_map  an attribute mapping provided by the BPEL4Chor parser
 */
void BPEL4Chor::add_participant(map<string, string> &attribute_map)
{
  // add the participant to the current participantSet if present
  if (current_participantSet != NULL)
  {
    if (attribute_map["forEach"] != "")
      current_participantSet->iterator_participant_names.insert(attribute_map["name"]);
    else
      current_participantSet->unique_participant_names.insert(attribute_map["name"]);
  }

  // reset attributes
  attribute_map["type"] = "";
  attribute_map["name"] = "";
  attribute_map["forEach"] = "";
  attribute_map["count"] = "";
}





/*!
 * \param  attribute_map  an attribute mapping provided by the BPEL4Chor parser
 */
void BPEL4Chor::start_participantSet(map<string, string> &attribute_map)
{
  current_participantSet = new BPEL4Chor_participantSet(attribute_map);
  participantSets[attribute_map["name"]] = current_participantSet;
}





void BPEL4Chor::end_participantSet()
{
  current_participantSet = NULL;
}





/*!
 * \param  attribute_map  an attribute mapping provided by the BPEL4Chor parser
 */
void BPEL4Chor::add_messageLink(map<string, string> &attribute_map)
{
  BPEL4Chor_messageLink *temp = new BPEL4Chor_messageLink(attribute_map);
  messageLinks[temp->id] = temp;
}





/*!
 * \param  ASTE_id  the AST id of an activity
 *
 * \return the name of the message link that can be used to create a channel
 *         name
 */
string BPEL4Chor::channel_name(unsigned int ASTE_id) const
{
  assert(globals::ASTEmap[ASTE_id] != NULL);

  string activity_name = (globals::ASTEmap[ASTE_id]->attributes["id"] != "") ?
    globals::ASTEmap[ASTE_id]->attributes["id"] :
    globals::ASTEmap[ASTE_id]->attributes["name"];

  // if no activity name is given, display an error
  if (activity_name == "")
  {
    genericError(132, globals::ASTEmap[ASTE_id]->activityTypeName(),
	globals::ASTEmap[ASTE_id]->attributes["referenceLine"], ERRORLEVEL_ERROR);
    return "";
  }

  // search for the message link
  for (map<string, BPEL4Chor_messageLink*>::const_iterator messageLink = messageLinks.begin();
      messageLink != messageLinks.end(); messageLink++)
  {
    if (messageLink->second->sendActivity == activity_name)
      return messageLink->first;

    if (messageLink->second->receiveActivity == activity_name)
      return messageLink->first;
  }

  // if no message link was found, display an error
  genericError(131, "activity id or name `" + activity_name + "' of <" +
      globals::ASTEmap[ASTE_id]->activityTypeName() + "> does not reference a BPEL4Chor <messageLink>",
      globals::ASTEmap[ASTE_id]->attributes["referenceLine"], ERRORLEVEL_ERROR);
  return "";
}





/*!
 * \param  ASTE_id  the AST id of an activity
 *
 * \return maximal iterations
 */
unsigned int BPEL4Chor::forEach_count(unsigned int ASTE_id) const
{
  assert(globals::ASTEmap[ASTE_id] != NULL);

  // find the forEach's name
  string forEach_name = (globals::ASTEmap[ASTE_id]->attributes["id"] != "") ?
    globals::ASTEmap[ASTE_id]->attributes["id"] :
    globals::ASTEmap[ASTE_id]->attributes["name"];

  // find the participantSet using the given forEach
  BPEL4Chor_participantSet *participantSet = NULL;
  for (map<string, BPEL4Chor_participantSet*>::const_iterator participantSet_it = participantSets.begin();
      participantSet_it != participantSets.end(); participantSet_it++)
  {
    assert(participantSet_it->second != NULL);

    if (participantSet_it->second->forEach_name == forEach_name)
    {
      participantSet = participantSet_it->second;
      break;
    }      
  }

  // if the participantSet is not found, display a warning
  if (participantSet == NULL)
  {
    genericError(136, forEach_name, globals::ASTEmap[ASTE_id]->attributes["referenceLine"], ERRORLEVER_WARNING);
    return UINT_MAX;
  }

  return participantSet->count;
}





unsigned int BPEL4Chor::channel_count(unsigned int ASTE_id, bool sending) const
{
  string temp_channel_name = channel_name(ASTE_id);
  BPEL4Chor_messageLink* messageLink = (messageLinks.find(temp_channel_name) != messageLinks.end()) ?
    messageLinks.find(temp_channel_name)->second :
    NULL;

  // if messageLink was not found, display an error
  if (messageLink == NULL)
  {
    std::cerr << "messageLink not found" << std::endl;
    return 0;
  }

  if (messageLink->participantSet == NULL)
    return 0;

  // activity is sending messages to a receiver nested in a forEach
  if (sending && (messageLink->participantSet->iterator_participant_names.find(messageLink->receiver) != messageLink->participantSet->iterator_participant_names.end()))
    return messageLink->participantSet->count;

  // activity is receiving messages from a sender nested in a forEach
  if (!sending && (messageLink->participantSet->iterator_participant_names.find(messageLink->sender) != messageLink->participantSet->iterator_participant_names.end()))
    return messageLink->participantSet->count;

  return 0;
}





void BPEL4Chor::print_information() const
{
  std::cerr << messageLinks.size() << " message links, " <<
    participantTypes.size() << " participant types, " <<
    participantSets.size() << " participantSets" << std::endl;
}





BPEL4Chor::BPEL4Chor() :
  current_participantSet(NULL)
{
}
