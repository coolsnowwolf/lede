#include <sys/mount.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>

#include "insmod.h"
#include "mknod.h"
#include "mkdev_node.h"
#include "cmdline.h"
#include "switch_root.h"

static void hang(void);

static int mount_fs(void)
{
	int ret;

	unsigned long flags = MS_NOEXEC | MS_NOSUID | MS_RELATIME | MS_NODEV;

	ret = mount(NULL, "/proc", "proc", flags, NULL);

	if (ret < 0)
		goto err;

	ret = mount(NULL, "/sys", "sysfs", flags, NULL);

	if (ret < 0)
		goto err;

	ret = mount(NULL, "/dev", "tmpfs", MS_RELATIME, NULL);

	if (ret < 0)
		goto err;

	return 0;
err:
	ret = errno;
	return ret;
}

struct dev_node_t
{
	const char *mode;
	const char *name;
	char type;
	uint8_t major;
	uint8_t minor;
} dev_nodes[] = {
	{
		.mode = "0666",
		.name = "/dev/tty",
		.type = 'c',
		.major = 5,
		.minor = 0,
	},
	{
		.mode = "0622",
		.name = "/dev/console",
		.type = 'c',
		.major = 5,
		.minor = 1,
	},
	{
		.mode = "0666",
		.name = "/dev/null",
		.type = 'c',
		.major = 1,
		.minor = 3,
	},
	{
		.mode = "0666",
		.name = "/dev/zero",
		.type = 'c',
		.major = 1,
		.minor = 5,
	},
};

static int make_node(void)
{
	for (int i = 0; i < sizeof(dev_nodes) / sizeof((dev_nodes)[0]); i++)
	{
		if (do_mknod(dev_nodes[i].mode, dev_nodes[i].name, dev_nodes[i].type, dev_nodes[i].major, dev_nodes[i].minor) < 0)
		{
			if (errno == EEXIST)
				continue;

			fprintf(stderr, "make %s failed:%s\n", dev_nodes[i].name, strerror(errno));
			return -1;
		}
	}
	return 0;
}

static int insert_kernel_module(const char *path, const char **argv)
{
	int ret;
	FILE *fp = NULL;
	char module_name[1024] = {0};

	if (access(path, F_OK) != 0)
	{
		fprintf(stderr, "%s not found\n", path);
		return -1;
	}

	fp = fopen(path, "r");
	if (!fp)
	{
		fprintf(stderr, "open %s failed:%s\n", path, strerror(errno));
		return -1;
	}

	while ((fgets(module_name, sizeof(module_name) - 1, fp)) != NULL)
	{
		for (int i = 0; i < sizeof(module_name); i++)
		{
			if (module_name[i] == '\n')
			{
				module_name[i] = '\0';
				break;
			}
		}

		ret = do_insmod(module_name, argv);

		if (ret)
		{
			break;
		}
	}

	fclose(fp);

	return ret;
}

static void e2fsck(const char *dev_path)
{
	int pid;
	pid = fork();
	if (pid < 0)
	{
		printf("fork failed");
		hang();
	}
	if (pid == 0)
	{
		execl("/bin/e2fsck", "-f", "-y", dev_path);
		printf("execl failed");
		hang();
	}
	else
	{
		waitpid(pid);
	}
}

static int mount_rootfs(struct bootargs_t *bootargs)
{
	int ret = 0;
	char realroot[PATH_MAX];
	struct fstype *fstype = NULL;

	// OpenWrt dont have /dev/disk/by-* yet, use realpath as workaround.
	if (realpath(bootargs->root, realroot) == NULL)
		return -1;

	fstype = bootargs->fstype;

	for (struct fstype *next = fstype->next; fstype; next = fstype->next, fstype = next)
	{
		if (strcmp(fstype->name, "ext4") == 0)
		{
			e2fsck(realroot);
		}
		ret = mount(realroot, "/root", fstype->name, MS_RELATIME, NULL);
		if (ret == 0)
			break;
	}

	return ret;
}

static void hang()
{
	exit(-1);
	while (1)
	{
	}
}

int main(int argc, const char *argv[])
{
	int ret;
	struct bootargs_t *bootargs = NULL;

	if (getpid() != 1)
	{
		fprintf(stderr, "must run PID 1\n");
		hang();
	}

	if ((ret = mount_fs()) < 0)
	{
		fprintf(stderr, "mount ramdisk fs failed:%s\n", strerror(ret));
		hang();
	}

	if (make_node() < 0)
	{
		fprintf(stderr, "make ramdisk dev node failed\n");
		hang();
	}

	if ((ret = parse_cmdline(&bootargs)) < 0)
	{
		fprintf(stderr, "cmdline format error\n");
		hang();
	}

	if (insert_kernel_module("/etc/rdloader_list", argv + 1) != 0)
	{
		fprintf(stderr, "failed to load kmods\n");
		hang();
	}

	printf("Waiting for root: %s\n", bootargs->root);

	do
	{
		ret = mkdev_node();

		if (ret)
			continue;

		ret = mount_rootfs(bootargs);
	} while (ret != 0);

	free_bootargs(&bootargs);

	umount("/dev");
	umount("/proc");
	umount("/sys");

	switch_root("/root", "/etc/preinit", argv++);

	return 0;
}
