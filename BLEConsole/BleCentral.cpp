#include "stdafx.h"
#include "BleCentral.h"
#include "btle_helpers.h"

using namespace std;

BleCentral::BleCentral() {

}

BleCentral::~BleCentral() {

}

bool BleCentral::EnumDevice() {
	HDEVINFO handle_ = INVALID_HANDLE_VALUE;
	GUID BluetoothClassGUID = GUID_BLUETOOTHLE_DEVICE_INTERFACE;
	HDEVINFO result = SetupDiGetClassDevs(
		&BluetoothClassGUID,
		NULL,
		NULL,
		//DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE));
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (result == INVALID_HANDLE_VALUE) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (handle_ != INVALID_HANDLE_VALUE) {
		SetupDiDestroyDeviceInfoList(handle_);
		handle_ = INVALID_HANDLE_VALUE;
	}
	handle_ = result;

	GUID BluetoothInterfaceGUID = GUID_BLUETOOTHLE_DEVICE_INTERFACE;

	std::vector<SP_DEVICE_INTERFACE_DATA> ble_interfaces;
	std::vector<std::wstring> ble_paths;

	// Enumerate device of LE_DEVICE interface class
	for (int i = 0;; i++) {
		std::string error;
		SP_DEVICE_INTERFACE_DATA device_interface_data = { 0 };
		device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		BOOL success = SetupDiEnumDeviceInterfaces(
			handle_,
			NULL,
			(LPGUID)&BluetoothInterfaceGUID,
			(DWORD)i,
			&device_interface_data);
		if (!success) {
			DWORD last_error = GetLastError();
			if (last_error == ERROR_NO_MORE_ITEMS) {
				//Enum devices finished.
				break;
			}
			else {
				//"Error enumerating device interfaces: " << last_error;				
				return FALSE;
			}
		}

		// Retrieve required # of bytes for interface details
		ULONG required_length = 0;
		success = SetupDiGetDeviceInterfaceDetail(
			handle_,
			&device_interface_data,
			NULL,
			0,
			&required_length,
			NULL);
		if (!CheckInsufficientBuffer(success, "SetupDiGetDeviceInterfaceDetail", &error))
			return FALSE;

		scoped_array<UINT8> interface_data(new UINT8[required_length]);
		RtlZeroMemory(interface_data.get(), required_length);

		PSP_DEVICE_INTERFACE_DETAIL_DATA device_interface_detail_data = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(interface_data.get());
		device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		SP_DEVINFO_DATA device_info_data = { 0 };
		device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

		ULONG actual_length = required_length;
		success = SetupDiGetDeviceInterfaceDetail(
			handle_,
			&device_interface_data,
			device_interface_detail_data,
			actual_length,
			&required_length,
			&device_info_data);

		//Store data
		std::wstring strpath = std::wstring(device_interface_detail_data->DevicePath);
		OLECHAR* bstrGuid;
		StringFromCLSID(device_interface_data.InterfaceClassGuid, &bstrGuid);
		std::wcout << "  Path: " << strpath << " GUID:" << bstrGuid << std::endl;
		ble_paths.push_back(strpath);
		ble_interfaces.push_back(device_interface_data);
	}

	//Select device to open
	std::wstring path = ble_paths[0];

	//Open File
	m_deviceHandle = CreateFile(path.c_str(), GENERIC_WRITE | GENERIC_READ, NULL, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_deviceHandle == INVALID_HANDLE_VALUE) {
		DWORD last_error = GetLastError();
		std::ostringstream string_stream;
		std::wcout << "Error opening device: " << last_error << std::endl;
		return -1;
	}

	//Bluetooth GATT API..
	USHORT required_count;
	// Get GATT Services Count
	HRESULT hr = BluetoothGATTGetServices(m_deviceHandle, 0, NULL, &required_count, BLUETOOTH_GATT_FLAG_NONE);
	if (NoDataResult(hr, required_count)) {
		return true;
	}

	if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
		std::ostringstream string_stream;
		std::wcout << "Unexpected return value from BluetoothGATTGetServices: " << hr;
		return FALSE;
	}

	//Get GATT Services
	std::string error;
	scoped_array<BTH_LE_GATT_SERVICE> services(new BTH_LE_GATT_SERVICE[required_count]);
	USHORT actual_count = required_count;
	hr = BluetoothGATTGetServices(m_deviceHandle, actual_count, services.get(), &required_count, BLUETOOTH_GATT_FLAG_NONE);
	if (!CheckSuccessulHResult(hr, actual_count, required_count, "BluetoothGATTGetServices", &error))
		return false;

	for (int i = 0; i < actual_count; i++) {
		BTH_LE_GATT_SERVICE& service(services.get()[i]);
		std::cout << "  Service GUID:" << SERVICE_UUID_TO_STRING(service.ServiceUuid) << std::endl;


		//Get GATT Characteristics
		USHORT required_count;
		HRESULT hr = BluetoothGATTGetCharacteristics(m_deviceHandle, &service, 0, NULL, &required_count, BLUETOOTH_GATT_FLAG_NONE);
		if (NoDataResult(hr, required_count)) {
			return true;
		}

		if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
			std::ostringstream string_stream;
			std::wcout << "Error getting characteristics" << std::endl;
			return false;
		}

		scoped_array<BTH_LE_GATT_CHARACTERISTIC> gatt_characteristics(new BTH_LE_GATT_CHARACTERISTIC[required_count]);
		USHORT actual_count = required_count;
		hr = BluetoothGATTGetCharacteristics(m_deviceHandle, &service, actual_count, gatt_characteristics.get(), &required_count, BLUETOOTH_GATT_FLAG_NONE);
		if (!CheckSuccessulHResult(hr, actual_count, required_count, "BluetoothGATTGetCharacteristics", &error))
			return false;

		//Enum Characteristics
		for (int i = 0; i < actual_count; i++) {
			BTH_LE_GATT_CHARACTERISTIC& gatt_characteristic(gatt_characteristics.get()[i]);
			m_Characteristics.push_back(gatt_characteristic);
			std::cout << "Characteristic GUID:" << SERVICE_UUID_TO_STRING(gatt_characteristic.CharacteristicUuid) << std::endl;
		}
	}

