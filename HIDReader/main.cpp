#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <Windows.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <initguid.h>

#include "HidSensor.h"
#include "Utils.h"
#pragma warning(disable : 4996) // disable CRT security warning

std::mutex stdioMutex;
bool enablePrint = false;
std::ofstream out;

void printHidData(kat::HidSensor& sensor)
{
	while (true)
	{
		uint8_t buf[64] = { 0 };
		auto ret = sensor.readRawData(buf, 64);
		auto time = std::chrono::system_clock::now().time_since_epoch();

		if (!ret)
		{
			kat::PrintErrorMessage();
		}
		
		if (stdioMutex.try_lock())
		{
			out << std::setw(4) << std::setfill('0') 
				<< std::hex << sensor.vendorId << " " << sensor.productId 
				<< " @ " << std::dec << GetTickCount() << "ms  ";
			for (int i = 0; i < 33; ++i)
			{
			
				out << std::setw(2) << std::setfill('0') << std::hex << (int)buf[i] << "  ";
			}
			out << std::endl;
			stdioMutex.unlock();
		}
	}
}

int main()
{
	out = std::ofstream("log.txt");
	kat::HidSensor sensor1(0x0451, 0x160a);
	kat::HidSensor sensor2(0x0451, 0x16b4);

	std::thread th1(printHidData, sensor1);
	std::thread th2(printHidData, sensor2);
	
	th1.join();
	th2.join();

	out.close();
	return 0;
}