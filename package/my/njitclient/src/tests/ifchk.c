
#include <stdio.h>
#include <string.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_ether.h>
#include <net/ethernet.h>
#include <net/if_types.h>

int main()
{
	struct ifaddrs *ifap = NULL;
	struct ifaddrs *i    = NULL;
	const struct sockaddr_dl *sdl = NULL;
	caddr_t addr;

	if (getifaddrs(&ifap) != 0)
	{
		char reason[256]="";
		strerror_r(errno, reason, sizeof(reason));
		fprintf(stderr, "Error: %s: %s\n", "getifaddrs() failed", reason);
		return (EXIT_FAILURE);
	}

	for (i=ifap; i!=NULL; i=i->ifa_next)
	{
		sdl = (const struct sockaddr_dl *) i->ifa_addr;
		if (sdl != NULL
			&& sdl->sdl_family == AF_LINK
			&& sdl->sdl_type == IFT_ETHER
			&& sdl->sdl_alen == ETHER_ADDR_LEN)
		{
			addr = LLADDR(sdl);

			if (i->ifa_name != NULL)
			{
				printf("%s", i->ifa_name);
			}
			else
			{
				printf("%s", "(unknown)");
			}
			printf("\t");
			printf("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n",
				addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
		}
	}

	freeifaddrs(ifap);

	return (0);
}


