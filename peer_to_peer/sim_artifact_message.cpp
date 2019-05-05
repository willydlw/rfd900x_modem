#include <cstdlib>              // rand
#include <cstdio>
#include <cstring>              // memset, memcpy
#include <chrono>
#include <iostream>


#include "message900.h"
#include "sim_artifact_message.h"


namespace rfd900sim
{
    double random_double(int max ){
        double value;
        value = double(rand())/double(RAND_MAX);        // fractional portion
        value += double(rand() % max);                  // integer portion
        return value;
    }

    // returns time since epoch
    void get_timestamp(timestamp_t *ts){
    
        uint64_t nsec = std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        ts->sec = nsec / 1000000000UL;
        ts->nsec = nsec - (ts->sec * 1000000000UL);
    }


    void fill_header_message(header_msg_t  *hm)
    {
        static uint32_t seqID = 0;
        hm->seq = seqID;
        get_timestamp(&hm->stamp);
        hm->frame_id = "sim";
        ++seqID;
    }

    void random_point(point_t *p){
        p->x = random_double();
        p->y = random_double();
        p->z = random_double();
    } 


    void random_point_stamped_message(point_stamped_message_t *psm)
    {
        fill_header_message(&psm->hdr);
        random_point(&psm->p);

    }

    void print_point_stamped_message(const point_stamped_message_t *psm)
    {
        fprintf(stdout, "seq: %5d\nstamp\n\tsec: %lu\n\tnsec: %lu\nframe id: %s\n",
                    psm->hdr.seq, psm->hdr.stamp.sec, psm->hdr.stamp.nsec, psm->hdr.frame_id.c_str());
        
        fprintf(stdout, "point\n\tx: %12f\n\ty: %12f\n\tz: %12f\n", psm->p.x, psm->p.y, psm->p.z);
    }


    /**************** ARTIFACT POSITION MESSAGE FUNCTIONS ********************/
    void simulate_artifact_message(artifact_message_t *art, uint8_t dest, uint8_t src)
    {
        static uint16_t message_id = 0;
        
        art->msg_id = message_id;
        ++message_id;

        art->dest_id = dest;
        art->src_id = src;
        art->msg_type = SimConstants::ARTIFACT_POSITION;
        
        // randomly choose artifact and position
        art->artifact = static_cast<uint8_t>(rand() % SimConstants::NUM_ARTIFACT_CATEGORIES);
        random_point(&art->position);
        // record time 
        get_timestamp(&art->stamp);
    }

    void print_artifact_message(const artifact_message_t *art)
    {
        printf("To/From\n\tdest:   %4hhu, src: %4hhu\n", art->dest_id, art->src_id);
        printf("Message\n\ttype:   %4hhu, id: %5hu\n", art->msg_type, art->msg_id);
        printf("Artifact\n\tvalue:  %4hhu, string: %s\n", art->artifact, artifact_strings[art->artifact]);
        printf("Time\n\tsec:  %lu\n\tnsec: %lu\n", art->stamp.sec, art->stamp.nsec);
        printf("point\n\tx: %12f\n\ty: %12f\n\tz: %12f\n", art->position.x, art->position.y, art->position.z);
    }


