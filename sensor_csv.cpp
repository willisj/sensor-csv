#include <libgpsmm.h>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <gps_sensor.hpp>
#include <imu_sensor.hpp>
#include <misc_sensor.hpp>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//#define ENABLE_GPS 
//#define ENABLE_IMU 
#define ENABLE_MISC 

#include <fstream>

#ifdef ENABLE_GPS
#define GPS_DATA_LABEL  "GPS"
#endif

#ifdef  ENABLE_IMU
#define IMU_DATA_LABEL  "IMU"
#endif

bool useOutputFile = false;
std::string outputFile;
std::ofstream outStream; 
struct imu_data_t newdata;

#ifdef  ENABLE_MISC
char * build_wireshark_string(char *header){
    char base_command[] = "tshark -i wlan0 -I -Eheader=y -Eseparator=,  -Tfields ";
    //char header[] = "frame.time_epoch,radiotap.dbm_antsignal,wlan.fc.type,wlan.fc.subtype,wlan.ra,wlan.da,wlan.ta,wlan_mgt.ssid";
    char * command_buffer = (char *)malloc(2048);
    char * token; 
    size_t command_len;
    
    printf("Header: %s\n",header);
    printf("base_command: %s\n",base_command);
    
    header = strdup(header);
    token = strtok(header,",");
    printf("First Token: %s\n", token);

    strcpy(command_buffer, base_command);
    printf("Got %s\n", header);
    while(token != NULL){
        printf("Token: %s\n",token);
        command_len = strlen(command_buffer);
        if(command_len > 1){
            strcat(command_buffer, " -e");
            strcat(command_buffer, token);  
 //            sprintf(command_buffer + command_len, "-e%s ", token);
        }  
        token = strtok(NULL, ",");
    }
free(header);

    printf("Command: %s\n",command_buffer);
    return command_buffer;
}

void mark_wireshark(misc_res* misc){
    size_t rcv_sz;
    char * data;
    uint8_t snr, wlan_type, wlan_subtype;
    uint32_t rmac, dmac, tmac;
    char * ssid, *token = NULL;
    size_t index = 0;

    rcv_sz = misc->readline((void **)&data, 2048);

    if(rcv_sz <= 0) return;

    printf("{ ");

    token = strtok( data, ",");
    while(token != NULL){

        if( strlen(token) <= 0 )
            break;

        // frame.time_epoch,radiotap.dbm_antsignal,wlan.fc.type,wlan.fc.subtype,wlan.ra,wlan.da,wlan.ta,wlan_mgt.ssid
        switch(index++){
            case 0:
                printf("\"time\"");
                break;
            case 1:
                printf("\"snr\"");
                break;
            case 2:
                printf("\"type\"");
                break;
            case 3:
                printf("\"subtype\"");
                break;
            case 4:
                printf("\"ra\"");
                break;
            case 5:
                printf("\"da\"");
                break;
            case 6:
                printf("\"ta\"");
                break;
            case 7:
                printf("\"ssid\",");
                break;
        }
        printf(":\"%s\", ", token);

        token = strtok(NULL,",\n");
    }
    printf("}\n");
    //free(data);
}
#endif

#ifdef ENABLE_GPS
void mark_gps(gps_res* gps){
	struct gps_data_t newdata;
    gps_fix_t f;

	if (!gps->get_gps_data(&newdata))
        return;

	f = newdata.fix;
	char time_string[300];
	
	
	sprintf(time_string, "%s, %lf, %d, %lf, %lf, %lf, %lf, %lf\n", GPS_DATA_LABEL,
		f.time, newdata.satellites_used,  f.latitude, f.longitude, f.altitude, f.speed, f.track);

	std::cout << time_string; 
	if(useOutputFile){
		outStream << time_string;

	}
}
#endif

#ifdef ENABLE_IMU
void mark_imu(imu_res *imu){
	char imu_string[300];
	
	if (imu->get_imu_data(&newdata)){
		
        sprintf(imu_string, "{\"time\":\"%s\", \"accel-x\":\"%lf\", \"accel-y\":\"%lf\", \"accel-z\":\"%lf\", \"gyro-x\":\"%lf\", \"gyro-y\":\"%lf\", \"gyro-z\":\"%lf\", \"mag-x\":\"%lf\", \"mag-y\":\"%lf\", \"mag-z\":\"%lf\", \"%lf\"}\n", IMU_DATA_LABEL, 
            newdata.timestamp, newdata.accel.x, newdata.accel.y, newdata.accel.z,
            newdata.gyro.x, newdata.gyro.y, newdata.gyro.z,
            newdata.mag.x, newdata.mag.y, newdata.mag.z );
        std::cout << imu_string; 
        if(useOutputFile){
            outStream << imu_string ;
        }
    }
}
#endif

int main(int argc, char *argv[])
{
	char * port = "3000";
	int flags, opt, delay=0;
#ifdef ENABLE_GPS
    std::string csvHeader = "gps_time, sat, lat, lon, alt, imu_time, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z";
    gps_res gps(port);
#endif
#ifdef ENABLE_MISC
	char * command = build_wireshark_string( "frame.time_epoch,radiotap.dbm_antsignal,wlan.fc.type,wlan.fc.subtype,wlan.ra,wlan.da,wlan.ta,wlan_mgt.ssid" );
	misc_res misc(command); 
#endif
    
#ifdef ENABLE_IMU
    imu_res imu(5555); 
#endif

	while ((opt = getopt(argc, argv, "f:d:")) != -1) {
		switch (opt) {
			case 'f':
				outputFile = optarg;
				useOutputFile = true;
				break;
			case 'd':
				delay = atoi(optarg);
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [ -f output_file ][ -d delay ] \n",
						argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if(useOutputFile){
		outStream.open(outputFile);
    }   

    sleep(delay);

	/* || + + + + END OF CONFIGURATION + + + + || */

#ifdef  ENABLE_IMU
	// IMU
	std::cout << "Starting IMU thread" << std::endl;
	imu.thread_imu();
#endif
#ifdef ENABLE_GPS
	// GPS
	std::cout << "Starting GPS thread" << std::endl;
	gps.thread_gps();
    outStream << csvHeader << std::endl;
#endif
    

    for (;;) {
#ifdef ENABLE_GPS
        mark_gps(&gps);
#endif
#ifdef ENABLE_IMU
        mark_imu(&imu);
#endif
#ifdef ENABLE_MISC
        mark_wireshark(&misc);
#endif
        usleep(10000);
    }

	// DONE
	
	if(useOutputFile)
		outStream.close();

	std::cerr << "Exiting." << std::endl;
	return 0;
}

