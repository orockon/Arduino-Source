/*  Nintendo Switch Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Navigation_H
#define PokemonAutomation_NintendoSwitch_Navigation_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast);

//Joycon must not be sideways
void home_to_date_time(JoyconContext& context, bool to_date_change);



}
}
#endif
