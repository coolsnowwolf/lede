
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char *argv[]) {
	char buf[1024];
	ssize_t r;

	if (argc != 2) {
		fprintf(stderr, "Write to and read from a Unix domain socket.\n\nUsage: %s <socket>\n", argv[0]);
		return 1;
	}

	size_t addrlen = strlen(argv[1]);

	/* Allocate enough space for arbitrary-length paths */
	char addrbuf[offsetof(struct sockaddr_un, sun_path) + addrlen + 1];
	memset(addrbuf, 0, sizeof(addrbuf));

	struct sockaddr_un *addr = (struct sockaddr_un *)addrbuf;
	addr->sun_family = AF_UNIX;
	memcpy(addr->sun_path, argv[1], addrlen+1);

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
		return 1;
	}

	if (connect(fd, (struct sockaddr*)addr, sizeof(addrbuf)) < 0) {
		fprintf(stderr, "Can't connect to `%s': %s\n", argv[1], strerror(errno));
		return 1;
	}

	/* Check if stdin refers to a terminal */
	if (!isatty(fileno(stdin))) {
		/* Read from stdin and write to socket */
		while (0 < (r = fread(buf, 1, sizeof(buf), stdin))) {
			send(fd, buf, r, 0);
		}
	}

	/* Read from socket and write to stdout */
	while (1) {
		r = recv(fd, buf, sizeof(buf), 0);
		if (r < 0) {
			fprintf(stderr, "read: %s\n", strerror(errno));
			return 1;
		}

		if (r == 0)
			return 0;

		fwrite(buf, r, 1, stdout);
	}

	return 0;
}
