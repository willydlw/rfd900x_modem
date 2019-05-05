/**
 * @brief Declares rfd900Modem class for rfd900x modem radios
 * 
 * The rfd900x modem hardware requires a serial interface. 
 * 
 * The class implements functions to
 *      initialize the serial port connection
 *      read from the serial port
 *      write to the serial port
 *      close the serial connection
 * 
 * 
 * Author: Diane Williams
 * Date: 3/29/2019
 * 
 */


#ifndef RFD900_MODEM_INCLUDED_H
#define RFD900_MODEM_INCLUDED_H

#include <cstdint>          // uint8_t
#include <string>           // std::string


namespace rfd900comm{
    class rfd900Modem{

        public:
        
        static constexpr int DEFAULT_BAUD_RATE = 57600;

        public:

        rfd900Modem();                      // constructor
        ~rfd900Modem();                     // destructor

        // disable copy constructor
        rfd900Modem(const rfd900Modem&) = delete;

        // disable assignment
        rfd900Modem& operator=(const rfd900Modem&) = delete;

         

        int init(const char* devicePath = "/dev/ttyUSB0", int baud_rate = DEFAULT_BAUD_RATE);

        ssize_t send_message(const char* msg, size_t length);

        ssize_t read_serial(uint8_t* readbuffer, size_t numbytes, long int delay_time);



        private:

        int serialfd;               // serial file descriptor
        int baudRate;
        std::string serialDeviceName;


        // serial functions
        int initialize_serial(int baud_rate);
        int set_baud_speed(int baud_rate);
        void close_serial();
        
    };
}


#endif