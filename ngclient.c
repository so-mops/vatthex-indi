extern "C"
{
/*############################################################################
#  Title: ngclient.c
#  Author: C. Johnson
#  Date: 12/2/12
#  Description: Contains a set of commands specific for communicating with
#	a TCS-NG server
#
#############################################################################*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* close */
#include "ngindi.h"


int initialize(char *name, int port);

#define MAX_MSG 1500
int indiNetErr(char *in);

  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;
  int sd, rc, initialized=0;

#define mydev              "TCS-NG-INDI"

#define TELID "VATT"
#define SYSID "TCS"

/*############################################################################
#  Title: ng_send()
#  Author: C. Johnson
#  Date: 12/2/12
#  Args:type="COMMAND" or "REQUEST"  
#	input= string to be sent
#	output=return string
#  Description: sends and receives data using the TCS-NG protocol
#
#############################################################################*/
int ng_send(char *type, char *input, char *output)
{
char data[MAX_MSG];
char term = '\n';
char preamble[100];

initialize("10.0.1.10", 5750);

/*	if(!initialized)
	{
	initialize(NET_ADDR, PORT);
	if(!initialized)
		return 0;
	}*/



  strcpy(output,"\0");
  strcpy(preamble,"\0");
  sprintf(preamble,"%s %s 123 ", TELID, SYSID);
  strcpy(data,"\0"); 
  strcpy(data,preamble);
  strncat(data, type, strlen(type));
  strncat(data, " ", 1);
  strncat(data, input, strlen(input));
  strncat(data, &term, 1);
  
 
  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    indiNetErr("cannot connect ");
    return(1);
  }
    rc = send(sd, data, strlen(data) + 1, 0);
    
    if(rc<0) {
	indiNetErr("cannot send data ");
      close(sd);
      return(1);
    
       }

    if (recv(sd,&data,sizeof(data),0) == -1)
      		{
		indiNetErr("recv error");
		return(1);
      		}
    //printf("got: %s\n", data);
    close(sd);

 
strcpy(output, &data[strlen(preamble)]);
      

return 0;
}

/*############################################################################
#  Title: initialize()
#  Author: C. Johnson
#  Date: 12/2/12
#  Args:  name=name or IP address of TCS-NG server 
#	port=port on server to talk to
#  Description: Initializes the network stack to talk to a TCS-NG 
#  server
#
#############################################################################*/
int initialize(char *name, int port)
{
char out[50];
      
h = gethostbyname(name);
  if(h==NULL) {
    sprintf(out, "unknown host '%s'\n",name);
	indiNetErr(out);
    return 1;
  }
    
  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(port);

  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    indiNetErr("cannot open socket ");
    return 1;
  }

  /* bind any port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0) {
    printf("cannot bind port TCP %u\n",port);
    indiNetErr("error ");
    return 1;
  }

initialized=1;
return 0;
}

/*############################################################################
#  Title: ng_request()
#  Author: C. Johnson
#  Date: 12/2/12
#  Args:  input=request string to be sent
#	output=return string
#  Description: sends a request to the TCS and populates a string with the 
#        return
#
#############################################################################*/
int ng_request(char *input, char *output)
{
char type[] = "REQUEST";
int err;
err = ng_send(type, input, output);
//if(output!=NULL);
//	return(strlen(output));
	return(err);
//return 0;
}

/*############################################################################
#  Title: ng_command()
#  Author: C. Johnson
#  Date: 12/2/12
#  Args:  input=command string to be sent
#	output=return string
#  Description: sends a command to the TCS and populates a string with the 
#        return
#
#############################################################################*/
int ng_command(char *input, char *output)
{
char type[] = "COMMAND";
int err;
err = ng_send(type, input, output);
//if(output!=NULL);
	return(err);
//return 0;

}

/*############################################################################
#  Title: ngatof()
#  Author: C. Johnson
#  Date: 12/2/12
#  Args:  in=char ptr to input string containing ascii degrees in "D:M:S.s"
#	out=float describing degrees.
#  Description: converts an ascii "D:M:S.s" string to a float "D.d"
#
#############################################################################*/
int ngatof(char *in, double *out)
{
double H, M, S;
sscanf(in, "%lf:%lf:%lf", &H, &M, &S);
*out = H + (M/60) + (S/3600);
return 0;
}

/*############################################################################
#  Title: ngdegtoa()
#  Author: C. Johnson
#  Date: 12/2/12
#  Args:  in=char ptr to input string containing ascii degrees in NG format
#	out=char ptr to ouput string containing ascii degrees in DD:MM:SS.ss
#  Returns: always returns 0	
#  Description: converts the NG style DMS string to a ':' delimited string
#
#############################################################################*/
int ngdegtoa(char *in, char *out)
{
char temp[20];
int iH, iM, iS, is;
double M, S, s;
sscanf(in, "%i.%i", &iH, &is);
sprintf(temp, "%06i", iH);
sscanf(temp, "%2i%2i%2i", &iH, &iM, &iS);
sprintf(out, "%02i:%02i:%02i.%2i", iH, iM, iS, is);
return 0;
}

/*############################################################################
#  Title: indiNetErr()
#  Author: C. Johnson
#  Date: 9/2/15
#  Args:  in=char ptr to input string containing eror
#	
#  Returns: always returns 0	
#  Description: converts the NG style DMS string to a ':' delimited string
#
#############################################################################*/
int indiNetErr(char *in)
{
char buff[50];
sprintf(buff, "NET:%s", in);
return 0;
}


}
