#include <dirent.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
//begin modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 
#include <errno.h>
//end modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 
#include "util.h"
#include "query_pcie_mode.h"
#define MAJOR 1
#define MINOR 0
#define REVISION 4
/*
 * Generally, we do not modify version info, so several modifications will share
 * the same version code. SUBVERSION is used for customized modification to
 * distinguise this version from previous one. SUBVERSION adds up before you
 * send the code to customers and it should be set to 0 if VERSION_STRING info
 * is changed.
 */
#define SUBVERSION 0
#define STRINGIFY_HELPER(v) #v
#define STRINGIFY(v) STRINGIFY_HELPER(v)
#define VERSION_STRING() \
    STRINGIFY(MAJOR) "." STRINGIFY(MINOR) "." STRINGIFY(REVISION)
#define MAX_PATH 256

#define _PARAM_GLOBALS_
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
//begin modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 
#include <ctype.h>
//end modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 
#define SECTION_MAX_LEN 256
#define STRVALUE_MAX_LEN 256
#define LINE_CONTENT_MAX_LEN 256
#define READ_STR_ERR -1
#define READ_STR_OK 0
//begin modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 

//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
//#include "query_pcie_mode.h"
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817

static int is_pcie_dial()
{
    return (access("/dev/mhi_QMI0", F_OK) == 0);
}

static int is_gobinet_dial()
{
    return (access("/dev/qcqmi0", F_OK) == 0);
}

static int is_qmiwwan_dial()
{
    return (access("/dev/cdc-wdm0", F_OK) == 0);
}

static size_t fibo_fread(const char *filename, void *buf, size_t size)
{
    FILE *fp = fopen(filename, "r");
    size_t n = 0;

    memset(buf, 0x00, size);

    if (fp) {
	n = fread(buf, 1, size, fp);
	if (n <= 0 || n == size) {
	    dbg_time(
		"warnning: fail to fread(%s), fread=%zd, buf_size=%zd, "
		"errno: %d (%s)",
		__func__, filename, n, size, errno, strerror(errno));
	}
	fclose(fp);
    }

    return n > 0 ? n : 0;
}

int get_qmap_num()
{
    int n;
    int qmap_num=-1;
    char buf[128];
    struct {
	char filename[255 * 2];
	char linkname[255 * 2];
    } * pl;

    if (is_pcie_dial())
        return 8;

    pl = (typeof(pl))malloc(sizeof(*pl));

    if(is_gobinet_dial())
    {
        snprintf(pl->linkname, sizeof(pl->linkname),
    	     "/sys/class/net/usb0/device/driver");
        n = readlink(pl->linkname, pl->filename, sizeof(pl->filename));
        pl->filename[n] = '\0';
        while (pl->filename[n] != '/') n--;

        snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/usb0/qmap_num");
    //2021-02-22 kaibo.zhangkaibo@fibocom.com changed begin for support mantis 0071171
        if (access(pl->filename, F_OK) == 0) {
            dbg_time("access %s", pl->filename);
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno,
                    strerror(errno));
                goto _out;
            }
        }
        else
        {
        snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/usb0/qmap_mode");
            if (access(pl->filename, F_OK) == 0) {
            dbg_time("access %s", pl->filename);
                if (errno != ENOENT) {
                    dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno,
                        strerror(errno));
                    goto _out;
                }
            }
        }
    }
    if(is_qmiwwan_dial())
    {
        snprintf(pl->linkname, sizeof(pl->linkname),
        "/sys/class/net/wwan0/device/driver");
        n = readlink(pl->linkname, pl->filename, sizeof(pl->filename));
        pl->filename[n] = '\0';
        while (pl->filename[n] != '/') n--;

        snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/wwan0/qmap_mode");
        if (access(pl->filename, F_OK) == 0) {
            dbg_time("access %s", pl->filename);
                if (errno != ENOENT) {
                    dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno,
                        strerror(errno));
                    goto _out;
                }
            }
    }

    dbg_time("access %s", pl->filename);

    if (!access(pl->filename, R_OK)) {
	n = fibo_fread(pl->filename, buf, sizeof(buf));
	if (n > 0) {
	    qmap_num = atoi(buf);
	}
    } 
