// BLEConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BLEConsole.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <setupapi.h>
#include <devguid.h>
#include <devpkey.h>
#include <Bluetoothleapis.h>
#include <winerror.h>
#include <vector>

#include "base.h"
#include "BleCentral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// {00002A00-0000-0000-0000-000000000000}
static const GUID BLE_TEST =
{ 0x00002A00, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };


// The one and only application object

CWinApp theApp;

using namespace std;


int InitATLMFC() {
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}
	return nRetCode;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = InitATLMFC();
	BleCentral *bleDev = new BleCentral();
	//Enum all BLE devices
	bleDev->EnumDevice();

	//Get specific Characteristric by UUID
	BleCharacteristic *test_characteristic = bleDev->GetCharacteristicByUUID(BLE_TEST);
	if (test_characteristic != NULL)  {
		HRESULT hr = E_FAIL;
		//Get data from Characteristic
		USHORT data_length = 0;
		int VALUE_SIZE = 10;
		scoped_ptr<BTH_LE_GATT_CHARACTERISTIC_VALUE> value(reinterpret_cast<BTH_LE_GATT_CHARACTERISTIC_VALUE*>(new UINT8[VALUE_SIZE]));
		std::string err_str;
		hr = test_characteristic->GetCharacteristicData(&data_length, value.get(), &err_str);

		//Set data to specific Characteristic
		hr = test_characteristic->SetCharacteristicData(value.get());
	}

	return 0;
}
