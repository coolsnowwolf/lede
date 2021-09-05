#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

int discovery()
{
    DIR *dir;
    struct dirent *ent;
    bool comma = false;
    if ((dir = opendir ("/sys/kernel/debug/ieee80211/")) != NULL) {
        printf("{\"data\":[");
        while ((ent = readdir (dir)) != NULL) {
            if (strcmp(".", ent->d_name) && strcmp("..", ent->d_name)) {
                if (comma)
                    printf(",");
                printf("{\"{#PHY}\":\"%s\"}", ent->d_name);
                comma = true;
            }
        }
        printf("]}\n");
        closedir(dir);
    } else {
        perror("");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int get_param(char *phy, char *stat)
{
    char *filename = NULL;
    FILE *f = NULL;
    phy = basename(phy);
    stat = basename(stat);
    if (asprintf(&filename, "/sys/kernel/debug/ieee80211/%s/statistics/%s", phy, stat) > 0)
        f = fopen(filename, "r");

    if (f != NULL) {
        char temp[256];
        while (fgets(temp, 256, f) != NULL)
            printf("%s",temp);

        fclose(f);
    } else {
        perror("");
        return EXIT_FAILURE;
    }
    free(filename);
    return EXIT_SUCCESS;
}

int usage(char *name)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, " %s discovery\n", name);
    fprintf(stderr, " => print mac80211.phydiscovery discovery rule\n");
    fprintf(stderr, " %s PHY STAT\n", name);
    fprintf(stderr, " => cat /sys/kernel/debug/ieee80211/PHY/statistics/STAT as root\n");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{

    switch (argc) {
    case 2:
        return discovery();
    case 3:
        return get_param(argv[1], argv[2]);
    default:
        return usage(argv[0]);
    }
}