_out:
    free(pl);

    return qmap_num;
}
//end modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605    
int IsInt(char* str)
{
	int len;
    len = strlen(str-1);
    len = len>=1?len:1;
    int i=0;
    for(; i < len ; i++)
    {
	    if(!(isdigit(str[i])))
            return 0;
    }
    return 1;
}
 
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605       
// read value from .ini
void IniReadValue(char *section, char *key, char *val, const char *file)
{
    FILE *fp;
    int i = 0;
    int lineContentLen = 0;
    int position = 0;
    char lineContent[LINE_CONTENT_MAX_LEN];
    bool bFoundSection = false;
    bool bFoundKey = false;
    fp = fopen(file, "r");
    if (fp == NULL) {
	printf("%s: Opent file %s failed.\n", __FILE__, file);
	return;
    }
    while (feof(fp) == 0) {
	memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
	fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
	if ((lineContent[0] == ';') || (lineContent[0] == '\0') ||
	    (lineContent[0] == '\r') || (lineContent[0] == '\n')) {
	    continue;
	}

	// check section
	if (strncmp(lineContent, section, strlen(section)) == 0) {
	    bFoundSection = true;
	    // printf("Found section = %s\n", lineContent);
	    while (feof(fp) == 0) {
		memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
		fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
		// check key
		if (strncmp(lineContent, key, strlen(key)) == 0) {
		    bFoundKey = true;
		    lineContentLen = strlen(lineContent);
		    // find value
		    for (i = strlen(key); i < lineContentLen; i++) {
			if (lineContent[i] == '=') {
			    position = i + 1;
			    break;
			}
		    }
		    if (i >= lineContentLen)
			break;
		    strncpy(val, lineContent + position,
			    strlen(lineContent + position));
		    lineContentLen = strlen(val);
		    for (i = 0; i < lineContentLen; i++) {
			if ((val[i] == '\0') || (val[i] == '\r') ||
			    (val[i] == '\n')) {
			    val[i] = '\0';
			    // break;
			}
		    }
		} else if (lineContent[0] == '[') {
		    break;
		}
	    }
	    break;
	}
    }
    if (!bFoundSection) {
	printf("No section = %s\n", section);
    } else if (!bFoundKey) {
	printf("No key = %s\n", key);
    }

    fclose(fp);
}

int readStringValue(const char *section, char *key, char *val, const char *file)
{
    char sect[SECTION_MAX_LEN];
    // printf("section = %s, key = %s, file = %s\n", section, key, file);
    if (section == NULL || key == NULL || val == NULL || file == NULL) {
	printf("%s: input parameter(s) is NULL!\n", __func__);
	return READ_STR_ERR;
    }

    memset(sect, 0, SECTION_MAX_LEN);
    sprintf(sect, "[%s]", section);
    // printf("reading value...\n");
    IniReadValue(sect, key, val, file);

    return READ_STR_OK;
}

char fileContents[80][LINE_CONTENT_MAX_LEN];
int fileIndex = 0;

void IniWriteValue(const char *section, char *key, char *val, const char *file)
{
    int err = 0;
    char lineContent[LINE_CONTENT_MAX_LEN];
    char strWrite[LINE_CONTENT_MAX_LEN];
    bool bFoundSection = false;
    bool bFoundKey = false;
    int fileIndextemp = 0;
    memset(lineContent, '\0', LINE_CONTENT_MAX_LEN);
    memset(strWrite, '\0', LINE_CONTENT_MAX_LEN);
    sprintf(strWrite, "%s=%s\n", key, val);

    while (fileIndextemp <= fileIndex) {
	memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
	memcpy(lineContent, fileContents[fileIndextemp++],
	       LINE_CONTENT_MAX_LEN);
	if ((lineContent[0] == ';') || (lineContent[0] == '\0') ||
	    (lineContent[0] == '\r') || (lineContent[0] == '\n')) {
	    continue;
	}
	// check section
	if (strncmp(lineContent, section, strlen(section)) == 0) {
	    bFoundSection = true;
	    while (fileIndextemp <= fileIndex) {
		memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
		memcpy(lineContent, fileContents[fileIndextemp],
		       LINE_CONTENT_MAX_LEN);
		// check key
		if (strncmp(lineContent, key, strlen(key)) == 0) {
		    bFoundKey = true;
		    // printf("%s: %s=%s\n", __func__, key, val);
		    memset(fileContents[fileIndextemp], 0,
			   LINE_CONTENT_MAX_LEN);
		    memcpy(fileContents[fileIndextemp], strWrite,
			   strlen(strWrite));
		    // printf("%s\n",strWrite);
		    if (err < 0) {
			printf("%s err.\n", __func__);
		    }
		    break;
		} else if (lineContent[0] == '[') {
		    fileIndextemp++;
		    break;
		}
		fileIndextemp++;
	    }
	    break;
	}
    }
    if (!bFoundSection) {
	printf("No section = %s\n", section);
    } else if (!bFoundKey) {
	printf("No key = %s\n", key);
    }
}

