#define POLYNOMIAL 0xEDB88320

static unsigned int table[256];
static int have_table = 0;

void make_table()
{
    int i, j;
    have_table = 1;
    for (i = 0; i < 256; i++)
        for (j = 0, table[i] = i; j < 8; j++)
            table[i] = (table[i] >> 1) ^ ((table[i] & 1) ? POLYNOMIAL : 0);
}

unsigned int crc32(unsigned char *data, int len)
{
    int i;
    unsigned int crc = 0xFFFFFFFF;
    if(!have_table) make_table();
    for (i = 0; i < len; i++)
        crc = (crc >> 8) ^ table[(crc ^ data[i]) & 0xFF];
    return ~crc;
}
