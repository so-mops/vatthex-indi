#ifndef NGCLIENT_H
#define NGCLIENT_H
//char SERVER_NAME[] = "QNXTCS";
//int  SERVER_PORT   =  5750;

/*****************************************************/
extern "C" int ng_request(char *input, char *output);
extern "C" int ng_command(char *input, char *output);
#endif