int writeStringVlaue(const char *section, char *key, char *val,
		     const char *file)
{
    char sect[SECTION_MAX_LEN];

    // printf("section = %s, key = %s, value=%s , file = %s\n", section, key,
    // val, file);
    if (section == NULL || key == NULL || val == NULL || file == NULL) {
	printf("%s: input parameter(s) is NULL!\n", __func__);
	return READ_STR_ERR;
    }
    memset(sect, '\0', SECTION_MAX_LEN);
    sprintf(sect, "[%s]", section);
    IniWriteValue(sect, key, val, file);
    return READ_STR_OK;
}

int writeIntValue(const char *section, char *key, int val, const char *file)
{
    char strValue[STRVALUE_MAX_LEN];
    memset(strValue, '\0', STRVALUE_MAX_LEN);
    sprintf(strValue, "%d", val);

    return writeStringVlaue(section, key, strValue, file);
}

int readIntValue(const char *section, char *key, const char *file)
{
    char strValue[STRVALUE_MAX_LEN];
    memset(strValue, '\0', STRVALUE_MAX_LEN);
    if (readStringValue(section, key, strValue, file) != READ_STR_OK) {
	printf("%s: error", __func__);
	return 0;
    }
    return (atoi(strValue));
}

int mygetch()
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

#define CONFIG_FILE "multi-pdn.ini"
#define DIAL_PROCESS "fibocom-dial"
#define MAX_QMAP_NUM 8

struct profile {
    char apn[MAX_PATH];
    char user[MAX_PATH];
    char password[MAX_PATH];
    int auth;
    int ipfamily;
//begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612 
    char pin[MAX_PATH];
//end modified by zhangkaibo add sim pin operate. mantis 0049142  20200612 
};

int shmid;
void *shm;
int qmap_num = -1;

bool connectState[MAX_QMAP_NUM];
int lock_file = -1;

void initShareMemory()
{
    shmid = shmget((key_t)9527, sizeof(int) * 8, 0666 | IPC_CREAT);
    if (shmid == -1) {
	fprintf(stderr, "shmget failed\n");
	exit(EXIT_FAILURE);
    }
    shm = shmat(shmid, 0, 0);
    if (shm == (void *)-1) {
	fprintf(stderr, "shmat failed\n");
	exit(EXIT_FAILURE);
    }
}

void destoryShareMemory()
{
    if (shmdt(shm) == -1) {
	fprintf(stderr, "shmdt failed\n");
	exit(EXIT_FAILURE);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
	fprintf(stderr, "shmctl(IPC_RMID) failed\n");
	exit(EXIT_FAILURE);
    }
}

void SIGINT_handle(int sig_num)

{
    printf("reveice signal %d \n", sig_num);
	{
        int *connectPids = (int *)shm;
        int cIndex;
        for(cIndex = 1 ; cIndex <= qmap_num ; cIndex++)
        {
            dbg_time("kill pid %d", connectPids[cIndex - 1]);
            kill(connectPids[cIndex - 1], SIGINT);
            connectState[cIndex - 1] = false;
        }
    }
    destoryShareMemory();
	close(lock_file); // 不要忘记释放文件指针
    exit(0);
}

