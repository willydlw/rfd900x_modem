/**
 * Purpose:
 *  Simulate sending artifact messages to a second radio
 *  Uses rfd900Modem class to transmit
 *  
 * 
 *  Default serial path is /dev/ttyUSB0
 *  Default baud rate is 57600
 *  
 * Required Command line arguments
 *  argv[1] - number of loop iterations
 *  argv[2] - time period between transmission
 * 
 * Author: Diane Williams
 * Date: 4/7/2019
 * 
 */

#include <signal.h>
#include <cstring>

#include <string>
#include <sstream>
#include <unistd.h>             // sleep
#include <chrono>               



#include "rfd900_modem.h"
#include "message900.h"
#include "simulation_constants.h"
#include "sim_artifact_message.h"

#define SERIAL_RX_BUFFER_LENGTH  256


static volatile sig_atomic_t exitRequest = 0;


static void signal_handler_term(int sig)
{
    /* psignal used for debugging purposes
       debugging console output lets us know this function was
       triggered. Prints the string message and a string for the
       variable sig
    */
    psignal(sig, "signal_handler_term");
    if(sig == SIGINT || sig == SIGTERM){
        exitRequest = 1;
    }
    
}



void parse_command_line(char **argv, int* loopCount, int* tx_millis)
{
    *loopCount = atoi(argv[1]);
    *tx_millis = atoi(argv[2]);
}


int main(int argc, char **argv)
{
    // signal handling
    struct sigaction saint;             // SIGINT caused by ctrl + c


    // constant buffer lengths
    const size_t SERIAL_ARTIFACT_BUFFER_LENGTH = sizeof(rfd900sim::artifact_message_t)
                        + rfd900sim::SimConstants::MESSAGE_900_START_INDICATOR_LENGTH 
                        + rfd900sim::SimConstants::MESSAGE_900_END_INDICATOR_LENGTH;

    // serial
    std::string serialDevicePath = "/dev/ttyUSB0";
    int baudRate = rfd900comm::rfd900Modem::DEFAULT_BAUD_RATE;
    rfd900comm::rfd900Modem radio;

    ssize_t bytesSent;

    // comm node identification
    uint8_t myCommId = rfd900sim::SimConstants::AERIAL01;

    // allocate serial buffer
    uint8_t serial_tx_buffer[SERIAL_ARTIFACT_BUFFER_LENGTH];

    // 900 MHz message tracking
    rfd900comm::message900 msg900;

    // milliseconds between transmission
    int tx_milliseconds = 1000;

    int txcount = 0;                            // number of messages transmitted
    int loopCount;

    // timing
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;

    if(argc < 3){
       fprintf(stderr, "usage: %s <loop iterations> <milliseconds between transmission>\n", argv[0]);
       return 1;
    }

    parse_command_line(argv, &loopCount, &tx_milliseconds);

    // 10 bits per byte: 1 start bit, 8 data bits, 1 stop bit
    fprintf(stderr, "SERIAL_ARTIFACT_BUFFER_LENGTH: %lu\n", SERIAL_ARTIFACT_BUFFER_LENGTH);
    fprintf(stderr, "Max bytes per second: %d\n", baudRate/10);
    fprintf(stderr, "Max messages per second: %lu\n", (baudRate/10)/SERIAL_ARTIFACT_BUFFER_LENGTH);

    // initialize radio serial connection
    if( radio.init(serialDevicePath.c_str(), baudRate) != 0){
        fprintf(stderr, "error, %s radio init failure\n", __func__);
        return 1;
    }
    

    // register the SIGINT signal handler function
    memset(&saint, 0, sizeof(saint));
    saint.sa_handler = signal_handler_term;
    if(sigaction(SIGINT, &saint, NULL) < 0){
        fprintf(stderr, "sigaction saint, errno: %s", strerror(errno));
        return 1;
    }
    

    while(txcount < loopCount && exitRequest == 0){

        start = std::chrono::steady_clock::now();

        rfd900sim::artifact_message_t artmsg;

        simulate_artifact_message(&artmsg, rfd900sim::SimConstants::BASE_STATION, myCommId);
        serialize_artifact_for_900MHz(&artmsg, serial_tx_buffer, SERIAL_ARTIFACT_BUFFER_LENGTH);
        
        bytesSent = radio.send_message((const char*)serial_tx_buffer, SERIAL_ARTIFACT_BUFFER_LENGTH);
        if(bytesSent == -1){
            fprintf(stderr, "error, %s, bytesSent: %ld\n", __func__, bytesSent);
            break;
        }
        
         ++txcount;
        // print every so often to inform user of progress
        if( (txcount % 25) == 0){
            fprintf(stdout, "sending message number %d\n", txcount);
        }
        
        end = std::chrono::steady_clock::now();
        diff = end - start;
        while(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() < tx_milliseconds){
            end = std::chrono::steady_clock::now();
            diff = end - start;
        }

    }

    // do not close serial connection immediately as all data may not have been transmitted
    // at end of loop
    start = std::chrono::steady_clock::now();
    end = std::chrono::steady_clock::now();
    diff = end - start;
    while(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() < 3000){
        end = std::chrono::steady_clock::now();
        diff = end - start;
    }

    
    fprintf(stderr, "program terminating, tx_count: %d\n", txcount);
    return 0;

}