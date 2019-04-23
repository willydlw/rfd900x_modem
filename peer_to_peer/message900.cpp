#include <cstdio>                   // fprintf
#include <cstring>                  // strlen
#include <cstdlib>                  // malloc, free
#include "message900.h"

namespace marblecomm{

    message900::message900()
    {
        // intentionally blank
    }

    message900::~message900()
    {
        empty_ack_wait_list();
    }
    
    void message900::empty_ack_wait_list()
    {
        while(!ack_wait_list.empty()){
            free(ack_wait_list.front().data);
            ack_wait_list.pop_front();
        }

    }

    int message900::add_to_ack_wait_list(uint8_t dest_id, uint16_t msg_id, uint8_t msg_type, const uint8_t* txdata, size_t txdata_length)
    {
        message900_t msg900;
        msg900.dest_id = dest_id;
        msg900.message_id = msg_id;
        msg900.message_type = msg_type;
        msg900.data = (uint8_t*)malloc(txdata_length*sizeof(uint8_t));
        if(msg900.data == nullptr){
            fprintf(stderr, "error, %s malloc failure\n", __func__);
            return -1;
        }

        memcpy(msg900.data, txdata, txdata_length);
        get_timestamp(&msg900.sec, &msg900.nsec);                 // record transmit time
        ack_wait_list.push_back(msg900);
        return 0;
    }

   

    void message900::process_received_ack()
    {
        fprintf(stderr, "\nerror: %s incomplete\n", __func__);
    }


     void message900::scan_list_for_retransmission()
     {
         fprintf(stderr, "\nerror: %s incomplete\n", __func__);
     }

     // returns time since epoch
    void message900::get_timestamp(uint64_t* sec, uint64_t* nsec){
    
        *nsec = std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        *sec = *nsec / 1000000000UL;
        *nsec = *nsec - (*sec * 1000000000UL);
    }
    
}