int loadConfigFromIni(struct profile *profileList, bool *connectState)
{
    int profileIndex;
//begin modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 
    //qmap_num = readIntValue("Setting", "qmap_num", CONFIG_FILE);
    qmap_num = get_qmap_num();
	if (qmap_num == 0 || qmap_num == -1 )
		return -1;
//end modified by zhangkaibo get qmap_num process flow. mantis 0048920,0048917  20200605 
    for (profileIndex = 0; profileIndex < 8; profileIndex++) {
	char profilename[MAX_PATH];
	sprintf(profilename, "profile%d", profileIndex + 1);
	memset(profileList[profileIndex].apn, 0, MAX_PATH);
	readStringValue(profilename, "apn", profileList[profileIndex].apn,
			CONFIG_FILE);
	memset(profileList[profileIndex].user, 0, MAX_PATH);
	readStringValue(profilename, "user", profileList[profileIndex].user,
			CONFIG_FILE);
	memset(profileList[profileIndex].password, 0, MAX_PATH);
	readStringValue(profilename, "password",
			profileList[profileIndex].password, CONFIG_FILE);
	profileList[profileIndex].auth =
	    readIntValue(profilename, "auth", CONFIG_FILE);
	profileList[profileIndex].ipfamily =
	    readIntValue(profilename, "ipfamily", CONFIG_FILE);
//begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
	memset(profileList[profileIndex].pin, 0, MAX_PATH);
	readStringValue(profilename, "pin", profileList[profileIndex].pin,
			CONFIG_FILE);
//end modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
    }
    FILE *fp;
    fileIndex = 0;
    char lineContent[LINE_CONTENT_MAX_LEN];
    fp = fopen(CONFIG_FILE, "r+");
    if (fp == NULL) {
	printf("%s: Opent file %s failed.\n", __FILE__, CONFIG_FILE);
	return -2;
    }
    // read all
    while (feof(fp) == 0) {
	memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
	fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
	memcpy(fileContents[fileIndex], lineContent, strlen(lineContent));
	// printf("%s",fileContents[fileIndex]);
	fileIndex++;
    }
    fclose(fp);
	return 0;
}

void saveConfigToIni()
{
    FILE *fp;
    fp = fopen(CONFIG_FILE, "w");
    int fileIndextemp = 0;
    fseek(fp, 0, SEEK_SET);
    while (fileIndextemp <= fileIndex) {
	fwrite(fileContents[fileIndextemp], strlen(fileContents[fileIndextemp]),
	       1, fp);
	// printf("%s",fileContents[fileIndextemp]);
	fileIndextemp++;
    }
    fclose(fp);
}

