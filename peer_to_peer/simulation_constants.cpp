#include "simulation_constants.h"

#include <cstring>          // strlen

namespace rfd900sim{


     // define const that are not constexpr
    const char* SimConstants::MESSAGE_900_START_INDICATOR = "<#@";
    const size_t SimConstants::MESSAGE_900_START_INDICATOR_LENGTH = strlen(MESSAGE_900_START_INDICATOR);


    const char* SimConstants::MESSAGE_900_END_INDICATOR = "@#>";
    const size_t SimConstants::MESSAGE_900_END_INDICATOR_LENGTH = strlen(MESSAGE_900_END_INDICATOR);


    SimConstants::SimConstants()
    { // intentionally blank 
    }
}