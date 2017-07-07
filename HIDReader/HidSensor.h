#pragma once

#include <Windows.h>
#include <string>
#include <stdexcept>

namespace kat
{
	static constexpr int DEFAULT_VENDOR_ID = 0x0451;
	static constexpr int DEFAULT_PRODUCT_ID = 0x1609;

	class HidSensor
	{
	public:
		HANDLE handle;
		OVERLAPPED readOverlapped;
		
		std::wstring devicePath;
		
		int vendorId, productId;

		void initialize();

	public:
		HidSensor(int _vendorId = DEFAULT_VENDOR_ID, int _productId = DEFAULT_PRODUCT_ID);
		~HidSensor();
		bool readRawData(BYTE* buf, DWORD bufLen);
	};
}