void modifyProfile(struct profile *profileList)
{
    int profileIndex;
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    struct profile tempprofile;
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    char *f;
    // show current fprfile setting
    for (profileIndex = 0; profileIndex < 8; profileIndex++) {
	dbg_time(
	    "Fibocom manager \nprofile%d apn:\"%s\" username:\"%s\" password: "
	    "\"%s\" auth: \"%d\" ip family: \"%d\"",
	    profileIndex + 1, profileList[profileIndex].apn,
	    profileList[profileIndex].user, profileList[profileIndex].password,
	    profileList[profileIndex].auth, profileList[profileIndex].ipfamily);
    }
    // select instance to be set
    int connectProfile;
    printf("set profile index:");
    f = malloc(sizeof(f));
    fgets(f,sizeof(f),stdin);
    connectProfile = atoi(f);
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    if(!IsInt(f)){    	
	dbg_time("invalid input");
	return;
    }
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    if (connectProfile < 0 || connectProfile > 8) {
	dbg_time("error profile id");
	return;
    }
    free(f);
    /*
     * Begin: 2020-11-16, Lizhengyang for multi-pdn-manager,
     * Solve the problem of setting length
     * of apn/username/password, mantis 0062495.
     */
    printf("profile apn:");
    f = malloc(MAX_PATH);
    if (f == NULL)
    {
        printf("profile apn f malloc null\n");
        return;
    }
    fgets(f, MAX_PATH, stdin);
    f[strlen(f) - 1] = '\0';
    strcpy(tempprofile.apn, f);
    free(f);

    printf("profile username:");
    f = malloc(MAX_PATH);
    if (f == NULL)
    {
        printf("profile username f malloc null\n");
        return;
    }
    fgets(f, MAX_PATH, stdin);
    f[strlen(f) - 1] = '\0';
    strcpy(tempprofile.user, f);
    free(f);

    printf("profile password:");
    f = malloc(MAX_PATH);
    if (f == NULL)
    {
        printf("profile password f malloc null\n");
        return;
    }
    fgets(f, MAX_PATH, stdin);
    f[strlen(f) - 1] = '\0';
    strcpy(tempprofile.password, f);
    free(f);
    /*
     * End: 2020-11-16, Lizhengyang for multi-pdn-manager,
     * Solve the problem of setting length
     * of apn/username/password, mantis 0062495.
     */
    printf("profile auth[0-3]:");
    f = malloc(sizeof(f));
    fgets(f,sizeof(f),stdin);
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    tempprofile.auth = atoi(f);
    if(!IsInt(f)){    	
	dbg_time("invalid input");
	return;
    }
    if(tempprofile.auth < 0 || tempprofile.auth > 3){       	
	dbg_time("invalid auth type");
	return;
    }
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    free(f);
    
    printf("profile ipfamily[1-3]:");
    f = malloc(sizeof(f));
    fgets(f,sizeof(f),stdin);
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    tempprofile.ipfamily = atoi(f);
    if(!IsInt(f)){    	
	dbg_time("invalid input");
	return;
    }
    free(f);
    if(tempprofile.ipfamily < 1 || tempprofile.ipfamily > 3){       	
	dbg_time("invalid auth type");
	return;
    }
//begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612    
    printf("pin:");
    f = malloc(sizeof(f));
    fgets(f,sizeof(f),stdin);
    f[strlen(f)-1]='\0';
    strcpy(tempprofile.pin, f);
    free(f);
//end modified by zhangkaibo add sim pin operate. mantis 0049142  20200612    
    memset(profileList[connectProfile-1].apn, 0, MAX_PATH);
    memset(profileList[connectProfile-1].user, 0, MAX_PATH);
    memset(profileList[connectProfile-1].password, 0, MAX_PATH);

    strcpy(profileList[connectProfile-1].apn , tempprofile.apn);
	strcpy(profileList[connectProfile-1].user , tempprofile.user);
	strcpy(profileList[connectProfile-1].password , tempprofile.password);
	profileList[connectProfile-1].auth = tempprofile.auth;
	profileList[connectProfile-1].ipfamily = tempprofile.ipfamily;
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
//begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
	//strcpy(profileList[connectProfile].pin , tempprofile.pin);
	strcpy(profileList[connectProfile-1].pin , tempprofile.pin);
//end modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
    // save in
    for (profileIndex = 0; profileIndex < qmap_num; profileIndex++) {
	dbg_time(
	    "Fibocom manager \nprofile%d apn:\"%s\" username:\"%s\" pawssword: "
	    "\"%s\" auth: \"%d\" ip family: \"%d\"",
	    profileIndex + 1, profileList[profileIndex].apn,
	    profileList[profileIndex].user, profileList[profileIndex].password,
	    profileList[profileIndex].auth, profileList[profileIndex].ipfamily);
	char profilename[MAX_PATH];
	sprintf(profilename, "profile%d", profileIndex + 1);
	writeStringVlaue(profilename, "apn", profileList[profileIndex].apn,
			 CONFIG_FILE);
	writeStringVlaue(profilename, "user", profileList[profileIndex].user,
			 CONFIG_FILE);
	writeStringVlaue(profilename, "password",
			 profileList[profileIndex].password, CONFIG_FILE);
	writeIntValue(profilename, "auth", profileList[profileIndex].auth,
		      CONFIG_FILE);
	writeIntValue(profilename, "ipfamily",
		      profileList[profileIndex].ipfamily, CONFIG_FILE);
//begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612			  
	writeStringVlaue(profilename, "pin",
		      profileList[profileIndex].pin, CONFIG_FILE);
//end modified by zhangkaibo add sim pin operate. mantis 0049142  20200612			  
    }
}

