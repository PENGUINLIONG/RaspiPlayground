#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>

using namespace LiongStudio::RaspiPlayground;
using namespace LiongStudio::RaspiPlayground::Devices;
using namespace std::chrono_literals;

int main()
{
	SSD1322Info info;
	info.Channel = 0;
	info.ResetPinId = 17;
	info.DcPinId = 27;
	info.MaxClock = 1000000;
	SSD1322 dev(info, 256, 64);
	dev.FillScreen(1 << 3);
	dev.Flush();
	std::this_thread::sleep_for(100000ms);
}