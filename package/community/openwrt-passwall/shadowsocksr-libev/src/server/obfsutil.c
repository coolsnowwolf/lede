int get_head_size(char *plaindata, int size, int def_size) {
    if (plaindata == NULL || size < 2)
        return def_size;
    int head_type = plaindata[0] & 0x7;
    if (head_type == 1)
        return 7;
    if (head_type == 4)
        return 19;
    if (head_type == 3)
        return 4 + plaindata[1];
    return def_size;
}

static int shift128plus_init_flag = 0;
static uint64_t shift128plus_s[2] = {0x10000000, 0xFFFFFFFF};

void init_shift128plus(void) {
    if (shift128plus_init_flag == 0) {
        shift128plus_init_flag = 1;
        uint32_t seed = time(NULL);
        shift128plus_s[0] = seed | 0x100000000L;
        shift128plus_s[1] = ((uint64_t)seed << 32) | 0x1;
    }
}

uint64_t xorshift128plus(void) {
    uint64_t x = shift128plus_s[0];
    uint64_t const y = shift128plus_s[1];
    shift128plus_s[0] = y;
    x ^= x << 23; // a
    x ^= x >> 17; // b
    x ^= y ^ (y >> 26); // c
    shift128plus_s[1] = x;
    return x + y;
}