void startConnectCM(struct profile *profileList, bool *connectState)
{
    int connectIndex;
    int connectProfile;
    char *f;
    printf("connect visual net interface use:");
    f = malloc(sizeof(f));
    fgets(f,sizeof(f),stdin);
    connectIndex = atoi(f);
    free(f);
    if (connectIndex < 1 || connectIndex > qmap_num) {
	dbg_time("error visual net interface ");
	return;
    }
    printf("connect profile use:");
    f = malloc(sizeof(f));
    fgets(f,sizeof(f),stdin);
    connectProfile = atoi(f);
    free(f);
    if (connectProfile < 1 || connectProfile > 8) {
	dbg_time("error profile id");
	return;
    }
//begin modified by zhangkaibo modify mantis0048806. 20200605
    if (connectState[connectIndex - 1] == true) {
	dbg_time(
	    "visual net interface %d now is connected",
	    connectIndex);
	    return;
    }
//end modified by zhangkaibo modify mantis0048806. 20200605
    dbg_time("Start connect network use instance %d profile %d", connectIndex,
	     connectProfile);
    char cmd[1024];
    //./fibicom-dial -n -s -s apn user password auth -4 -6

    sprintf(cmd, "-n %d -m %d -s %s %s %s %d", connectIndex, connectProfile,
	    profileList[connectProfile - 1].apn,
	    profileList[connectProfile - 1].user,
	    profileList[connectProfile - 1].password,
	    profileList[connectProfile - 1].auth);
    char *argvCmd[30];
    int index = 0;
    // DIAL_PROCESS,","-N","4,""-n","2","-m","1","-s","ctnet","","","","-4","-6","-f","temp.txt",NULL;
    argvCmd[index] = malloc(256);
    sprintf(argvCmd[index++], "%s", DIAL_PROCESS);
    argvCmd[index] = malloc(5);
    sprintf(argvCmd[index++], "-N");
    argvCmd[index] = malloc(256);
    sprintf(argvCmd[index++], "%d", qmap_num);
    argvCmd[index] = malloc(5);
    sprintf(argvCmd[index++], "-n");
    argvCmd[index] = malloc(256);
    sprintf(argvCmd[index++], "%d", connectIndex);
    argvCmd[index] = malloc(5);
    sprintf(argvCmd[index++], "-m");
    argvCmd[index] = malloc(256);
    sprintf(argvCmd[index++], "%d", connectProfile);
    argvCmd[index] = malloc(5);
    sprintf(argvCmd[index++], "-s");
    argvCmd[index] = malloc(256);
    sprintf(argvCmd[index++], "%s", profileList[connectProfile - 1].apn);
    if (strlen(profileList[connectProfile - 1].user) > 0) {
	argvCmd[index] = malloc(256);
	sprintf(argvCmd[index++], "%s", profileList[connectProfile - 1].user);
	argvCmd[index] = malloc(256);
	sprintf(argvCmd[index++], "%s",
		profileList[connectProfile - 1].password);
	argvCmd[index] = malloc(256);
	sprintf(argvCmd[index++], "%d", profileList[connectProfile - 1].auth);
    }

    if (profileList[connectProfile - 1].ipfamily == 2) {
	argvCmd[index] = malloc(5);
//begin modified by zhangkaibo fix ipv6 dial process flow. mantis 0048789 20200605	
	sprintf(argvCmd[index++], "-6");
//end modified by zhangkaibo fix ipv6 dial process flow. mantis 0048789  20200605
    } else if (profileList[connectProfile - 1].ipfamily == 3) {
	argvCmd[index] = malloc(5);
	sprintf(argvCmd[index++], "-4");
	argvCmd[index] = malloc(5);
	sprintf(argvCmd[index++], "-6");
    } else {
	argvCmd[index] = malloc(5);
	sprintf(argvCmd[index++], "-4");
    }
//begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612    
	if (strlen(profileList[connectProfile - 1].pin) > 0) {
    argvCmd[index] = malloc(5);
    sprintf(argvCmd[index++], "-p");
	argvCmd[index] = malloc(256);
	sprintf(argvCmd[index++], "%s", profileList[connectProfile - 1].pin);
    }
    
    argvCmd[index] = malloc(5);
//end modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
    sprintf(argvCmd[index++], "-f");
    argvCmd[index] = malloc(256);
    sprintf(argvCmd[index++], "instance%d.txt", connectIndex);
    argvCmd[index] = NULL;
    char temp[MAX_PATH];
    sprintf(temp, "rm -f instance%d.txt", connectIndex);
    system(temp);
    if (0 == fork()) {
	int *connectPids = (int *)shm;
	connectPids[connectIndex - 1] = getpid();
	dbg_time("exec pid %d", connectPids[connectIndex - 1]);
	char tempcmd[MAX_PATH];
	sprintf(tempcmd, "./%s", DIAL_PROCESS);
	execvp(tempcmd, argvCmd);
	// system(cmd);
//begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
    }
//2021-02-01 willa.liu@fibocom.com changed begin for support mantis 0069837
    //for(int ii=0; ii < index;ii++)
    int ii;
    for(ii = 0;ii < index;ii++)
//2021-02-01 willa.liu@fibocom.com changed end for support mantis 0069837
    {
    	printf("%s ",argvCmd[ii]);
    	free(argvCmd[ii]);
    }
    printf("\r\n");
    connectState[connectIndex - 1] = true;
}

