#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#define NUM_PRODUCT_IDS 5

static uint16_t pololuVendorID = 0x1ffb;
static uint16_t smcProductIDs[NUM_PRODUCT_IDS] = { 0x98, 0x9A, 0x9C, 0x9E, 0xA1 };

enum SmcRequest {
	GetSettings = 0x81,
	SetSettings = 0x82,
	GetVariables = 0x83,
	ResetSettings = 0x84,
	GetResetFlags = 0x85,
	SetSpeed = 0x90,
	ExitSafeStart = 0x91,
	SetMotorLimit = 0x92,
	UsbKill = 0x93,
	StartBootloader = 0xFF
};

enum SmcDirection {
	Forward = 0,
	Reverse = 1,
	Brake = 2,
};




void error(char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

libusb_device_handle *smc_list[2];
int smc_count = 0;

void set_speed(int motor, int speed)
{
	enum SmcDirection dir = Forward;

	if (speed < 0) {
		speed = -speed;
		dir = Reverse;
	}
	// 0x40: host to device, type class, recipient device

	int r = libusb_control_transfer(smc_list[motor], 0x40, SetSpeed, speed, dir, NULL, 0, 5000);
	if (r < 0) error("ERROR: failed to set speed (r = %d)\n", r);
}

void set_speeds(int m1Speed, int m2Speed)
{
	set_speed(0, m1Speed);
	set_speed(1, m2Speed);
}

/*void get_errors(int motor)
{
	int r = libusb_control_transfer(smc_list[motor], 0xC0, GetVariables, 0, 0, &vars, dir, NULL, 0, 5000);
	if (r < 0) error("ERROR: failed to set speed (r = %d)\n", r);
	
}*/

int main()
{
	int r; // return values

	r = libusb_init(NULL);
	if (r < 0) error("ERROR: failed to initialize libusb (r = %d)\n", r);

	libusb_set_debug(NULL, 3);

	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(NULL, &list);
	if (cnt < 0) error("ERROR: failed to get device list (cnt = %d)\n", cnt);


	for (ssize_t i = 0; i < cnt; i++) {
		libusb_device *dev = list[i];
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) error("ERROR: failed to get device descriptor (r = %d)\n", r);

		if (desc.idVendor == pololuVendorID) {

			for (int j = 0; j < NUM_PRODUCT_IDS && smc_count < 2; j++) {

				if (desc.idProduct == smcProductIDs[j]) {
					libusb_device_handle *handle;
					r = libusb_open(dev, &handle);
					if (r < 0) error("ERROR: failed to open device (r = %d)\n", r);

					smc_list[smc_count] = handle;
					smc_count++;

					char buf[100];
					r = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, buf, sizeof(buf));
					if (r < 0) error("ERROR: failed to get serial number string from device (r = %d)\n", r);

					switch(desc.idProduct) {
						case 0x98: printf("18v15"); break;
						case 0x9A: printf("24v12"); break;
						case 0x9C: printf("18v25"); break;
						case 0x9E: printf("24v23"); break;
						case 0xA1: printf("18v7"); break;
					}
					printf(" #%s\n", buf);

					break;
				}
			}
		}
	}

	if (smc_count == 1)
		printf("1 Simple Motor Controller found.\n", smc_count);
	else
		printf("%d Simple Motor Controllers found.\n", smc_count);
		
	printf("Sending ExitSafeStart...\n");
	for (int i = 0; i < smc_count; i++) {
		r = libusb_control_transfer(smc_list[i], 0x40, ExitSafeStart, 0, 0, NULL, 0, 5000);
		if (r < 0) error("ERROR: failed to exit safe start (r = %d)\n", r);
	}

	set_speeds(3200, 3200);
	sleep(4);
	set_speeds(0, 0);

	/*char c;

	while (1)
	{
		scanf("%c", &c);
		fflush(stdin);

		printf("input: %c\n", c);

		switch (c) {
			case 'w':
				set_speeds(3200, 3200);
				break;
			case 'a':
				set_speeds(-3200, 3200);
				break;	
			case 's':
				set_speeds(-3200, -3200);
				break;	
			case 'd':
				set_speeds(3200, -3200);
				break;	
			default:
				set_speeds(0, 0);
		}
		usleep(500000);
	}*/

	for (int i = 0; i < smc_count; i++)
		libusb_close(smc_list[i]);

	libusb_free_device_list(list, 1);
	libusb_exit(NULL);

	return 0;
}

