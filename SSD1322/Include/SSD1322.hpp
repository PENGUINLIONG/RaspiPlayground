#pragma once
#include <memory>
#include "RespiIO.hpp"

#define SSD1322_LCDWIDTH 256 
#define SSD1322_LCDHEIGHT 64
#define SSD1322_BITS_PER_PIXEL 4

#define SSD1322_SETCOMMANDLOCK 0xFD
#define SSD1322_DISPLAYOFF 0xAE
#define SSD1322_DISPLAYON 0xAF
#define SSD1322_SETCLOCKDIVIDER 0xB3
#define SSD1322_SETDISPLAYOFFSET 0xA2
#define SSD1322_SETSTARTLINE 0xA1
#define SSD1322_SETREMAP 0xA0
#define SSD1322_FUNCTIONSEL 0xAB
#define SSD1322_DISPLAYENHANCE 0xB4
#define SSD1322_SETCONTRASTCURRENT 0xC1
#define SSD1322_MASTERCURRENTCONTROL 0xC7
#define SSD1322_SETPHASELENGTH 0xB1
#define SSD1322_DISPLAYENHANCEB 0xD1
#define SSD1322_SETPRECHARGEVOLTAGE 0xBB
#define SSD1322_SETSECONDPRECHARGEPERIOD 0xB6
#define SSD1322_SETVCOMH 0xBE
#define SSD1322_NORMALDISPLAY 0xA6
#define SSD1322_INVERSEDISPLAY 0xA7
#define SSD1322_SETMUXRATIO 0xCA
#define SSD1322_SETCOLUMNADDR 0x15
#define SSD1322_SETROWADDR 0x75
#define SSD1322_WRITERAM 0x5C
#define SSD1322_ENTIREDISPLAYON 0xA5
#define SSD1322_ENTIREDISPLAYOFF 0xA4
#define SSD1322_SETGPIO 0xB5
#define SSD1322_EXITPARTIALDISPLAY 0xA9
#define SSD1322_SELECTDEFAULTGRAYSCALE 0xB9

#define MIN_SEG	0x1C
#define MAX_SEG	0x5B


namespace LiongStudio
{
	namespace RaspiPlayground
	{
		namespace Devices
		{
			using std::swap;

			struct SSD1322Info
			{
				int Channel = 0;
				int ResetPinId = 0;
				int DcPinId = 0;
				int MaxClock = 0;
			};

			class SSD1322
			{
				friend void swap(SSD1322& a, SSD1322& b)
				{
					swap(a._Info, b._Info);

					swap(a._Bitmap, b._Bitmap);
					swap(a._Width, b._Width);
					swap(a._Height, b._Height);

					swap(a._Spi, b._Spi);
				}
			private:
				SSD1322Info _Info;
				int _Width, _Height;
				unsigned char* _Bitmap;
				std::unique_ptr<Spi> _Spi;

				void Launch();

			public:
				SSD1322(SSD1322Info info, int width, int height);
				SSD1322(const SSD1322& instance) = delete;
				SSD1322(SSD1322&& instance);
				~SSD1322();

				void Flush();
				void FillScreen(unsigned char color);
				void GetBitmap(unsigned char*& bitmap, int& width, int& height);
				void Reset();
				void SendCommand(unsigned char cmd);
				void SendData(unsigned char data);
				void SendData(unsigned char* field, int length);
			};
		}
	}
}