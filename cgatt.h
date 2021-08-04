#ifndef __CGATT_H__
#define __CGATT_H__

#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <map>
#include <vector>

#include "blzlib.h"
#include "blzlib_util.h"

typedef void (*PFN_DISCOVERED_DEVICE) (char *, char *);


class CGatt {
private:
	blz_ctx *m_hAdapter;
	blz_dev *m_hDev;

	std::string m_szServiceUuid;

	std::mutex m_mutexScan;
	std::thread *m_threadScan;
	PFN_DISCOVERED_DEVICE m_discovered_cb;

private:
	static void scan_devices(const uint8_t* mac, enum blz_addr_type atype, int8_t rssi, const uint8_t *data, size_t len, void *user);

public:
	CGatt();
	virtual ~CGatt();
	blz_ctx* instance() { return m_hAdapter; }

	bool open(char *name = nullptr);
	void close();

	void startPeriphrial();
	void startScan(PFN_DISCOVERED_DEVICE cb);
	void stopScan();

	bool connect(char *addr);
	bool connect(std::string addr);
	void disconnect();

	void send(char *servuuid, char *charuuid, char *data, int leng);
	void send(char *servuuid, char *charuuid, std::vector<char> &data);
	void send(char *servuuid, char *charuuid, std::string &data);

	void read(char *servuuid, char *charuuid, char *data, int limit);
	void read(char *servuuid, char *charuuid, std::vector<char> &data);


	void registerService(char *uuid);
	void registerCharacteristic(char *uuid);
	void registerDescriptor(char *charuuid, char *descuuid);
};

#endif