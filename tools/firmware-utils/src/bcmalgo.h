#ifndef bcmutils_H
#define bcmutils_H

typedef struct
{
	uint16_t magic;
	uint16_t control;
	uint16_t rev_maj;
	uint16_t rev_min;
	uint32_t build_date;
	uint32_t filelen;
	uint32_t ldaddress;
	char filename[64];
	uint16_t hcs;
	uint16_t her_znaet_chto; //v dushe ne ebu
	uint32_t crc;
} ldr_header_t;


/**
 * Reverses endianess of a 32bit int, if the ENDIAN_REVERSE_NEEDED defined at compile-time
 * @param data
 * @return
 */
uint32_t reverse_endian32 ( uint32_t data );

/**
 * Reverses endianess of a 16bit int, if the ENDIAN_REVERSE_NEEDED defined at compile-time
 * @param data
 * @return
 */
uint16_t reverse_endian16 ( uint16_t data );
/**
 * Calculates the strange crc (used by bcm modems) of the file. Thnx fly out to Vector for the algorithm.
 * @param filename
 * @return
 */
uint32_t get_file_crc ( char* filename );

/**
 * Calculates HCS of the header.
 * @param hd
 * @return
 */
uint16_t get_hcs ( ldr_header_t* hd );

/**
 * Constructs the header of the image with the information given It also automagically calculates HCS and writes it there.
 * @param magic - magic device bytes
 * @param rev_maj - major revision
 * @param rev_min - minor revision
 * @param build_date - build date (seconds from EPOCH UTC)
 * @param filelen - file length in bytes
 * @param ldaddress - Load adress
 * @param filename - filename
 * @param crc_data - the crc of the data
 * @return
 */
ldr_header_t* construct_header ( uint32_t magic, uint16_t rev_maj,uint16_t rev_min, uint32_t build_date, uint32_t filelen, uint32_t ldaddress, const char* filename, uint32_t crc_data );

/**
 * Dumps header information to stdout.
 * @param hd
 */
int dump_header ( ldr_header_t* hd );


/**
 * Returns a null terminated string describing what the control number meens
 * DO NOT FREE IT!!!
 * @param control
 * @return
 */
char* get_control_info ( uint16_t control );
#endif

/**
 * Calculates bcmCRC of a data buffer.
 * @param filebuffer - pointer to buffer
 * @param size - buffer size
 * @return
 */
uint32_t get_buffer_crc ( char* filebuffer, size_t size );