    /**
     * 
     * 
     * 
     * Note: serial_buffer_length must include bytes for the message plus the start message bytes
     * SERIAL_ARTIFACT_BUFFER_LENGTH = SERIAL_ARTIFACT_MESSAGE_LENGTH + MESSAGE_900_START_INDICATOR_LENGTH
     * 
     * The entire contents of the artifact_message_t struct are copied into the message to be transmitted.
     * When a struct does not perfectly byte align on even numbered boundaries, there may be zero padded bytes
     * embedded in the struct. As this is a very short message, there are currentlye three extra zero-padded bytes
     * that will be transmitted.
     * 
     * The alternative version of this function is commented out below. It does not copy the zero-padded bytes,
     * but has the additional overhead of copying each field individually.
     * 
     * These artifact messages are not frequently transmitted, so the extra 3 byte overhead is deemed acceptable.
     */
    void serialize_artifact_for_900MHz(const artifact_message_t* art, uint8_t *serial_buffer, size_t serial_buffer_length)
    {
        uint8_t *current_ptr = serial_buffer;
        memset(serial_buffer, 0, serial_buffer_length);

        memcpy(current_ptr, SimConstants::MESSAGE_900_START_INDICATOR, SimConstants::MESSAGE_900_START_INDICATOR_LENGTH*sizeof(char));
        current_ptr += SimConstants::MESSAGE_900_START_INDICATOR_LENGTH*sizeof(char);

        // copy contents of artifact message
        memcpy(current_ptr, art, sizeof(artifact_message_t));
        current_ptr += sizeof(artifact_message_t);

        memcpy(current_ptr, SimConstants::MESSAGE_900_END_INDICATOR, 
                    sizeof(SimConstants::MESSAGE_900_END_INDICATOR_LENGTH));
        current_ptr += SimConstants::MESSAGE_900_END_INDICATOR_LENGTH * sizeof(char);

        if(current_ptr != serial_buffer + (serial_buffer_length*sizeof(uint8_t))) {
            fprintf(stderr, "warning: %s, current_ptr address: %p, serial_buffer address: %p, serial_buffer_length: %lx\n",
            __func__, current_ptr, serial_buffer, serial_buffer_length);

        }

    }

    /**
     * Note: serial_buffer_length must include bytes for the message plus the start message bytes
     * SERIAL_ARTIFACT_BUFFER_LENGTH = SERIAL_ARTIFACT_MESSAGE_LENGTH 
     *                                  + MESSAGE_900_START_INDICATOR_LENGTH
     *                                  + MESSAGE_900_END_INDICATOR_LENGTH
     *      
     * 
     * This version of the serialize function copies each field individually, so that the zero padded struct
     * bytes are not copied and eventually transmitted.
     * 
     *
    void serialize_artifact_for_900MHz(const artifact_message_t* art, uint8_t *serial_buffer, size_t serial_buffer_length)
    {
        uint8_t *current_ptr = serial_buffer;
        memset(serial_buffer, 0, serial_buffer_length);

        memcpy(current_ptr, SimConstants::MESSAGE_900_START_INDICATOR, SimConstants::MESSAGE_900_START_INDICATOR_LENGTH*sizeof(char));
        current_ptr += SimConstants::MESSAGE_900_START_INDICATOR_LENGTH*sizeof(char);

        memcpy(current_ptr, &art->dest_id, sizeof(((artifact_message_t*)0)->dest_id));
        current_ptr += sizeof(((artifact_message_t*)0)->dest_id);
       
        memcpy(current_ptr, &art->src_id, sizeof(((artifact_message_t*)0)->src_id));
        current_ptr += sizeof(((artifact_message_t*)0)->src_id);

        memcpy(current_ptr, &art->msg_type, sizeof(((artifact_message_t*)0)->msg_type));
        current_ptr += sizeof(((artifact_message_t*)0)->msg_type);

        memcpy(current_ptr, &art->msg_id, sizeof(((artifact_message_t*)0)->msg_id));
        current_ptr += sizeof(((artifact_message_t*)0)->msg_id);

        memcpy(current_ptr, &art->artifact, sizeof(((artifact_message_t*)0)->msg_id));
        current_ptr += sizeof(((artifact_message_t*)0)->artifact);

        // time stamp
        memcpy(current_ptr, &art->stamp.sec, sizeof(((artifact_message_t*)0)->stamp.sec));
        current_ptr += sizeof(((artifact_message_t*)0)->stamp.sec);

        memcpy(current_ptr, &art->stamp.nsec, sizeof(((artifact_message_t*)0)->stamp.nsec));
        current_ptr += sizeof(((artifact_message_t*)0)->stamp.nsec);

        // position
        memcpy(current_ptr, &art->position.x, sizeof(((artifact_message_t*)0)->position.x));
        current_ptr += sizeof(((artifact_message_t*)0)->position.x);

        memcpy(current_ptr, &art->position.y, sizeof(((artifact_message_t*)0)->position.y));
        current_ptr += sizeof(((artifact_message_t*)0)->position.y);
        
        memcpy(current_ptr, &art->position.z, sizeof(((artifact_message_t*)0)->position.z));
        current_ptr += sizeof(((artifact_message_t*)0)->position.z);

        memcpy(current_ptr, marblecomm::SimConstants::MESSAGE_900_END_INDICATOR, 
                    sizeof(marblecomm::SimConstants::MESSAGE_900_END_INDICATOR_LENGTH));
        current_ptr += marblecomm::SimConstants::MESSAGE_900_END_INDICATOR_LENGTH * sizeof(char);

        if(current_ptr != serial_buffer + (serial_buffer_length*sizeof(uint8_t))) {
            fprintf(stderr, "warning: %s, current_ptr address: %p, serial_buffer address: %p, serial_buffer_length: %lx\n",
            __func__, current_ptr, serial_buffer, serial_buffer_length);

        }

    }
    */

