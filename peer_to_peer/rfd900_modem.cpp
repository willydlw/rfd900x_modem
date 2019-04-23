/**
 * @brief rfd900Modem class function definitions.
 * 
 * The rfd900x modem hardware requires a serial interface. 
 * 
 * The class implements functions to
 *      initialize the serial port connection
 *      read from the serial port
 *      write to the serial port
 *      close the serial connection
 * 
 * Author: Diane Williams
 * Date: 3/29/2019
 * 
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>                     // memset
#include <termios.h>
#include <unistd.h>

#include <sys/time.h>


#include "rfd900_modem.h"

namespace marblecomm{

    /**
     * \fn rfd900Modem(const char* devicePath)
     * 
     * \param[in] devicePath - serial device path, example: "/dev/ttyUSB0"
     * \param[in] baud
     * 
     */
    rfd900Modem::rfd900Modem()
    {
        baudRate = 0;
        serialfd = -1;
    }

    rfd900Modem::~rfd900Modem()
    {
        if(serialfd != -1){
            close_serial();
        }
    }

    int rfd900Modem::init(const char* devicePath, int baud_rate){
        
        serialDeviceName = devicePath;
        serialfd = initialize_serial(baud_rate);
        if(serialfd == -1){
            return -1;
        }

        return 0;
    }


   

    /**
    *\fn int initialize_serial(int baud_rate)
    *
    *\param[in]
    *   	serial_device_name - example: "/dev/ttyS1"
    *   	baud_rate - baud rate in bits per second
    *
    *\return
    *       Success - returns the serial port file descriptor.
    *       Failure - returns -1
    *
    *
    */
    int rfd900Modem::initialize_serial(int baud_rate)
    {
        int serial_port_fd;
        int serial_speed = set_baud_speed(baud_rate);
        struct termios newtio;

        // Open the serial port nonblocking (read returns immediately)
        serial_port_fd = open(serialDeviceName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if(serial_port_fd < 0)             // open returns -1 on error
        {
            fprintf(stderr, "Error: %s, serial port not open, errno -%s\n", __FUNCTION__, strerror(errno));
            fprintf(stderr, "serial device name: %s\n", serialDeviceName.c_str());
            return -1;
        }

        else    fprintf(stderr, "success, open serial port fd = %d\n", serial_port_fd);


        // New port settings
        memset(&newtio, 0, sizeof(newtio));           // set all struct values to zero
        /* Set Control mode
        *  CS8      - 8 data bits, no parity, 1 stop bit
        *  CLOCAL   - local connnection, no modem control
        *  CREAD    - enable receiving characters
        *  IGNBRK   - ignore break condition
        */
        newtio.c_cflag = serial_speed | CS8 | CLOCAL | CREAD; //  | IGNBRK;

        /* IGNPAR - ignore bytes with parity errors
        *  ICRNL -  map CR to NL (otherwise a CR input on the other computer
                        will not terminate input)
        *
        */
        newtio.c_iflag = IGNPAR | ICRNL;
        newtio.c_oflag = 0;                     // raw output
        /*
        * ICANON  : enable canonical input
        *           disable all echo functionality, and don't send signals to calling program
        */
        newtio.c_lflag = ~ICANON;
        newtio.c_cc[VMIN] = 1;
        newtio.c_cc[VTIME] = 0;

        // Load new settings
        if( tcsetattr(serial_port_fd, TCSAFLUSH, &newtio) < 0){
            fprintf(stderr, "error, %s, set term attributes: %s\n", __func__, strerror(errno));
            close_serial();
            return -1;
        }

        // clear data from both input/output buffers
        /* When using a usb serial port, the USB driver does not
        know if there is data in the internal shift register, FIFO
        or USB subsystem. As a workaround, to ensure the data
        is flushed, add a sleep delay here to suspend program
        execution. This allows time for data to arrive and be
        stored in the buffers. A call to flush will then work.

        May need to experiment with sleep time.
        */
        usleep(10000);                                  // 10 ms
        tcflush(serial_port_fd, TCIOFLUSH);

        fprintf(stderr, "info, %s success\n", __func__);

        return serial_port_fd;
    }


    int  rfd900Modem::set_baud_speed(int baud_rate)
    {
        switch(baud_rate)
        {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        default:
            fprintf(stderr, "warning: %s, argument baud_rate: %d not supported, setting to default %d\n",
                        __func__, baud_rate, DEFAULT_BAUD_RATE);
            return DEFAULT_BAUD_RATE;
        }
    }



    ssize_t rfd900Modem::read_serial(uint8_t* readbuffer, size_t numbytes, long int delay_time )
    {
        /*  The GNU C library provides two data types specifically for representing an elapsed time.
            Data Type: struct timeval

            The struct timeval structure represents an elapsed time. It is declared in sys/time.h and has the following members:

            long int tv_sec     This represents the number of whole seconds of elapsed time.
            long int tv_usec    This is the rest of the elapsed time (a fraction of a second),
                                represented as the number of microseconds. It is always less than one million.

    */

        int rv;                                             // return value
        struct timeval  timeout;
        fd_set read_set;                                    // file descriptor set

        timeout.tv_sec = delay_time / 1000000L;             // one sec in units of microseconds
        timeout.tv_usec = delay_time % 1000000L;


        /*  void FD_SET(int fd, fd_set *set);
            void FD_ZERO(fd_set *set);
        */
        FD_ZERO (&read_set);                       // clears the set
        FD_SET(serialfd, &read_set);         // adds the file descriptor to the set

        /*  select() allows a program to monitor multiple file descriptors,
            waiting until one or more of the file descriptors become "ready" for
            some class of I/O operation (e.g., input possible). A file descriptor is
            considered ready if it is possible to perform the corresponding I/O operation
            (e.g., read(2)) without blocking.

            int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
        */

        rv = select(serialfd+1, &read_set, NULL, NULL, &timeout);

        if(rv > 0){
            if(FD_ISSET(serialfd, &read_set))    // input from source available
            {
                    /* ssize_t read(int fd, void *buf, size_t count);
                    read attempts to read count bytes from file descriptor fd into the buffer starting at buf

                    On success, the number of bytes read is returned.
                    On error, -1 is returned and errno is set appropriately
                    */

                    memset(readbuffer, 0, numbytes);
                    return read(serialfd,readbuffer,numbytes);
            }
            else{
                // Because our file descriptor was the only one in the read set, it is highly unlikey that
                // select would return a value of 1 and our FD_ISSET would return 0
                fprintf(stderr, "warn: %s, select returned %d, but FD_ISSET was false, serialfd: %d\n",
                        __func__, rv, serialfd);

            }
        }
        else if(rv < 0){
                /*  Errors
                EBADF - An invalid file descriptor was given in one of the sets.
                (Perhaps a file descriptor that was already closed, or one on which an error has occurred.)

                EINTR - A signal was caught.
                EINVAL - nfds is negative or the value contained within timeout is invalid.
                ENOMEM - unable to allocate memory for internal tables.
                */
                fprintf(stderr, "%s, error on select - %s\n", __FUNCTION__, strerror(errno));
                fprintf(stderr, "select return value: %d\n", rv);
                return -1;
        }
        /* else select returned 0 because it timed out and there was nothing to read */

        return 0;

    }

    /**
    *\fn ssize_t rfd900Modem::send_message(const void* msg, size_t length)
    *
    *\param[in]
    *   	msg - message bytes 
    *   	length - number of bytes to be sent
    *
    *\return
    *       returns number of bytes sent
    *
    */
    ssize_t rfd900Modem::send_message(const char* msg, size_t length)
     {
         ssize_t totalBytesSent = 0;
         ssize_t bytesSent;
         ssize_t bytesRemaining = length - totalBytesSent;

         while(bytesRemaining > 0){
             bytesSent = write(serialfd, &msg[totalBytesSent], bytesRemaining);

             if(bytesSent >= 0){
                 totalBytesSent += bytesSent;
             }
             else{
                 fprintf(stderr, "error: %s, bytesSent: %ld, errno: %s\n", __func__, bytesSent, strerror(errno));
                 break;
             }

             bytesRemaining = length - totalBytesSent;
         }

         return totalBytesSent;
     }



    void rfd900Modem::close_serial(){

        if( close(serialfd) != -1){
                serialfd = -1;
        }
        else
        {
            fprintf(stderr, "%s, serial close error: %s\n", __func__, strerror(errno));
        }
        
    }

}
