/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FastCodeEntry_H
#define PokemonAutomation_NintendoSwitch_FastCodeEntry_H

#include <stdint.h>
#include <string>
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{


using namespace std::chrono_literals;



enum class KeyboardLayout{
    QWERTY,
    AZERTY
};

class FastCodeEntrySettingsOption : public GroupOption{
public:
    FastCodeEntrySettingsOption(LockMode lock_while_program_is_running);

public:
    EnumDropdownOption<KeyboardLayout> KEYBOARD_LAYOUT;
    BooleanCheckBoxOption SKIP_PLUS;

    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption DIGIT_REORDERING;
    MillisecondsOption SCROLL_DELAY0;
    MillisecondsOption WRAP_DELAY0;
};




//  Internals for testing only.

struct CodeboardPosition{
    uint8_t row;
    uint8_t col;
};
struct CodeboardScroll{
    DpadPosition direction;
    Milliseconds delay;
};
//static_assert(sizeof(CodeboardScroll) == sizeof(uint8_t));
struct DigitPath{
    uint8_t length = 0;
    bool left_cursor = false;
    CodeboardScroll path[14];
};
DigitPath get_codeboard_digit_path(
    CodeboardPosition source, CodeboardPosition destination,
    uint8_t scroll_delay, uint8_t wrap_delay, bool reordering
);
void move_codeboard(SwitchControllerContext& context, const DigitPath& path);



//  This is the one you want to call.

struct FastCodeEntrySettings{
    KeyboardLayout keyboard_layout = KeyboardLayout::QWERTY;
    bool include_plus = true;
    Milliseconds scroll_delay = 64ms;
    Milliseconds wrap_delay = 64ms;
    bool digit_reordering = false;

    FastCodeEntrySettings() = default;
    FastCodeEntrySettings(const FastCodeEntrySettingsOption& option);
};
void enter_alphanumeric_code(
    Logger& logger,
    SwitchControllerContext& context,
    const FastCodeEntrySettings& settings,
    const std::string& code
);











}
}
#endif
