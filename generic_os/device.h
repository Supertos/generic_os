// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Generic OS Kernel, Device Manager

#include "drv/pci.h"
#include "drv/usb.h"
#include "drv/sata.h"

typedef enum BusType BusType;
typedef struct Device Device;

enum BusType {
	BusPCI,
	BusUSB,
	BusSATA
};

struct Device {
	enum BusType type;
	uint8_t busid;
	uint8_t functionid;
	uint8_t deviceid;
}__attribute__((__packed__));

void InitializeDeviceList( void );

