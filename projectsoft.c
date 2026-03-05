/*############################################################################
#  Title: projectsoft.c
#  Author: C. Johnson
#  Date: 5/30/24
#  Description: communications routines for projectsoft plc for hexapod
#
#############################################################################*/


#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include "projectsoft.h"
#define PS_MAX 80
#define PS_SA struct sockaddr



/*############################################################################
#  Title: projectsoft_temp_azel(float *el, float *tstrut)
#  Author: C. Johnson
#  Date: 5/30/24
#  Args:  *el = elevation angle
#	*tstrut = float value of strut temp in c
#  Returns: 0 on error, 1 on success   
#  Description: communicates with projectsoft PLC for temp and elevation
#
#############################################################################*/

int projectsoft_el_strut(float *el, float *tstrut)
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    char azel[PS_MAX], temps[PS_MAX], buff[PS_MAX];
    float fazel[2], ftemps[8];
    
   *el=0;
   *tstrut=0;
     // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return PROJECTSOFT_ERROR;
    }
    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(PROJECTSOFT_IP);
    servaddr.sin_port = htons(PROJECTSOFT_PORT);
 
    // connect the client socket to server socket
    if (connect(sockfd, (PS_SA*)&servaddr, sizeof(servaddr))
        != 0) {
        return PROJECTSOFT_ERROR;
    }
 
	//read az/el
        sprintf(buff, "TRAA\n");
        write(sockfd, buff, sizeof(buff));
        bzero(azel, sizeof(azel));
        read(sockfd, azel, sizeof(temps));
 	
	//read temps
        sprintf(buff, "GLTE\n");
        write(sockfd, buff, sizeof(buff));
        bzero(temps, sizeof(temps));
        read(sockfd, temps, sizeof(temps));

    // close the socket
    close(sockfd);

 
    // Keep printing tokens while one of the
    // delimiters present in str[].
    int ctr=0;
    char* token = strtok(azel, " ");
    while (token != NULL) {
	fazel[ctr]=atof(token);
        token = strtok(NULL, " ");
	ctr++;
    }
    if (ctr<2)
	return PROJECTSOFT_ERROR;
 
    ctr=0;
    token = strtok(temps, " ");
    while (token != NULL) {
	ftemps[ctr]=atof(token);
        token = strtok(NULL, " ");
	ctr++;
    }

    if (ctr<8)
	return PROJECTSOFT_ERROR;

    *el=fazel[0];
    *tstrut=(ftemps[1]+ftemps[2])/2;
       // printf("strut temp = %f \n elevation = %f\n", &tstrut, &el);
    return PROJECTSOFT_SUCCESS;



}

