#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <chrono>

#include "debug.h"
#include "cgatt.h"

#define BLE_CHAT_UUID					("0000fff0-0000-1000-8000-00805f9b34fb")
#define BLE_CHAT_MSG_UUID				("0000fff1-0000-1000-8000-00805f9b34fb")
#define BLE_CHAT_CFG_UUID				("0000fff2-0000-1000-8000-00805f9b34fb")

std::string g_mac;

static void discover_device(char *addr, char *name) {
	DMSG("discover device: %s, %s\n", addr, name);
	g_mac = addr;
}

int main(int argc, char* argv[]) {
	CGatt gatt;

	gatt.open();
	gatt.registerService(BLE_CHAT_UUID);
	DMSG("start scan\n");
	gatt.startScan(discover_device);
	DMSG("after start scan\n");

	bool b;
	blz_loop_wait(gatt.instance(), &b, 10000);

	if (g_mac.empty()) {
		DMSG("device not found!!\n");
		return 1;
	}

	gatt.connect(g_mac);

	std::string msg;
	if (1 < argc)
		msg = argv[1];
	else
		msg = "12345678901234567890";

	gatt.send(BLE_CHAT_UUID, BLE_CHAT_MSG_UUID, msg);
	DMSG("done!\n");
	return 0;
}