   /**
     * 
     * Note: assumes serial_buffer does not include start of message field
     * 
     * This version of the deserialize function extracts the entire serial buffer at once
     * into the artifact_message_t art.
     * 
     * It should only be used if the serialize function did include the zero padded struct
     * bytes in the data transmitted
     * 
     */
    void deserialize_artifact_for_900MHz(artifact_message_t* art, const uint8_t *serial_buffer)
    {
        *art = *(const artifact_message_t*)serial_buffer;
    }

    /**
     * 
     * Note: assumes serial_buffer does not include start of message field
     * 
     * This version of the deserialize function extracts each artifact_message_t field individually.
     * It should only be used if the serialize function did not include the zero padded struct
     * bytes in the data transmitted
     * 
     *
    void deserialize_artifact_for_900MHz(artifact_message_t* art, const uint8_t *serial_buffer)
    {
        // need a better method of coding this to make it robust to any data type changes
        // in the structures
        const uint8_t* current_ptr = serial_buffer;
       
        art->dest_id = *(const uint8_t*)current_ptr;
        current_ptr += sizeof(uint8_t);
        
        art->src_id = *(const uint8_t*)current_ptr;
        current_ptr += sizeof(uint8_t);

        art->msg_type = *(const uint8_t*)current_ptr;
        current_ptr += sizeof(uint8_t);
        
        art->msg_id = *(const uint16_t*)current_ptr;
        current_ptr += sizeof(uint16_t);

        art->artifact = *(const uint8_t*)current_ptr;
        current_ptr += sizeof(uint8_t);

        art->stamp.sec = *(const uint64_t*)current_ptr;
        current_ptr += sizeof(uint64_t);

        art->stamp.nsec = *(const uint64_t*)current_ptr;
        current_ptr += sizeof(uint64_t);

        art->position.x = *(const double*)current_ptr;
        current_ptr += sizeof(double);

        art->position.y = *(const double*)current_ptr;
        current_ptr += sizeof(double);

        art->position.z = *(const double*)current_ptr;
        current_ptr += sizeof(double);       
    }
    */



   /** NOTE: this function likely belongs in message900 class. It is here due to the simulation of ROS types.
     *  Referencing the data structures in this file in the message900 class creates circular references.
     * 
     */

    int process_rx_message(const std::string& rx_string, ack_message_t* ack, bool ack_required)
    {
        static uint16_t expectedMessageId = 0;
        static uint16_t messageIdMismatch = 0;

         // assuming string contains minimum number required bytes (quick and dirty proof of concept code)
        switch(rx_string[2])
        {
            case SimConstants::ROBOT_POSITION:
                fprintf(stderr, "message type is robot position, time to deserialize, publish and ack\n");
            break;
            case SimConstants::ARTIFACT_POSITION:
                //fprintf(stderr, "message type is artifact position, time to deserialize, publish, and ack\n");
                artifact_message_t art;
                deserialize_artifact_for_900MHz(&art, (const uint8_t*)rx_string.c_str());

                // The following is simply for testing that all pack
                if(expectedMessageId != art.msg_id){
                    fprintf(stdout, "message id mismatch, expected: %hu, received: %hu\n", expectedMessageId, art.msg_id);
                    ++messageIdMismatch;
                    expectedMessageId = static_cast<uint16_t>(art.msg_id + 1);
                }
                else{
                    if(expectedMessageId % 20 == 0){
                        fprintf(stderr, "processed message %hu\n", expectedMessageId);
                    }
                    ++expectedMessageId;
                }

                //fprintf(stderr, "Deserialized artifact message\n");
                //print_artifact_message(&art);
                
                if(ack_required){
                    //fprintf(stderr, "next step is sending an ack\n");
                    //fprintf(stderr, "if this were ROS, this is time to PUBLISH ARTIFACT POSITION topic\n");
                    populate_ack_message(ack, art.src_id, art.dest_id, art.msg_id);
                    return SimConstants::SEND_ACK;
                }
                else{
                    return SimConstants::NO_ACK;
                }
                
            break;
            case SimConstants::ACK:
                fprintf(stderr, "message type is ACK, record the received ack\n");
            break;
            case SimConstants::REPORT_TO_ANCHOR:
                fprintf(stderr, "message type is report to anchor, time to deserialize, publish, and ack\n");
            break;
            default:
                fprintf(stderr, "error, %s, unknown message type: %hhu\n", __func__, rx_string[2]);
        }

        return -1;
    }

