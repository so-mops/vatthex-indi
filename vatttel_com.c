#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>

/*
 * Read the index {0-63} of of the vatt pkt as
 * an integer. This can later be converted 
 * to a float in arcseconds. 
 *
 * 
  */
int read_sec( char pkt[], int index )
{

	int temp;
        int idxbase = 8 + index*4 + 3;
        if( (index < 0) || (index >= 64) ) {
	    printf("bad index %i", index);
            return 0;
        }
        temp = (int)pkt[idxbase--] & 0xFF;
        temp += ((int)pkt[idxbase--] & 0xFF) * 0x100;
        temp += ((int)pkt[idxbase--] & 0xFF) * 0x10000;
        temp += (int)pkt[idxbase]*0x1000000;
        return temp;
}


/*
 * Populate the cmd_pkt for retrieving the secondary1
 * vatttel server information.
 */
void put_cmd( char cmd_pkt[] )
{
	//VATT_CMD_STATUS_SECONDARY1
	int command = 0x0a13;

	//THe VATT header
	cmd_pkt[0] = 'V';
	cmd_pkt[1] = 'A';
	cmd_pkt[2] = 'T';
	cmd_pkt[3] = 'T';
	
	cmd_pkt[4] = 0; // we won't be using the upper byte for now.  Maybe later
        cmd_pkt[5] = (char)( (command >> 16) & 0xFF ); // LS three bytes are delivered for now
        cmd_pkt[6] = (char)( (command >> 8) & 0xFF );
        cmd_pkt[7] = (char)( command & 0xFF );
}


int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    signed char recvBuff[64];
    signed char pkt[64];
    char rcv_pkt[64];

    //Populate the secondary status request pkt.
    put_cmd(pkt);

    //Socket stuff 
    struct sockaddr_in serv_addr; 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 
	
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    
    serv_addr.sin_family = AF_INET; //address type (IPV4)
    serv_addr.sin_port = htons(1040); //port

    if(inet_pton(AF_INET, "10.0.1.10", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }

    //send the secondary status request pkt.
    write(sockfd, pkt, sizeof(pkt));

    //read the 64 bytes
    n=read(sockfd, recvBuff, sizeof(recvBuff));

	//print a timestamp
	printf("%ld ", time(NULL) );    

	int i;

	//print the values one by one seperated by white space.
	//divide by 3600000.0 to convert to GUI units or arcseconds. 
	for( i=0; i<13; i++ )
	{
		printf( "%f ", read_sec(recvBuff, i)/3600000.0 );
	}
	//give us a new line. 
	printf("\n");
     
    if(n < 0)
    {
        printf("\n Read error \n");
    } 

    return 0;
}


