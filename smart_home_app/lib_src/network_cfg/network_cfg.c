#include "network_cfg.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

char BT_addr[18];

char * get_ip_addr(char *ip_address)
{
	FILE *fp;
	char * popen_cmd = "wpa_cli status";
	char * tmp_buf=NULL;

	if(ip_address == NULL)
	{
		printf("%s: ip_address is NULL.\n",__func__);
		return NULL;
	}
	fp = popen(popen_cmd, "r");
	if(NULL == fp)
	{
		perror("dpi_init: popen wpa_cli status error!\n");
		return NULL;
	}
	tmp_buf = malloc(256);
	if(tmp_buf == NULL)
	{
		printf("%s:iofMalloc err.\n",__func__);
		return NULL;
	}
	memset(tmp_buf,0,256);
	while(fgets(tmp_buf, 256, fp) != NULL)
	{
		printf("fgets: %s\n",tmp_buf);
		if((strstr(tmp_buf, "ip_address="))!=NULL)
		{
            sscanf(tmp_buf, "ip_address=%s", ip_address);
			printf("%s: ip_address is %s.\n",__func__,ip_address);
			if(tmp_buf != NULL)
			{
				free(tmp_buf);
				tmp_buf=NULL;
			}
			pclose(fp);
			return ip_address;
		}
		memset(tmp_buf,0,256);
	}
	if(tmp_buf != NULL)
	{
		free(tmp_buf);
		tmp_buf=NULL;
	}
	pclose(fp);

	return ip_address;
}

//return 1 if wifi connect
//return 0 if wifi disconnect
int check_wifi_status()
{
    char ipAddress[20]={0};
	if (NULL == get_ip_addr(ipAddress))
		return 0;
	else
		return 1;
}

static void write_config_to_file(const char * SSID,const char * passwd)
{
   const char *wifi_config_file ="/data/select.txt";
   FILE *fp; 
   int i;

   fp=fopen(wifi_config_file, "w+");  
   if(fp==NULL)
      puts("File open error");

   fprintf(fp,"%s\n%s\n",SSID,passwd);

   fflush(fp);

   i=fclose(fp); 
   if(i != 0)
     puts("File close error");
}

int set_wifi_config(const char * SSID,const char * passwd)
{
	const char *f2fs_path = "/usr/bin/wifi_setup_bt.sh";
	char cmd[50] = {0};

	// write config to file.
	write_config_to_file(SSID,passwd);

	//set config from file.
	sprintf(cmd,"%s",f2fs_path);
	int status = system(cmd);
	printf("system return: %d\n",status);
	return 0;
}

void start_config_wifi_with_BT(void)
{
	system("/etc/adckey/adckey_function.sh WifiConfig");
}
#define TEST_MAC	"88:83:5D:A4:0C:53"//"80:5E:4F:CC:84:8E"
char* pTestMac= "88:83:5D:A4:0C:53";//"80:5E:4F:CC:84:8E";

#if 0
static int dev_info(char * mac_addr)
{
	#if 0 //for test
	//memcpy(mac_addr, pTestMac, strlen(pTestMac));
	mac_addr = pTestMac;
	#else
	int dev_id;
	bdaddr_t ba;

	dev_id = hci_get_route (NULL);

	hci_devba(dev_id, &ba);

	ba2str(&ba, mac_addr);
	#endif
	
	printf("mac : %s\n",mac_addr);
	return 0;
}

char* get_BT_Mac_addr(void)
{
	dev_info(BT_addr);

	return (char*)BT_addr;
}
#endif

char* get_BT_Mac_addr(void)
{
	printf("pTestMac : %s\n",pTestMac);
	return pTestMac;
}

char * get_bcast_addr(char *bcast_address)
{
 	int inet_sock;
 	struct ifreq ifr;
	int count = 0;
 
 	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
 
 	bzero(&ifr, sizeof(ifr));
 	strcpy(ifr.ifr_name, "wlan0");

	if (ioctl(inet_sock, SIOCGIFBRDADDR, &ifr) < 0)
	perror("ioctl");
	sprintf(bcast_address,"%s", inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	while (strcmp(bcast_address,"0.0.0.0")== 0 && count++ < 10)
	{
		sleep(1);
		if (ioctl(inet_sock, SIOCGIFBRDADDR, &ifr) < 0)
		perror("ioctl");

		memset(bcast_address,0,20);
		sprintf(bcast_address,"%s", inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	}

	return bcast_address;
}

