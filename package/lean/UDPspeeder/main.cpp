#include "common.h"
#include "log.h"

#include "lib/rs.h"
#include "packet.h"
#include "connection.h"
#include "fd_manager.h"
#include "delay_manager.h"
#include "fec_manager.h"
#include "misc.h"
#include "tunnel.h"
//#include "tun_dev.h"
#include "git_version.h"
using namespace std;


static void print_help()
{
	char git_version_buf[100]={0};
	strncpy(git_version_buf,gitversion,10);

	printf("UDPspeeder V2\n");
	printf("git version: %s    ",git_version_buf);
	printf("build date: %s %s\n",__DATE__,__TIME__);
	printf("repository: https://github.com/wangyu-/UDPspeeder\n");
	printf("\n");
	printf("usage:\n");
	printf("    run as client: ./this_program -c -l local_listen_ip:local_port -r server_ip:server_port  [options]\n");
	printf("    run as server: ./this_program -s -l server_listen_ip:server_port -r remote_ip:remote_port  [options]\n");
	printf("\n");
	printf("common options, must be same on both sides:\n");
	printf("    -k,--key              <string>        key for simple xor encryption. if not set, xor is disabled\n");

	printf("main options:\n");
	printf("    -f,--fec              x:y             forward error correction, send y redundant packets for every x packets\n");
	printf("    --timeout             <number>        how long could a packet be held in queue before doing fec, unit: ms, default: 8ms\n");
	printf("    --report              <number>        turn on send/recv report, and set a period for reporting, unit: s\n");

	printf("advanced options:\n");
	printf("    --mode                <number>        fec-mode,available values: 0,1; mode 0(default) costs less bandwidth,no mtu problem.\n");
	printf("                                          mode 1 usually introduces less latency, but you have to care about mtu.\n");
	printf("    --mtu                 <number>        mtu. for mode 0, the program will split packet to segment smaller than mtu value.\n");
	printf("                                          for mode 1, no packet will be split, the program just check if the mtu is exceed.\n");
	printf("                                          default value: 1250. you typically shouldnt change this value.\n");
    printf("    -q,--queue-len        <number>        fec queue len, only for mode 0, fec will be performed immediately after queue is full.\n");
	printf("                                          default value: 200. \n");
	printf("    -j,--jitter           <number>        simulated jitter. randomly delay first packet for 0~<number> ms, default value: 0.\n");
	printf("                                          do not use if you dont know what it means.\n");
	printf("    -i,--interval         <number>        scatter each fec group to a interval of <number> ms, to protect burst packet loss.\n");
	printf("                                          default value: 0. do not use if you dont know what it means.\n");
	printf("    --random-drop         <number>        simulate packet loss, unit: 0.01%%. default value: 0.\n");
	printf("    --disable-obscure     <number>        disable obscure, to save a bit bandwidth and cpu.\n");
//	printf("    --disable-xor         <number>        disable xor\n");

	printf("developer options:\n");
	printf("    --fifo                <string>        use a fifo(named pipe) for sending commands to the running program, so that you\n");
	printf("                                          can change fec encode parameters dynamically, check readme.md in repository for\n");
	printf("                                          supported commands.\n");
	printf("    -j ,--jitter          jmin:jmax       similiar to -j above, but create jitter randomly between jmin and jmax\n");
	printf("    -i,--interval         imin:imax       similiar to -i above, but scatter randomly between imin and imax\n");
    printf("    --decode-buf          <number>        size of buffer of fec decoder,u nit: packet, default: 2000\n");
    printf("    --fix-latency         <number>        try to stabilize latency, only for mode 0\n");
    printf("    --delay-capacity      <number>        max number of delayed packets\n");
	printf("    --disable-fec         <number>        completely disable fec, turn the program into a normal udp tunnel\n");
	printf("    --sock-buf            <number>        buf size for socket, >=10 and <=10240, unit: kbyte, default: 1024\n");
	printf("log and help options:\n");
	printf("    --log-level           <number>        0: never    1: fatal   2: error   3: warn \n");
	printf("                                          4: info (default)      5: debug   6: trace\n");
	printf("    --log-position                        enable file name, function name, line number in log\n");
	printf("    --disable-color                       disable log color\n");
	printf("    -h,--help                             print this help message\n");

	//printf("common options,these options must be same on both side\n");
}



int main(int argc, char *argv[])
{
	//working_mode=tunnel_mode;

	assert(sizeof(u64_t)==8);
	assert(sizeof(i64_t)==8);
	assert(sizeof(u32_t)==4);
	assert(sizeof(i32_t)==4);
	assert(sizeof(u16_t)==2);
	assert(sizeof(i16_t)==2);
	dup2(1, 2);		//redirect stderr to stdout
	int i, j, k;

	if (argc == 1)
	{
		print_help();
		myexit( -1);
	}
	for (i = 0; i < argc; i++)
	{
		if(strcmp(argv[i],"-h")==0||strcmp(argv[i],"--help")==0)
		{
			print_help();
			myexit(0);
		}
	}


	process_arg(argc,argv);

	delay_manager.set_capacity(delay_capacity);
	local_ip_uint32=inet_addr(local_ip);
	remote_ip_uint32=inet_addr(remote_ip);
	sub_net_uint32=inet_addr(sub_net);

	if(strlen(tun_dev)==0)
	{
		sprintf(tun_dev,"tun%u",get_true_random_number()%1000);
	}

	if(client_or_server==client_mode)
	{
		tunnel_client_event_loop();
	}
	else
	{
		tunnel_server_event_loop();
	}

	return 0;
}

