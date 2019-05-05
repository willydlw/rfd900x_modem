/**
 * Purpose:
 *  Uses rfd900Modem class to read any incoming messages.
 *  Displays messages as they are read
 * 
 *  Default serial path is /dev/ttyUSB0
 *  Default baud rate is 57600
 *  
 * Optional Command line arguments
 *  argv[1] - serial device path such as /dev/ttys0
 * 
 * Author: Diane Williams
 * Date: 3/29/2019
 * 
 */

#include <string>
#include <sstream>
#include <unistd.h>             // sleep

#include "rfd900_modem.h"


constexpr int LOOP_ITERATIONS = 10;
constexpr int READ_BUFFER_LENGTH = 1024;
constexpr long READ_TIMEOUT = 1000000L;         // units, microseconds


int main(int argc, char **argv)
{
    std::string serialDevicePath;
    int baudRate = 57600;
    rfd900comm::rfd900Modem radio;

    int messageCount = 0;

    uint8_t readBuffer[READ_BUFFER_LENGTH];
    ssize_t bytesRead;

    if(argc < 2){
        serialDevicePath = "/dev/ttyUSB0";
    }
    else{
        serialDevicePath = argv[1];
    }

    if( radio.init(serialDevicePath.c_str(), baudRate) != 0){
        fprintf(stderr, "error, %s radio init failure\n", __func__);
        return 1;
    }

    while(messageCount < LOOP_ITERATIONS){
        bytesRead = radio.read_serial(readBuffer, READ_BUFFER_LENGTH, READ_TIMEOUT);
        fprintf(stdout, "bytesRead: %ld", bytesRead);
        if(bytesRead > 0){
            fprintf(stdout, ", message: %s\n", readBuffer);
            ++messageCount;
        }
        else if(bytesRead == 0){
            fprintf(stdout, "\n");
        }
        else{
            fprintf(stdout, ", terminating due to read error\n");
            break;
        }
    }
    
    return 0;

}