
CXX = g++ -std=gnu++11
required_libs = -I. -Isensor-gps -Isensor-imu -Isensor-misc -lpthread -pthread -lgps
dest = "/usr/local/bin"

gps_o=sensor-gps/gps_sensor.o
imu_o=sensor-imu/imu_sensor.o 
misc_o=sensor-misc/misc_sensor.o

all: sensor_csv

prepare_dirs:
	@mkdir -p bin

sensor_csv: prepare_dirs $(gps_o) $(imu_o) $(misc_o) sensor_csv.cpp
	$(CXX) $(required_libs) $(local_debug_flags) $(imu_o) $(gps_o) $(misc_o) sensor_csv.cpp -o bin/sensor-csv 

sensor-gps/gps_sensor.o:
	make -C sensor-gps $(remote_debug_flags)

sensor-imu/imu_sensor.o:
	make -C sensor-imu $(remote_debug_flags)

sensor-misc/misc_sensor.o:
	make -C sensor-misc $(remote_debug_flags)

install:
	cp bin/* $(dest)
	@echo "Installed to ${dest}" 

uninstall: 
	@echo "You should probably stop any running pimon programs first."
	@wait rm $(dest)/sensor-csv 2> /dev/null 		&
	@echo "Uninstalled from ${dest}" 


debug: remote_debug_flags = debug
debug: local_debug_flags = -ggdb -O0
debug: all

clean:
	make -C sensor-gps clean
	make -C sensor-wifi clean
	make -C sensor-misc clean
	rm bin/*
