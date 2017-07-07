#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <devpkey.h>
#include <hidsdi.h>
#include <memory>
#include <iostream>

#include <hidclass.h>
#include "Utils.h"
#include "HidSensor.h"


namespace kat
{
	HidSensor::HidSensor(int _vendorId, int _productId) : vendorId(_vendorId), productId(_productId)
	{
		memset(&readOverlapped, 0, sizeof(OVERLAPPED));
		readOverlapped.hEvent = CreateEvent(0, true, false, nullptr);
		initialize();
	}

	HidSensor::~HidSensor()
	{
		if (handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);
		}
	}

	void HidSensor::initialize()
	{
		GUID hidDeviceClass;
		HidD_GetHidGuid(&hidDeviceClass);
		auto deviceInfoSet = SetupDiGetClassDevs(&hidDeviceClass, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

		if (deviceInfoSet != INVALID_HANDLE_VALUE)
		{
			SP_DEVINFO_DATA deviceInfoData;
			ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
			deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			int deviceIdx = 0;

			while (SetupDiEnumDeviceInfo(deviceInfoSet, deviceIdx, &deviceInfoData))
			{
				++deviceIdx;
				DEVPROPKEY devPropkey = DEVPKEY_Device_InstanceId;
				DEVPROPTYPE propType = DEVPROP_TYPE_STRING;
				DWORD size;
				SetupDiGetDeviceProperty(deviceInfoSet,
					&deviceInfoData,
					&devPropkey,
					&propType,
					NULL,
					0, &size, 0);
				wchar_t* instanceId = new wchar_t[size];
				if (!SetupDiGetDeviceProperty(deviceInfoSet,
					&deviceInfoData,
					&devPropkey,
					&propType,
					(PBYTE)instanceId,
					size, &size, 0) || propType != DEVPROP_TYPE_STRING)
				{
					PrintErrorMessage();
				}
				else
				{
					SP_DEVICE_INTERFACE_DATA interfaceData;
					int memberIndex = 0;

					ZeroMemory(&interfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
					interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

					while (true)
					{
						if (!SetupDiEnumDeviceInterfaces(deviceInfoSet,
							&deviceInfoData,
							&hidDeviceClass,
							memberIndex, &interfaceData))
						{
							int errCode = GetLastError();
							if (errCode == ERROR_NO_MORE_ITEMS)
							{
								break;
							}
							else
							{
								PrintErrorMessage();
							}
						}
						else
						{
							// 获取 Device Path
							DWORD detailSize = 0;
							SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &interfaceData, NULL, 0, &detailSize, &deviceInfoData);

							SP_DEVICE_INTERFACE_DETAIL_DATA* interfaceDetail = new SP_DEVICE_INTERFACE_DETAIL_DATA[detailSize];
							interfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

							if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &interfaceData, interfaceDetail, detailSize, &detailSize, &deviceInfoData))
							{
								PrintErrorMessage();
							}

							std::wstring devPath = reinterpret_cast<wchar_t*>(&interfaceDetail->DevicePath);
							delete interfaceDetail;

							// 获取 Vendor ID 和 Product ID
							HANDLE hdev = CreateFile(devPath.c_str(),
								GENERIC_READ,
								FILE_SHARE_READ,
								0,
								OPEN_EXISTING,
								FILE_FLAG_OVERLAPPED,	// async I/O
								NULL);
							if (hdev == NULL || hdev == INVALID_HANDLE_VALUE)
							{
								//PrintErrorMessage();
								goto Loop_End;
							}
							HIDD_ATTRIBUTES attrs;
							HidD_GetAttributes(hdev, &attrs);
							if (attrs.VendorID == vendorId && attrs.ProductID == productId)
							{
								devicePath = devPath;
								handle = hdev;
								goto Done;
							}
							if (hdev != nullptr && hdev != INVALID_HANDLE_VALUE)
							{
								CloseHandle(hdev);
							}
						}
						Loop_End:
						++memberIndex;
					}
				}
				if (instanceId != NULL)
				{
					delete instanceId;
				}
			}
		Done:
			if (deviceInfoSet)
			{
				SetupDiDestroyDeviceInfoList(deviceInfoSet);
			}
		}
	}

	bool HidSensor::readRawData(BYTE* buf, DWORD bufLen)
	{
		DWORD dataLen;
		auto ret = ReadFile(handle, buf, bufLen, &dataLen, &readOverlapped);
		if (!ret)
		{
			if (GetLastError() != 997) // ERROR_IO_PENDING
				return false;
			else 
			{
				ret = GetOverlappedResult(handle, &readOverlapped, &dataLen, true);
				
			}
		}
		return ret;
	}
}