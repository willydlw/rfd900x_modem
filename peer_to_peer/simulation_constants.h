#ifndef SIMULATION_CONSTANTS_INCLUDED_H
#define SIMULATION_CONSTANTS_INCLUDED_H

#include <cstdint>
#include <cstdlib>                  // size_t


namespace rfd900sim {

    class SimConstants{
        public:
        
        // robot, communication node id
        static constexpr uint8_t GROUND01 = 0;
        static constexpr uint8_t AERIAL01 = 1;
        static constexpr uint8_t AERIAL02 = 2;
        static constexpr uint8_t ANCHOR_STATION = 3;
        static constexpr uint8_t BASE_STATION = 4;

        // Message Type constants
        static constexpr uint8_t ROBOT_POSITION = 0;
        static constexpr uint8_t ARTIFACT_POSITION = 1;
        static constexpr uint8_t ACK = 2;
        static constexpr uint8_t REPORT_TO_ANCHOR = 3;
        static constexpr uint8_t NO_ACK = 4;

        // artifact types
        static constexpr uint8_t SURVIVOR = 1;
        static constexpr uint8_t CELLPHONE = 2;
        static constexpr uint8_t BACKPACK = 3;
        static constexpr uint8_t DRILL = 4;        
        static constexpr uint8_t FIRE_EXTINGUISHER = 5;
       
        static constexpr int NUM_ARTIFACT_CATEGORIES = 5;

        // actions
        static constexpr int SEND_ACK = 1;


        // define const that are not constexpr
        static const char* MESSAGE_900_START_INDICATOR;
        static const size_t MESSAGE_900_START_INDICATOR_LENGTH;


        static const char* MESSAGE_900_END_INDICATOR;
        static const size_t MESSAGE_900_END_INDICATOR_LENGTH;


        private:
        SimConstants();
    };

}

#endif
