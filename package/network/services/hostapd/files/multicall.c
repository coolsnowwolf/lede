#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern int hostapd_main(int argc, char **argv);
extern int wpa_supplicant_main(int argc, char **argv);

int main(int argc, char **argv)
{
	bool restart = false;
	const char *prog = argv[0];

restart:
	if (strstr(argv[0], "hostapd"))
		return hostapd_main(argc, argv);
	else if (strstr(argv[0], "wpa_supplicant"))
		return wpa_supplicant_main(argc, argv);

	if (!restart && argc > 1) {
		argv++;
		argc--;
		restart = true;
		goto restart;
	}

	fprintf(stderr, "Invalid command.\nUsage: %s wpa_supplicant|hostapd [<arguments>]\n", prog);
	return 255;
}
