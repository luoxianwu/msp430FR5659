
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

// Define a structure for the main CCSDS Packet Frame header
typedef struct {
    /*
    uint8_t version_number : 3;     // CCSDS Version Number (3 bits)
    uint8_t packet_type : 1;        // Packet Type (1 bit) . 0 telemetry, 1 command
    uint8_t second_header_flag : 1; // 2nd Header Flag (1 bit)
    uint16_t app_id : 11;           // Application ID (11 bits)
    */
    uint8_t ver_type_sec_apid;      //see bit field above
    uint8_t apid_lo;                //low 8 bits of apid
    uint8_t segence_flag : 2;       // Packet Group Flag (2 bits) 0 continu, 1 first, 2 last, 3 no segment
    uint16_t sequence_number : 14;  // Sequence Number (14 bits)
    uint16_t data_length : 16;      // Data Length (16 bits)
} __attribute__ ((aligned (1), packed)) Primary_Header;

// Define a structure for the CCSDS 2nd Header
typedef struct {
    uint64_t timing_info : 48;      // Timing Info (48 bits)
    uint8_t segment_number : 8;     // Segment Number (8 bits)
    uint8_t function_code : 8;      // Function Code (8 bits)
    uint16_t address_code : 16;     // Address Code (16 bits)
} __attribute__ ((aligned (1), packed )) Second_Header;


typedef struct {
    uint8_t pkt_sync[2];
    Primary_Header primary;
    Second_Header  secondary;
    uint8_t data[MAX_DATA_SIZE];
    uint32_t crc;
} CCSDS_Packet;

extern CCSDS_Packet ccsds_pkt;
extern CCSDS_Packet ccsds_pkt_response;
bool uart_get_ccsds_pkt();
// return ccsds packet length
int ccdss_pack_data( void *data, size_t data_len );
