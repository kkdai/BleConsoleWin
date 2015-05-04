#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <Bluetoothleapis.h>
#include <SetupAPI.h>

std::string GUID_TO_STRING(const GUID& uuid) {
	std::ostringstream stringStream;
	stringStream << std::hex;
	stringStream << std::setfill('0');
	stringStream << std::setw(8) << uuid.Data1;
	stringStream << "-";
	stringStream << std::setw(4) << uuid.Data2;
	stringStream << "-";
	stringStream << std::setw(4) << uuid.Data3;
	stringStream << "-";
	stringStream << std::setw(2);
	for (int i = 0; i < sizeof(uuid.Data4); i++) {
		if (i == 2)
			stringStream << "-";
		stringStream << static_cast<int>(uuid.Data4[i]);
	}
	return stringStream.str();
}

std::string BTH_LE_UUID_TO_STRING(const BTH_LE_UUID& uuid) {
	if (uuid.IsShortUuid) {
		std::ostringstream stringStream;
		stringStream << std::hex;
		stringStream << std::setfill('0');
		stringStream << "0x" << static_cast<int>(uuid.Value.ShortUuid);
		return stringStream.str();
	}
	else {
		return GUID_TO_STRING(uuid.Value.LongUuid);
	}

}

std::string SERVICE_UUID_TO_STRING(const BTH_LE_UUID& uuid) {
	if (uuid.IsShortUuid) {
		switch (uuid.Value.ShortUuid) {
#define DEFINE_SERVICE(id, name) case id: { \
      std::ostringstream stream; \
      stream << BTH_LE_UUID_TO_STRING(uuid) << " ['" << #name << "']"; \
      return stream.str(); \
				    }
#include "btle_services.h"
#undef DEFINE_SERVICE
		}
	}
	else {
#define DEFINE_SERVICE_LONG(uuid1, uuid2, uuid3, uuid_b1, uuid_b2, uuid_b3, uuid_b4, uuid_b5, uuid_b6, uuid_b7, uuid_b8, name) \
		      { \
        UUID long_uuid = {uuid1, uuid2, uuid3, uuid_b1, uuid_b2, uuid_b3, uuid_b4, uuid_b5, uuid_b6, uuid_b7, uuid_b8}; \
        if (uuid.Value.LongUuid == long_uuid) { \
          std::ostringstream stream; \
          stream << BTH_LE_UUID_TO_STRING(uuid) << " ['" << #name << "']"; \
          return stream.str(); \
				        } \
		      }
#include "btle_services_long.h"
#undef DEFINE_SERVICE_LONG
	}
	return BTH_LE_UUID_TO_STRING(uuid);
}

std::string DESCRIPTOR_UUID_TO_STRING(const BTH_LE_UUID& uuid) {
	if (uuid.IsShortUuid) {
		switch (uuid.Value.ShortUuid) {
#define DEFINE_DESCRIPTOR(id, name) case id: { \
      std::ostringstream stream; \
      stream << BTH_LE_UUID_TO_STRING(uuid) << " ['" << #name << "']"; \
      return stream.str(); \
				    }
#include "btle_descriptors.h"
#undef DEFINE_DESCRIPTOR
		}
	}
	return BTH_LE_UUID_TO_STRING(uuid);
}

bool CheckSuccessulHResult(HRESULT hr, size_t actual_length, size_t expected_length, std::string function_name, std::string* error) {
	if (FAILED(hr)) {
		std::ostringstream string_stream;
		string_stream << "Error calling " << function_name << ", hr=" << hr;
		*error = string_stream.str();
		return false;
	}
	if (actual_length != expected_length) {
		std::ostringstream string_stream;
		string_stream << "Returned length does not match required length when calling " << function_name << "";
		*error = string_stream.str();
		return false;
	}

	return true;
}

bool NoDataResult(HRESULT hr, int length) {
	if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
		return true;

	if (SUCCEEDED(hr) && length == 0)
		return true;

	return false;
}

bool CheckInsufficientBuffer(bool success, std::string function_name, std::string* error) {
	if (success) {
		std::ostringstream string_stream;
		string_stream << "Unexpected successfull call to " << function_name;
		*error = string_stream.str();
		return false;
	}

	DWORD last_error = GetLastError();
	if (last_error != ERROR_INSUFFICIENT_BUFFER) {
		std::ostringstream string_stream;
		string_stream << "Unexpected error from call to " << function_name << ", hr=" << HRESULT_FROM_WIN32(last_error);
		*error = string_stream.str();
		return false;
	}

	return true;
}


