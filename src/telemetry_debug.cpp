#include <stdio.h>
#include <string.h>
#include <math.h>

#include <libserialport.h> // cross platform serial port lib
//#include "protocol.h"

const char* desired_port = "COM5";

void list_ports() {
  int i;
  struct sp_port **ports;

  sp_list_ports(&ports);
  for (i = 0; ports[i]; i++) {
    printf("Found port: '%s'.\n", sp_get_port_name(ports[i]));
  }

  sp_free_port_list(ports);
}

int main() {
  list_ports();

  printf("Opening port %s \n", desired_port);
  struct sp_port *port;
  sp_get_port_by_name(desired_port,&port);

  sp_open(port,SP_MODE_READ_WRITE );

  return 0;
}
