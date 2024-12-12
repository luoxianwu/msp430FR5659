
#define PRIMARY_HEADER_SIZE 6
#define SECONDARY_HEADER_SIZE 10
#define MAX_DATA_SIZE 256
#define PACKET_SYNC 0x55AA
#define SYNC_BYTES  2

#define TYPE_TM 0
#define TYPE_TC 1

#define CCSDS_VERSION(x)  ((x>>5) & 0x7)
#define CCSDS_TYPE(x)     ( (x & 0x10) ? 1 : 0 )
#define CCSDS_SEC_HDR(x)  ( (x & 0x8)  ? 1 : 0 )
#define CCSDS_APID_H(x)   (x & 0x7)


typedef enum {
    STATE_IDLE,
    STATE_SYNC,
    STATE_PRIMARY_HEADER,
    STATE_SECONDARY_HEADER,
    STATE_DATA,
    STATE_VALID,
    STATE_CRC_ERR

} CCSDS_State;

/*
typedef struct {
    uint16_t version_type_flag_apid;
    uint16_t sequence_control;
    uint16_t data_length;
} __attribute__((packed)) CCSDS_PrimaryHeader;

typedef struct {
    uint8_t timing_info[6];
    uint8_t segment_number;
    uint8_t function_code;
    uint16_t address_code;
} __attribute__((packed)) CCSDS_SecondaryHeader;
*/

// Define a structure for the main CCSDS Packet Frame header, big endian
typedef struct {
    /*
    uint8_t version_number : 3;     // CCSDS Version Number (3 bits)
    uint8_t packet_type : 1;        // Packet Type (1 bit) . 0 telemetry, 1 command
    uint8_t second_header_flag : 1; // 2nd Header Flag (1 bit)
    uint16_t app_id : 11;           // Application ID (11 bits)
    */
    uint8_t ver_type_sec_apid;      //see bit field above
    uint8_t apid_lo;                //low 8 bits of apid
    uint8_t segence_flag_number_h ; // Packet Group Flag (2 bits) 0 continu, 1 first, 2 last, 3 no segment
    uint8_t sequence_number_l;      // Sequence Number (14 bits)
    uint8_t data_length_h;          // Data Length (16 bits)
    uint8_t data_length_l;
} __attribute__ ((aligned (1), packed)) Primary_Header;

// Define a structure for the CCSDS 2nd Header
typedef struct {
    uint8_t timing_info[6];         // Timing Info (48 bits)
    uint8_t segment_number;         // Segment Number (8 bits)
    uint8_t function_code;          // Function Code (8 bits)
    uint8_t address_code_h;         // Address Code (16 bits)
    uint8_t address_code_l;
} __attribute__ ((aligned (1), packed )) Second_Header;


typedef struct {
    uint8_t pkt_sync[2];
    Primary_Header primary;
    Second_Header  secondary;
    uint8_t data[MAX_DATA_SIZE];
    uint32_t crc;                   // location vary, don't refer to it
    //following variables are not part of ccsds packet for data transmit. just for management
    CCSDS_State state;
    uint16_t bytes_received;
    uint16_t data_length;

} CCSDS_Packet;

extern CCSDS_Packet ccsds_receive_pkt;
extern CCSDS_Packet ccsds_pkt_response;

void ccsds_init(CCSDS_Packet *packet);
bool uart_get_ccsds_pkt();
// return ccsds packet length
unsigned int ccdss_pack_data( void *data, size_t data_len );
