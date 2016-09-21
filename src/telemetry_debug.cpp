#include <stdio.h>
#include <string.h>
#include <math.h>

#include <unistd.h> // for sleep function

#include <libserialport.h> // cross platform serial port lib
//#include "protocol.h"

const char* desired_port = "COM8";

struct sp_port *port;

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
  sleep(1);

  printf("Opening port '%s' \n", desired_port);
  sp_return error = sp_get_port_by_name(desired_port,&port);
  if (error == SP_OK) {
    sp_set_baudrate(port,57600);
    error = sp_open(port,SP_MODE_READ);
    if (error == SP_OK) {
      bool something = true;
      while(something) {
        sleep(1);
        int bytes_waiting = sp_input_waiting(port);
        if (bytes_waiting > 0) {
          printf("Bytes waiting %i\n", bytes_waiting);
          char byte_buff[512];
          int byte_num = 0;
          byte_num = sp_nonblocking_read(port,byte_buff,512);
        }
        fflush(stdout);
      }

      sp_close(port);
    } else {
      printf("Error opening serial device\n");
    }
  } else {
    printf("Error finding serial device\n");
  }


  return 0;
}
