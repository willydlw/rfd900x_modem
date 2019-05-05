/**
 * Purpose:
 *  Uses rfd900Modem class to transmit the message hello + count
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


int main(int argc, char **argv)
{
    std::string serialDevicePath;
    int baudRate = 57600;
    marblecomm::rfd900Modem radio;

    int count = 0;

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

    while(count < 10){
        std::stringstream ss;
        std::string message;
        ss << "hello " << count << "\n";
        message = ss.str();
        radio.send_message(message.c_str(), message.length());
        fprintf(stdout, "sending message: %s\n", message.c_str());
        ++count;
        sleep(1);
    }
    
    return 0;

}