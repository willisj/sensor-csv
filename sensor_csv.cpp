#include <libgpsmm.h>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <gps_sensor.hpp>
#include <imu_sensor.hpp>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <fstream>


bool useOutputFile = false;
std::string outputFile;
std::ofstream outStream; 
gps_res gps; 
imu_res imu(5555); 
struct imu_data_t newdata;

void print_gps_part(){
	struct gps_data_t newdata;

	gps.get_gps_data(&newdata);
	gps_fix_t f = newdata.fix;
	char time_string[300];
	
	
	sprintf(time_string, "%lf, %d, %lf, %lf, %lf", 
		f.time, newdata.satellites_used,  f.latitude, f.longitude, f.altitude);

	std::cout << time_string; 
	if(useOutputFile){
		outStream << time_string ;

	}
}

void print_imu_part(){
	char imu_string[300];
	
	
	imu.get_imu_data(&newdata);
		
	sprintf(imu_string, "%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
		newdata.timestamp, newdata.accel.x, newdata.accel.y, newdata.accel.z,
		newdata.gyro.x, newdata.gyro.y, newdata.gyro.z,
		newdata.mag.x, newdata.mag.y, newdata.mag.z );

	std::cout << imu_string; 
	if(useOutputFile){
		outStream << imu_string ;
	}
}

int main(int argc, char *argv[])
{
	
	int flags, opt, delay=0;
	std::string csvHeader = "gps_time, sat, lat, lon, alt, imu_time, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z";

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

	sleep(delay);

	/* || + + + + END OF CONFIGURATION + + + + || */

	// GPS

	std::cout << "Starting GPS thread" << std::endl;
	gps.thread_gps();

	// IMU
	
	std::cout << "Starting IMU thread" << std::endl;
	imu.thread_imu();


	std::cout << csvHeader << std::endl;

	if(useOutputFile){
		outStream.open(outputFile);
		outStream << csvHeader << std::endl;
	}

    	for (;;) {
		print_gps_part();
		std::cout << ", ";
		if(useOutputFile) outStream << ", " ;

		print_imu_part();

		std::cout << std::endl;
		if(useOutputFile) outStream << std::endl ;

		usleep(10000);
    	}

	// DONE
	
	if(useOutputFile)
		outStream.close();

	std::cerr << "Exiting." << std::endl;
	return 0;
}

