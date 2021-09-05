
#include <string.h>
#include <stdlib.h>
#include "ssl.h"

int main(int argc, char *argv[])
{
    bigint *m1, *m2, *d;
    BI_CTX *ctx = bi_initialize();
    char cmp1[1024], cmp2[1024];

    const char *plaintext = /* 128 byte number */
        "01aaaaaaaaaabbbbbbbbbbbbbbbccccccccccccccdddddddddddddeeeeeeeeee"
        "01aaaaaaaaaabbbbbbbbbbbbbbbccccccccccccccdddddddddddddeeeeeeeeee";
    d = bi_import(ctx, (uint8_t *)plaintext, strlen(plaintext));
    memset(cmp1, 0, sizeof(cmp1));

    while (1)
    {
        bi_set_mod(ctx, bi_clone(ctx, d), 0);
        m1 = bi_square(ctx, bi_copy(d));
        m2 = bi_residue(ctx, m1);
        bi_free_mod(ctx, 0);

        //bi_export(ctx, bi_copy(d), cmp1, sizeof(cmp1));
        bi_export(ctx, m2, cmp2, sizeof(cmp2));

        if (memcmp(cmp1, cmp2, sizeof(cmp1)) != 0)
        {
            printf("Error!\n"); TTY_FLUSH();
            break;
        }

        d = bi_add(ctx, d, int_to_bi(ctx, 1));
    }

    bi_free(ctx, d);
    bi_terminate(ctx);
    printf("all good\n"); TTY_FLUSH();
    return 0;

}

