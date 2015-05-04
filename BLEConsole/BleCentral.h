#pragma once

#include "resource.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <setupapi.h>
#include <devguid.h>
#include <devpkey.h>

#include <setupapi.h>
#include <devguid.h>
#include <devpkey.h>
#include <Bluetoothleapis.h>
#include <winerror.h>
#include "base.h"



using namespace std;

class BleCharacteristic {
public:
	HRESULT GetCharacteristicData(USHORT *length, BTH_LE_GATT_CHARACTERISTIC_VALUE *data, std::string* error);
	HRESULT SetCharacteristicData(BTH_LE_GATT_CHARACTERISTIC_VALUE *data);

	BleCharacteristic(HANDLE device_handle, BTH_LE_GATT_CHARACTERISTIC Characteristic) {
		m_deviceHandle = device_handle;
		m_Characteristic = Characteristic;
	}

private:
	HANDLE m_deviceHandle;
	BTH_LE_GATT_CHARACTERISTIC m_Characteristic;
};

class BleCentral {
public:
	BleCentral();
	~BleCentral();

	bool EnumDevice();
	BleCharacteristic* GetCharacteristicByUUID(GUID uuid);

private:
	std::vector<BTH_LE_GATT_CHARACTERISTIC> m_Characteristics;
	HANDLE m_deviceHandle;
};

