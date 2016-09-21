#include <stdio.h>
#include <string.h>
#include <math.h>

#include <libserialport.h> // cross platform serial port lib
//#include "protocol.h"

const char* desired_port = "COM5";

void list_ports() {
  int i;
  struct sp_port **ports;

  sp_return error = sp_list_ports(&ports);
  if (error == SP_OK) {
    for (i = 0; ports[i]; i++) {
      printf("Found port: '%s'\n", sp_get_port_name(ports[i]));
    }
    sp_free_port_list(ports);
  } else {
    printf("No serial devices detected\n");
  }
  printf("\n");
}

int main() {
  list_ports();

  printf("Opening port '%s' \n", desired_port);
  struct sp_port *port;
  sp_return error = sp_get_port_by_name(desired_port,&port);
  if (error == SP_OK) {
    sp_set_baudrate(port,9600);
    sp_open(port,SP_MODE_READ_WRITE );


    sp_close(port);
  } else {
    printf("Error opening serial device\n");
  }


  return 0;
}
