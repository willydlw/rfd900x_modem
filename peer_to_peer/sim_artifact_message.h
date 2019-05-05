#ifndef SIMULATE_ARTIFACT_MESSAGE_INCLUDED_H
#define SIMULATE_ARTIFACT_MESSAGE_INCLUDED_H

#include <cstdint>
#include <string>
#include "simulation_constants.h"

namespace rfd900sim
{
    
    /************* GENERAL SIMULATION DATA TYPES *************/
    // simulate position of point in free space
    struct point_t{
        double x;
        double y;
        double z;
    };

    
    struct timestamp_t{
        uint64_t sec;
        uint64_t nsec;
    };

    // simulate ROS std_msgs/header.msg
    struct header_msg_t{
        uint32_t seq;           // sequence id
        timestamp_t stamp;
        std::string frame_id;   // frame this data is associated with
    };

   
    // simulate ros geometry_msgs/PointStamped.msg
    struct point_stamped_message_t{
        header_msg_t    hdr;
        point_t         p;
    };


    /**************  ARTIFACT MESSAGES *********************************/

    // Competition Artifcact Type Strings
    const char * const artifact_strings[] = {
        "Survivor", "Cell Phone", "Backpack"  //, "Drill", "Fire Extinguisher"
    };

    struct artifact_message_t{
        
        uint8_t dest_id;
        uint8_t src_id;
        uint8_t msg_type;
        uint16_t msg_id;
        uint8_t artifact;
        timestamp_t stamp;
        point_t position;
        
    };

    constexpr size_t SERIAL_ARTIFACT_MESSAGE_LENGTH = sizeof(((artifact_message_t*)0)->dest_id)
                + sizeof(((artifact_message_t*)0)->src_id)
                + sizeof(((artifact_message_t*)0)->msg_type)
                + sizeof(((artifact_message_t*)0)->msg_id)
                + sizeof(((artifact_message_t*)0)->artifact)
                + sizeof(((artifact_message_t*)0)->stamp.sec)
                + sizeof(((artifact_message_t*)0)->stamp.nsec)
                + sizeof(((artifact_message_t*)0)->position.x)
                + sizeof(((artifact_message_t*)0)->position.y)
                + sizeof(((artifact_message_t*)0)->position.z);


    /**************** ACK MESSAGES **********************/
    struct ack_message_t{
        uint8_t dest_id;
        uint8_t src_id;
        uint8_t msg_type;
        uint16_t msg_id;
    };

    
    // general simulation functions
    
    double random_double( int max = 3000);
    void get_timestamp(timestamp_t *ts);

    void print_point_stamped_message(const point_stamped_message_t *psm);

    void random_point_stamped_message(point_stamped_message_t *psm);
    void random_point(point_t *p);
    void fill_header_message(header_msg_t  *hm);


    // artifact functions

    void simulate_artifact_message(artifact_message_t *art, uint8_t dest, uint8_t src);
    void print_artifact_message(const artifact_message_t *art);

    void serialize_artifact_for_900MHz(const artifact_message_t* art, uint8_t *serial_buffer, size_t msg_length);
    void deserialize_artifact_for_900MHz(artifact_message_t* art, const uint8_t *serial_buffer);
    

    int process_rx_message(const std::string& rx_string, ack_message_t* ack, bool ack_required = false);
    void populate_ack_message(ack_message_t* ack, uint8_t dest_id, uint8_t src_id, uint16_t msg_id);



    // general message functions
    bool extract_rx_message(std::string& rx_data, std::string& extracted_rx_data);


} // marblecomm


#endif 