    void populate_ack_message(ack_message_t* ack, uint8_t dest_id, uint8_t src_id, uint16_t msg_id)
    {
        ack->dest_id = dest_id;
        ack->src_id = src_id;
        ack->msg_id = msg_id;
        ack->msg_type = SimConstants::ACK;
    }


    void serialize_acknowledgement_for_900MHz(const ack_message_t* ack, uint8_t *serial_buffer, size_t serial_buffer_length)
    {
        uint8_t *current_ptr = serial_buffer;
        memset(serial_buffer, 0, serial_buffer_length);

        memcpy(current_ptr, SimConstants::MESSAGE_900_START_INDICATOR, SimConstants::MESSAGE_900_START_INDICATOR_LENGTH*sizeof(char));
        current_ptr += SimConstants::MESSAGE_900_START_INDICATOR_LENGTH*sizeof(char);

        // copy contents of artifact message
        memcpy(current_ptr, ack, sizeof(ack_message_t));
        current_ptr += sizeof(ack_message_t);

        memcpy(current_ptr, SimConstants::MESSAGE_900_END_INDICATOR, 
                    sizeof(SimConstants::MESSAGE_900_END_INDICATOR_LENGTH));
        current_ptr += SimConstants::MESSAGE_900_END_INDICATOR_LENGTH * sizeof(char);

        if(current_ptr != serial_buffer + (serial_buffer_length*sizeof(uint8_t))) {
            fprintf(stderr, "warning: %s, current_ptr address: %p, serial_buffer address: %p, serial_buffer_length: %lx\n",
            __func__, current_ptr, serial_buffer, serial_buffer_length);

        }
    }


      bool extract_rx_message(std::string& rx_data, std::string& extracted_rx_data)
     {
         std::size_t foundStart, foundEnd;

         // find start of message
         foundStart = rx_data.find(SimConstants::MESSAGE_900_START_INDICATOR);
         if(foundStart == std::string::npos){
             return false;
         }
         //fprintf(stderr, "foundStart %lu\n", foundStart);

        // find end of message
        foundEnd = rx_data.find(SimConstants::MESSAGE_900_END_INDICATOR, foundStart + SimConstants::MESSAGE_900_START_INDICATOR_LENGTH);
        if(foundEnd == std::string::npos){
            return false;
        }
        //fprintf(stderr, "foundEnd %lu\n", foundEnd);

        if(foundEnd <= foundStart){
            fprintf(stderr, "%s, warning: foundEnd: %lu <= foundStart: %lu\n", __func__, foundEnd, foundStart);
            return false;
        }
      
        // extract the first character after the start indicator
        // string length is foundend - (foundStart+MESSAGE_900_START_INDICATOR_LENGTH,)
        extracted_rx_data = rx_data.substr(foundStart + SimConstants::MESSAGE_900_START_INDICATOR_LENGTH, 
                foundEnd-foundStart - SimConstants::MESSAGE_900_START_INDICATOR_LENGTH);
       
        // remove chararacters from string, including the end indicator
        rx_data = rx_data.substr(foundEnd + SimConstants::MESSAGE_900_END_INDICATOR_LENGTH);
 
        return true;

     }

    
} // end rfd900sim namespace
