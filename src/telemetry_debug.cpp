#include <stdio.h>
#include <string.h>
#include <math.h>

#include <unistd.h> // for sleep function

#include <libserialport.h> // cross platform serial port lib
#include "common/mavlink.h"

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

  printf("Opening port '%s' \n", desired_port);
  sp_return error = sp_get_port_by_name(desired_port,&port);
  if (error == SP_OK) {
    sp_set_baudrate(port,57600);
    error = sp_open(port,SP_MODE_READ);
    if (error == SP_OK) {
      bool something = true;
      while(something) {

        //usleep(200000); // can do something else in mean time
        if (sp_input_waiting(port) > 0){
          printf("Serial checked\n");
        }
        while ( sp_input_waiting(port) > 0) {
          mavlink_message_t msg;
          mavlink_status_t status;
          //printf("Bytes waiting %i\n", bytes_waiting);
          char byte_buff[1];
          if ( sp_nonblocking_read(port,byte_buff,1) ) {
            if(mavlink_parse_char(MAVLINK_COMM_1, byte_buff[0], &msg, &status)) {
              printf("  Found mavlink packet\n");
              // determine type of message
              switch(msg.msgid) {
          			case MAVLINK_MSG_ID_HEARTBEAT: {
                  printf("   Got Heartbeat\n");
          			}
          			break;

          			case MAVLINK_MSG_ID_VFR_HUD:
                  printf("   Got VFR HUD\n");
          			break;

          			case MAVLINK_MSG_ID_ATTITUDE:
                  printf("   Got Attitude\n");
          			break;

          			default:
                  printf("   Got unknown %d",msg.msgid);
          			//Do nothing
          			break;
          		}
            }
          }
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
