
#define PRIMARY_HEADER_SIZE 6
#define SECONDARY_HEADER_SIZE 10
#define MAX_DATA_SIZE 256

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


typedef struct {
    CCSDS_PrimaryHeader primary;
    CCSDS_SecondaryHeader secondary;
    uint8_t data[MAX_DATA_SIZE];
    uint32_t crc;
} CCSDS_Packet;

extern CCSDS_Packet ccsds_pkt;
bool uart_get_ccsds_pkt();
