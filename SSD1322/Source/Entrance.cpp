#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>
#include <iostream>

using namespace LiongStudio::RaspiPlayground;
using namespace LiongStudio::RaspiPlayground::Devices;
using namespace std::chrono_literals;


void printPattern(SSD1322& dev)
{
	unsigned char* bitmap;
	int width, height;
	dev.GetBitmap(bitmap, width, height);

	auto row = height;
	auto col = width;

	while (row-- > 0)
	{
		while (col-- > 0)
		{
			if ((row % 2) && (col % 2))
				bitmap[row * width + col] = 0xFF;
		}
	}
	std::cout << "Printing returned: " << (bool)dev.Flush() << '.' << std::endl;
}

int main()
{
	std::cout << "App started!" << std::endl;
	SSD1322Info info;
	info.Channel = 0;
	info.ResetPinId = 11;
	info.DcPinId = 13;
	info.CsPinId = 24;
	info.MaxClock = 1000000;
	std::cout << "Configuration finished! Trying to create instance." << std::endl;
	SSD1322 dev(info, 256, 64);

	dev.SendCommand(0xA5);
	std::this_thread::sleep_for(10000ms);
	dev.SendCommand(0xA4);
	std::cout << "Printing result..." << std::endl;
	printPattern(dev);
	std::this_thread::sleep_for(10000ms);
	dev.SendCommand(SSD1322_DISPLAYOFF);
	return 0;
}