#ifndef MESSAGE_RFD900_INCLUDED_H
#define MESSAGE_RFD900_INCLUDED_H


#include <chrono>
#include <cstdint>
#include <list>
#include <string>


namespace marblecomm{

     struct message900_t{
        //timestamp_t tx_time;                // time message was transmitted
        uint64_t sec;
        uint64_t nsec;
        uint8_t dest_id;
        uint16_t message_id;
        uint8_t message_type;               // to be determined if this is needed
        uint8_t *data;
    };

    class message900{
        public:

        static constexpr auto retransmission_interval = std::chrono::seconds(3);

        message900();
        ~message900();


        int add_to_ack_wait_list(uint8_t dest_id, uint16_t msg_id, uint8_t msg_type, const uint8_t* txdata, size_t txdata_length);
        void process_received_ack();
        void scan_list_for_retransmission();

        


        private:

        // list to store transmitted messages
        std::list<marblecomm::message900_t> ack_wait_list;

        void empty_ack_wait_list();
        void get_timestamp(uint64_t* sec, uint64_t* nsec);


    };
   

    
}



#endif 