static uint32_t crc32_table[256] = {0};

void init_crc32_table(void) {
    uint32_t c, i, j;
    if (crc32_table[0] == 0) {
        for (i = 0; i < 256; i++) {
            c = i;
            for (j = 0; j < 8; j++) {
                if (c & 1)
                    c = 0xedb88320L ^ (c >> 1);
                else
                    c = c >> 1;
            }
            crc32_table[i] = c;
        }
    }
}

uint32_t crc32(unsigned char *buffer, unsigned int size) {
    uint32_t crc = 0xFFFFFFFF;
    unsigned int i;
    for (i = 0; i < size; i++) {
        crc = crc32_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

void fillcrc32to(unsigned char *buffer, unsigned int size, unsigned char *outbuffer) {
    uint32_t crc = 0xFFFFFFFF;
    unsigned int i;
    for (i = 0; i < size; i++) {
        crc = crc32_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
    }
    crc ^= 0xFFFFFFFF;
    outbuffer[0] = crc;
    outbuffer[1] = crc >> 8;
    outbuffer[2] = crc >> 16;
    outbuffer[3] = crc >> 24;
}

void fillcrc32(unsigned char *buffer, unsigned int size) {
    uint32_t crc = 0xFFFFFFFF;
    unsigned int i;
    size -= 4;
    for (i = 0; i < size; i++) {
        crc = crc32_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
    }
    buffer += size;
    buffer[0] = crc;
    buffer[1] = crc >> 8;
    buffer[2] = crc >> 16;
    buffer[3] = crc >> 24;
}

void adler32_short(unsigned char *buffer, unsigned int size, uint32_t *a, uint32_t *b) {
    for (int i = 0; i < size; i++) {
        *a += buffer[i];
        *b += *a;
    }
    *a %= 65521;
    *b %= 65521;
}

#define NMAX 5552
uint32_t adler32(unsigned char *buffer, unsigned int size) {
    uint32_t a = 1;
    uint32_t b = 0;
    while ( size >= NMAX ) {
        adler32_short(buffer, NMAX, &a, &b);
        buffer += NMAX;
        size -= NMAX;
    }
    adler32_short(buffer, size, &a, &b);
    return (b << 16) + a;
}
#undef NMAX

void filladler32(unsigned char *buffer, unsigned int size) {
    size -= 4;
    uint32_t checksum = adler32(buffer, size);
    buffer += size;
    buffer[0] = checksum;
    buffer[1] = checksum >> 8;
    buffer[2] = checksum >> 16;
    buffer[3] = checksum >> 24;
}

int checkadler32(unsigned char *buffer, unsigned int size) {
    size -= 4;
    uint32_t checksum = adler32(buffer, size);
    buffer += size;
    return checksum == (((uint32_t)buffer[3] << 24)
            | ((uint32_t)buffer[2] << 16)
            | ((uint32_t)buffer[1] << 8)
            | (uint32_t)buffer[0]);
}

