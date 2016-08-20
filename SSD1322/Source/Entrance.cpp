#include "../Include/SSD1322.hpp"

using namespace LiongStudio::RaspiPlayground;
using namespace LiongStudio::RaspiPlayground::Devices;

int main()
{
	SSD1322Info info;
	info.Channel = 0;
	info.ResetPinId = 17;
	info.DcPinId = 27;
	info.MaxClock = 1000000;
	info.VccState = VccState::External;
	SSD1322 dev(info, 256, 64);
	dev.FillScreen(1 << 3);
}