return true;
}

BleCharacteristic* BleCentral::GetCharacteristicByUUID(GUID uuid) {
	bool found = false;
	BleCharacteristic* retObj = NULL;

	for (BTH_LE_GATT_CHARACTERISTIC &obj : m_Characteristics) {
		if (obj.CharacteristicUuid.Value.LongUuid == uuid) {
			found = true;
			retObj = new BleCharacteristic(m_deviceHandle, obj);
			break;
		}
	}
	return retObj;	
}

HRESULT BleCharacteristic::GetCharacteristicData(USHORT *length, BTH_LE_GATT_CHARACTERISTIC_VALUE *data, std::string* error) {
	USHORT required_length = 0;
	HRESULT hr = BluetoothGATTGetCharacteristicValue(
		&m_deviceHandle,
		&m_Characteristic,
		0,
		NULL,
		length,
		BLUETOOTH_GATT_FLAG_NONE);
	if (NoDataResult(hr, required_length)) {
		return true;
	}

	if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
		std::ostringstream string_stream;
		cout << "Error getting characteristic value";
		return E_FAIL;
	}

	scoped_ptr<BTH_LE_GATT_CHARACTERISTIC_VALUE> value(reinterpret_cast<BTH_LE_GATT_CHARACTERISTIC_VALUE*>(new UINT8[required_length]));
	RtlZeroMemory(value.get(), required_length);
	value.get()->DataSize = required_length;

	USHORT actual_length = required_length;
	hr = BluetoothGATTGetCharacteristicValue(
		m_deviceHandle,
		&m_Characteristic,
		actual_length,
		value.get(),
		&required_length,
		BLUETOOTH_GATT_FLAG_NONE);
	if (!CheckSuccessulHResult(hr, actual_length, required_length, "BluetoothGATTGetCharacteristicValue", error))
		return E_FAIL;

	data = value.get();
	return S_OK;
}

HRESULT BleCharacteristic::SetCharacteristicData(BTH_LE_GATT_CHARACTERISTIC_VALUE *data) {

	HRESULT hr = BluetoothGATTSetCharacteristicValue(
		m_deviceHandle,
		&m_Characteristic,
		data,
		NULL,
		BLUETOOTH_GATT_FLAG_NONE);
	if (FAILED(hr)) {
		std::ostringstream string_stream;
		cout << "Error calling BluetoothGATTSetCharacteristicValue: hr=" << hr;
		return E_FAIL;
	}

	return hr;
}
