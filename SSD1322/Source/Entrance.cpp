#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>
#include <iostream>

using namespace LiongStudio::RaspiPlayground;
using namespace LiongStudio::RaspiPlayground::Devices;
using namespace std::chrono_literals;

int main()
{
	std::cout << "App started!" << std::endl;
	SSD1322Info info;
	info.Channel = 0;
	info.ResetPinId = 11;
	info.DcPinId = 13;
	info.CsPinId = 24;
	info.MaxClock = 100000;
	std::cout << "Configuration finished! Trying to create instance." << std::endl;
	SSD1322 dev(info, 256, 64);
	std::cout << "Printing result..." << std::endl;
	dev.FillScreen(0xFF);
	std::cout << "Waiting for 100s to quit..." << std::endl;
	std::this_thread::sleep_for(100000ms);
	return 0;
}