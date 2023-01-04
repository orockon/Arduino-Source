/*  Tera Code Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/AbstractLogger.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "PokemonSV_TeraCodeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


std::string read_raid_code(Logger& logger, const ImageViewRGB32& image){
    using namespace Kernels::Waterfill;

    //  Direct OCR is unreliable. Instead, we will waterfill each character
    //  to isolate them, then OCR them individually.

    uint32_t THRESHOLD = 0xff5f5f5f;

    ImageRGB32 filtered = to_blackwhite_rgb32_range(image, 0xff000000, THRESHOLD, true);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000000, THRESHOLD);

    std::map<size_t, std::string> map;
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(50);
        WaterfillObject object;
        while (iter->find_next(object, false)){
//                cout << object.packed_matrix()->dump() << endl;
            ImageViewRGB32 character = extract_box_reference(filtered, object);
            ImageRGB32 padded = pad_image(character, character.width(), 0xffffffff);

            std::string raw = OCR::ocr_read(Language::English, padded);
            map.emplace(object.min_x, std::move(raw));
        }
    }

//    cout << map.size() << endl;

    static const std::map<char, char> SUBSTITUTIONS{
        {'I', '1'},
        {'i', '1'},
        {'l', '1'},
        {'O', '0'},
        {'Z', 'S'},
        {'\\', 'V'},
        {'/', '7'},
        {']', '1'},
    };

    std::string raw;
    std::string normalized;
    for (const auto& item : map){
        if (item.second.empty()){
            continue;
        }
        if ((uint8_t)item.second.back() < (uint8_t)32){
            raw += item.second.substr(0, item.second.size() - 1);
        }else{
            raw += item.second;
        }

        char ch = item.second[0];

        //  Upper case.
        if ('a' <= ch && ch <= 'z'){
            ch -= 'a' - 'A';
        }

        //  Character substitution.
        auto iter = SUBSTITUTIONS.find(ch);
        if (iter != SUBSTITUTIONS.end()){
            ch = iter->second;
        }

        normalized += ch;
    }

    std::string log = "Code OCR: \"" + raw + "\" -> \"" + normalized + "\"";
    if (normalized.size() != 4 && normalized.size() != 6){
        logger.log(log, COLOR_RED);
        return "";
    }

    logger.log(log, COLOR_BLUE);
    return normalized;
}



}
}
}