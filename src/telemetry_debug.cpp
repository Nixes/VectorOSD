#include <stdio.h>
#include <string.h>
#include <math.h>

#include <unistd.h> // for sleep function

#include <libserialport.h> // cross platform serial port lib
#include "common/mavlink.h"

struct sp_port *port;

void list_ports() {
  int i;
  struct sp_port **ports;

  sp_return error = sp_list_ports(&ports);
  if (error == SP_OK) {
    printf("Valid serial ports:\n");
    for (i = 0; ports[i]; i++) {
      printf("  Found port: '%s'\n", sp_get_port_name(ports[i]));
    }
    sp_free_port_list(ports);
  } else {
    printf("No serial devices detected\n");
  }
  printf("\n");
}


void readMav() {
  mavlink_message_t msg;
  mavlink_status_t status;
  while ( sp_input_waiting(port) > 0) {
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
            mavlink_vfr_hud_t vfr_packet;
            mavlink_msg_vfr_hud_decode(&msg, &vfr_packet);
            printf("Current heading %i\n",vfr_packet.heading);
          break;

          case MAVLINK_MSG_ID_ATTITUDE:
            printf("   Got Attitude\n");
            mavlink_attitude_t attitude_packet;
            mavlink_msg_attitude_decode(&msg, &attitude_packet);
            printf("Pitch %f, Yaw %f, Roll %f\n", attitude_packet.pitch, attitude_packet.yaw, attitude_packet.roll);
          break;

          default:
            printf("   Got unknown %d",msg.msgid);
          //Do nothing
          break;
        }
      }
    }
  }
}

bool openSerialPort(char* port_name) {
  printf("Opening port '%s' \n", port_name);
  sp_return error = sp_get_port_by_name(port_name,&port);
  if (error == SP_OK) {
    sp_set_baudrate(port,57600);
    error = sp_open(port,SP_MODE_READ);
    if (error == SP_OK) {
      return true;
    } else {
      printf("Error opening serial device\n");
      return false;
    }
  } else {
    printf("Error finding serial device\n");
    return false;
  }
}

int main(int argc, char* args[] ) {
  list_ports();

  if (argc < 2) {
    printf("You must specify a serial port to obtain telemetry data\n");
    exit(1);
  }

  if (openSerialPort(args[1]) ) {
    while(true) {

      usleep(2000); // can do something else in mean time, but don't want to consume a whole cpu core
      if (sp_input_waiting(port) > 0){
        printf("Serial checked\n");
      }
      readMav();
      fflush(stdout);
    }

    sp_close(port);
  }


  return 0;
}
