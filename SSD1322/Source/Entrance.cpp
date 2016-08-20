#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>
#include <iostream>

using namespace LiongStudio::RaspiPlayground;
using namespace LiongStudio::RaspiPlayground::Devices;
using namespace std::chrono_literals;

void main()
{
	std::cout << "App started!" << std::endl;
	SSD1322Info info;
	info.Channel = 0;
	info.ResetPinId = 17;
	info.DcPinId = 27;
	info.MaxClock = 1000000;
	std::cout << "Configuration finished! Trying to create instance." << std::endl;
	SSD1322 dev(info, 256, 64);
	dev.FillScreen(1 << 3);
	std::cout << "Waiting for 100s to quit...";
	std::this_thread::sleep_for(100000ms);
}