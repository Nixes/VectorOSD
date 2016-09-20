#include <stdio.h>
#include <string.h>
#include <math.h>

#include <libserialport.h> // cross platform serial port lib
//#include "protocol.h"

int main() {
  int i;
  struct sp_port **ports;

  sp_list_ports(&ports);
  for (i = 0; ports[i]; i++) {
    printf("Found port: '%s'.\n", sp_get_port_name(ports[i]));
  }
  
  sp_free_port_list(ports);
  return 0;
}
