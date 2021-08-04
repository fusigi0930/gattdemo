#include "cgatt.h"
#include <regex>
#include <sstream>
#include <algorithm>
#include <chrono>

#include "debug.h"

#define DEFAULT_MTU_SIZE			20
#define MTU_SIZE(s)					(s+3)

CGatt::CGatt() : m_hAdapter(nullptr),
				m_discovered_cb(nullptr),
				m_hDev(nullptr)
{
	
}

CGatt::~CGatt() {
	close();
}

static bool check_mac_address(char *mac_address) {
	if (nullptr == mac_address)
		return true;

	std::regex pattern(
		"^([0-9A-Fa-f]{2}[:-]){5}"
		"([0-9A-Fa-f]{2})|([0-9a-"
		"fA-F]{4}\\.[0-9a-fA-F]"
		"{4}\\.[0-9a-fA-F]{4})$"
	);

	std::string mac = mac_address;
	if (!regex_match(mac, pattern)) {
		return false;
	}
	return true;
}

void CGatt::scan_devices(const uint8_t* mac, enum blz_addr_type atype, 
		int8_t rssi, const uint8_t *data, size_t len, void *user)
{
	CGatt *gatt = reinterpret_cast<CGatt*>(user);
	if (nullptr == gatt || nullptr == mac || nullptr == data || 0 == len)
		return;

	//DMSG("data leng: %d\n", len);
	//hex_dump("mac: ", mac, 6);
	ble_dev_info *info = reinterpret_cast<ble_dev_info*>(const_cast<uint8_t*>(data));

	std::list<std::string> listUUID;
	int i;
	char *u;
	for (i=0, u = info->serv_uuids; i < info->uuid_count; i++) {
		//DMSG("dev: %s, uuid: %s\n", info->name, u);
		std::string sz = u;
		listUUID.push_back(sz);
		u += 37;
	}

	if (!gatt->m_szServiceUuid.empty()) {
		std::list<std::string>::iterator i = std::find(listUUID.begin(), listUUID.end(), gatt->m_szServiceUuid);
		if (i == listUUID.end())
			return;
	}

	if (nullptr != gatt->m_discovered_cb) {
		gatt->m_discovered_cb(const_cast<char*>(blz_mac_to_string_s(mac)), info->name);
	}
}

bool CGatt::open(char *name) {
	if (nullptr == name) {
		m_hAdapter = blz_init("hci0");
	}
	else {
		m_hAdapter = blz_init(name);
	}

	if (nullptr == m_hAdapter)
		return false;

	return true;
}

void CGatt::close() {
	disconnect();

	if (nullptr != m_hAdapter) {
		blz_fini(m_hAdapter);
		m_hAdapter = nullptr;
	}

}

void CGatt::startPeriphrial() {
	
}

void CGatt::startScan(PFN_DISCOVERED_DEVICE cb) {
	if (nullptr == m_hAdapter)
		return;

	m_discovered_cb = cb;
	m_mutexScan.lock();
	blz_known_devices(m_hAdapter, scan_devices, this);
	blz_scan_start(m_hAdapter, scan_devices, this);
}

void CGatt::stopScan() {
	if (nullptr == m_hAdapter) {
		m_mutexScan.unlock();
		return;
	}

	blz_scan_stop(m_hAdapter);
	m_mutexScan.unlock();
}

bool CGatt::connect(char *addr) {
	if (nullptr == addr || nullptr == m_hAdapter)
		return false;

	disconnect();

	m_hDev = blz_connect(m_hAdapter, addr, BLZ_ADDR_UNKNOWN);

	if (nullptr == m_hDev)
		return false;
	
	return true;
}

bool CGatt::connect(std::string &addr) {
	if (addr.empty())
		return false;

	return this->connect(const_cast<char*>(addr.c_str()));
}

void CGatt::disconnect() {
	if (nullptr != m_hDev) {
		blz_disconnect(m_hDev);
		m_hDev = nullptr;
	}
}

void CGatt::send(char *servuuid, char *charuuid, char *data, int leng) {
	if (nullptr == m_hAdapter || nullptr == m_hDev ||
		nullptr == servuuid || nullptr == charuuid || nullptr == data || 0 == leng) 
	{
		return;	
	}

	blz_serv *serv = blz_get_serv_from_uuid(m_hDev, servuuid);
	if (nullptr == serv) {
		return;
	}

	blz_char *blechar = blz_get_char_from_uuid(serv, charuuid);
	if (nullptr == blechar) {
		return;
	}

	blz_char_write(blechar, reinterpret_cast<uint8_t*>(data), static_cast<size_t>(leng));
}

void CGatt::send(char *servuuid, char *charuuid, std::vector<char> &data) {
	return this->send(servuuid, charuuid, &data[0], data.size());
}

void CGatt::send(char *servuuid, char *charuuid, std::string &data) {
	return this->send(servuuid, charuuid, const_cast<char*>(data.c_str()), data.length());
}

void CGatt::read(char *servuuid, char *charuuid, char *data, int limit) {
	if (nullptr == m_hAdapter || nullptr == m_hDev ||
		nullptr == servuuid || nullptr == charuuid || nullptr == data || 0 == limit) 
	{
		return;	
	}

	blz_serv *serv = blz_get_serv_from_uuid(m_hDev, servuuid);
	if (nullptr == serv) {
		return;
	}

	blz_char *blechar = blz_get_char_from_uuid(serv, charuuid);
	if (nullptr == blechar) {
		return;
	}

	blz_char_read(blechar, reinterpret_cast<uint8_t*>(data), static_cast<size_t>(limit));
}

void CGatt::read(char *servuuid, char *charuuid, std::vector<char> &data) {
	return;
}

void CGatt::registerService(char *uuid) {
	if (nullptr == uuid) {
		m_szServiceUuid.clear();
	}
	else {
		m_szServiceUuid = uuid;
	}
}

void CGatt::registerCharacteristic(char *uuid) {
	if (nullptr == uuid)
		return;
#if 0
	std::string sz = uuid;

	std::list<std::string>::iterator pFind = std::find(m_listCharacteristic.begin(),
													   m_listCharacteristic.end(), sz);
	if (m_listCharacteristic.end() != pFind) {
		return;
	}

	m_listCharacteristic.push_back(sz);
#endif
}

void CGatt::registerDescriptor(char *charuuid, char *descuuid) {
	if (nullptr == charuuid || nullptr == descuuid)
		return;
#if 0
	std::string szChar = charuuid;
	std::string szDesc = descuuid;
	std::map<std::string, std::list<std::string>* >::iterator pFind;
	pFind = m_mapDescriptors.find(szChar);

	if (m_mapDescriptors.end() != pFind) {
		std::list<std::string>::iterator pDesc = std::find(pFind->second->begin(), pFind->second->end(), szDesc);
		if (pFind->second->end() != pDesc) {
			return;
		}

		pFind->second->push_back(szDesc);
	}
	else {
		std::list<std::string> *list = new std::list<std::string>();
		list->push_back(szDesc);
		m_mapDescriptors[szChar] = list;
	}
#endif
}