int main(int argc, char *argv[])
{
    struct profile profileList[MAX_QMAP_NUM];
    char *f;
    int profileIndex;
    int opsCode = -1;
    int ret=0;

    /* Exit if the process is already running */
    lock_file = open("/tmp/single_proc.lock", O_CREAT|O_RDWR, 0666);
    ret = flock(lock_file, LOCK_EX|LOCK_NB);
    if (ret)
    {
        if (EWOULDBLOCK == errno)
        {
            dbg_time("The Fibocom multi-pdn-manager is already running! So killall multi-pdn-manager!");
            system("killall -9 fibocom-dial");
            system("killall -9 multi-pdn-manager");
        }
    }
    else
    {
        char buffer[64];
        sprintf(buffer, "pid:%d\n", getpid());
        write(lock_file, buffer, strlen(buffer));
		system("killall -9 fibocom-dial");
        dbg_time("Start Fibocom multi-pdn-manager!\n");
    }

    system("clear");

    get_pcie_mode();
    
    signal(SIGCHLD, SIG_IGN);

    // read from config
    ret = loadConfigFromIni(profileList, connectState);
    if(ret == -1)
    {
        dbg_time("Fibocom multi-pdn-manager %s", VERSION_STRING());
        dbg_time("Fibocom manager current qmap_num is %d,not support multi-pdn", qmap_num);
        dbg_time("please use \"echo 4 > /sys/class/net/usb0/qmap_num\",set driver to multi mode");
        return -1;
    }
    if(ret == -2)
        return -2;
    initShareMemory();
    signal(SIGINT, SIGINT_handle);
    if (is_pcie_dial())
    {
        dbg_time("start qmi proxy server");
        //system("./fibo_qmimsg_server -d /dev/mhi_QMI0 &");
    }
    while (1)
    {
        // show tips
        dbg_time("Fibocom multi-pdn-manager %s", VERSION_STRING());
        dbg_time("Fibocom manager current qmap_num is %d", qmap_num);
        for (profileIndex = 0; profileIndex < qmap_num; profileIndex++)
        {
            dbg_time("Fibocom manager sub-instance%d %s", profileIndex + 1,
                 connectState[profileIndex] ? "Connected" : "Disconnected");
        }
        printf("Please select an action[0-9]\n");
        printf("1.show profile setting\n");
        printf("2.set profile setting\n");
        printf("3.connect to network\n");
        printf("4.disconnect to network\n");
        printf("5.show connect log\n");
        printf("6.reload profile setting\n");
        printf("7.save profile setting\n");
        printf("8.show max instance number\n");
        printf("9.set max instance number\n");
        printf("0.exit\n");

        // get operate
        printf("input [0-9]: ");
        f = malloc(sizeof(f));
        //begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605    
        fgets(f,sizeof(f),stdin);
        if(!IsInt(f))
        {
            dbg_time("invalid input");
            printf("Press any key to continus");
            getchar();
            system("clear");
            continue;
        }
        //end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        opsCode = atoi(f);
        free(f);

        // select operate
        switch (opsCode)
        {
            case 1:
                //begin modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
                for (profileIndex = 0; profileIndex < qmap_num; profileIndex++)
                {
                    dbg_time(
                        "Fibocom manager \nprofile%d apn:\"%s\" username:\"%s\" "
                        "password: \"%s\" auth: \"%d\" ip family: \"%d\" pin:\"%s\"",
                        profileIndex + 1, profileList[profileIndex].apn,
                        profileList[profileIndex].user,
                        profileList[profileIndex].password,
                        profileList[profileIndex].auth,
                        profileList[profileIndex].ipfamily,
                        profileList[profileIndex].pin);
                }
                //end modified by zhangkaibo add sim pin operate. mantis 0049142  20200612
                break;
            case 2:
                {
                    modifyProfile(profileList);
                }
                break;
            case 3:
            {
                startConnectCM(profileList, connectState);
                break;
            }
            case 4:
                {
                    int connectIndex;
                    printf("disconnect visual net interface :");
                    f = malloc(sizeof(f));
                    fgets(f,sizeof(f),stdin);
                    connectIndex = atoi(f);
                    free(f);
                    int *connectPids = (int *)shm;
                    if (connectIndex < 1 || connectIndex > qmap_num)
                    {
                        dbg_time("error visual net interface id");
                        break;
                    }
                    if (connectState[connectIndex - 1] == false)
                    {
                        dbg_time("visual net interface %d now is disconnected", connectIndex);
                        break;
                    }
                    dbg_time("kill pid %d", connectPids[connectIndex - 1]);
                    kill(connectPids[connectIndex - 1], SIGINT);
                    connectState[connectIndex - 1] = false;
                }
                break;
            case 5:
                {
                    int connectIndex;
                    printf("show visual net interface :");
                    f = malloc(sizeof(f));
                    fgets(f,sizeof(f),stdin);
                    connectIndex = atoi(f);
                    free(f);
                    if (connectIndex < 1 || connectIndex > qmap_num)
                    {
                        dbg_time("error visual net interface id");
                        break;
                    }
                    char temp[MAX_PATH];
                    sprintf(temp, "cat instance%d.txt", connectIndex);
                    system(temp);
                }
                break;
            case 6:
                {
                    loadConfigFromIni(profileList, connectState);
                }
                break;
            case 7:
                {
                    saveConfigToIni(profileList);
                }
                break;
            case 8:
                printf("qmap_num : %d", qmap_num);
                break;
            case 9:
                {
                    int temp_qmap;
                    printf("set qmap_num :");
                    f = malloc(sizeof(f));
                    fgets(f,sizeof(f),stdin);
                    temp_qmap = atoi(f);
                    free(f);
                    if (temp_qmap < 1 || temp_qmap > 8)
                    {
                        dbg_time("error qmap_num id");
                        break;
                    }
                    qmap_num = temp_qmap;
                }
                break;
            case 0:
                printf("exit? [Y/N] :");
                f = malloc(sizeof(f));
                fgets(f,sizeof(f),stdin);
                if (f[0] == 'y' || f[0] == 'Y')
                goto __main_exit;
                break;
            default:
                dbg_time("invalid input");
        };
        printf("Press any key to continus");
        getchar();
        system("clear");
    }
__main_exit:
    {
        int *connectPids = (int *)shm;
        int cIndex;
        for(cIndex = 1 ; cIndex <= qmap_num ; cIndex++)
        {
            dbg_time("kill pid %d", connectPids[cIndex - 1]);
            kill(connectPids[cIndex - 1], SIGINT);
            connectState[cIndex - 1] = false;
        }
        //system("ps -ef|grep fibo|grep -v grep|cut -c 9-15|xargs kill");
    }
    destoryShareMemory();
    close(lock_file); // 不要忘记释放文件指针
    return 0;
}
