/**
 * Purpose:
 *  Simulate receiving artifact messages 
 *  Uses rfd900Modem class to receive
 *  
 * 
 *  Default serial path is /dev/ttyUSB0
 *  Default baud rate is 57600
 *  
 * Optional Command line arguments
 *  argv[1] - loopCount
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


void parse_command_line(char **argv, int* loopCount)
{
    *loopCount = atoi(argv[1]);
}


int main(int argc, char **argv)
{
     // signal handling
    struct sigaction saint;             // SIGINT caused by ctrl + c

    // serial
    std::string serialDevicePath = "/dev/ttyUSB0";
    int baudRate = rfd900comm::rfd900Modem::DEFAULT_BAUD_RATE;
    rfd900comm::rfd900Modem radio;

    // comm node identification
    uint8_t myCommId = rfd900sim::SimConstants::BASE_STATION;

    // allocate serial buffer
    //uint8_t serial_tx_buffer[SERIAL_ARTIFACT_BUFFER_LENGTH];
    uint8_t serial_rx_buffer[SERIAL_RX_BUFFER_LENGTH];

    std::string temp_rx_storage;
    std::string extracted_rx_data;
         

    ssize_t bytesRead;

    // 900 MHz message tracking
    rfd900comm::message900 msg900;

    int rxcount = 0;
    int loopCount = 0;

    if(argc < 2){
        fprintf(stderr, "usage: %s <loopCount>\n", argv[0]);
        return 1;
    }

    parse_command_line(argv, &loopCount);

    // initialize radio serial connection
    if( radio.init(serialDevicePath.c_str(), baudRate) != 0){
        fprintf(stderr, "error, %s radio init failure, serialDevicePath: %s\n", __func__,
            serialDevicePath.c_str());
        return 1;
    }


    // register the SIGINT signal handler function
    memset(&saint, 0, sizeof(saint));
    saint.sa_handler = signal_handler_term;
    if(sigaction(SIGINT, &saint, NULL) < 0){
        fprintf(stderr, "sigaction saint, errno: %s", strerror(errno));
        return 1;
    }

    while(rxcount < loopCount && exitRequest == 0){
       
        // receive any messages
        bytesRead = radio.read_serial(serial_rx_buffer, SERIAL_RX_BUFFER_LENGTH, 10000000 );
        //fprintf(stderr, "bytesRead: %lu\n", bytesRead);
        if(bytesRead > 0){
            temp_rx_storage.append((const char*)serial_rx_buffer, bytesRead);
            
            extracted_rx_data.clear();

            if(rfd900sim::extract_rx_message(temp_rx_storage, extracted_rx_data)){
                ++rxcount;
                
                if(extracted_rx_data[0] == myCommId){
                    rfd900sim::ack_message_t ackmsg;
                    if( rfd900sim::process_rx_message(extracted_rx_data, &ackmsg, true) == rfd900sim::SimConstants::SEND_ACK){
                        //fprintf(stderr, "%s, SEND_ACK returned\n", __func__);
                    }
                    else{
                        fprintf(stderr, "%s, NO_ACK returned\n", __func__);
                    }
                }
                else{
                    fprintf(stderr, "Message is NOT for me, dest_id: %hhu, myCommId: %hhu\n", extracted_rx_data[0], myCommId);
                    fprintf(stderr, "discarding the data");
                }
            } 
        }
        else if(bytesRead < 0){
            fprintf(stderr, "bytesRead: %ld, loop terminating\n", bytesRead);
            break;
        }
    }


    fprintf(stderr, "program terminating, rxcount: %d\n", rxcount);

    return 0;

}