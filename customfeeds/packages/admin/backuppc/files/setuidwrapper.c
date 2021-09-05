#include <unistd.h>

int main(int argc, char* argv[])
{
    execv("/usr/share/backuppc/bin/BackupPC_Admin_real", argv);
    return 0;
}

