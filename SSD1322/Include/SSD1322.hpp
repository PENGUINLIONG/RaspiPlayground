#pragma once
#include <memory>
#include "RaspiIO.hpp"

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
#define SSD1322_ENABLEGRAYSCALETABLE 0x00
#define SSD1322_SETGRAYSCALETABLE 0xB8

#define MIN_SEG	0x0
#define MAX_SEG	0x77


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
				int CsPinId = 0;
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
				float _Brightness;
				std::unique_ptr<Spi> _Spi;

				void Launch();

			public:
				SSD1322(SSD1322Info info, int width, int height);
				SSD1322(const SSD1322& instance) = delete;
				SSD1322(SSD1322&& instance);
				~SSD1322();

				// Method:
				//   Clear the buffer on SSD1322 device with color 0x00.
				// Note:
				//   This method will directly clear the entire buffer on SSD1322 device with the buffer on host device not modified.
				//   Should be distinguished from FillScreen(0x00).
				SSD1322& ClearRam();
				// Method:
				//   Write the data in host buffer to SSD1322 device buffer.
				// Return:
				//   True if an error occurred, false otherwise.
				SSD1322& Flush();

				// Method:
				//   Get the information about the buffer on host device.
				// Params:
				//   $bitmap: [OUT] Pointer to the buffer.
				//   $width: [OUT] Width of the client area.
				//   $height: [OUT] Height of the client area.
				SSD1322& GetBitmap(unsigned char*& bitmap, int& width, int& height);
				// Method:
				//   Reset the SSD1322 device.
				SSD1322& Reset();
				// Method:
				//   Send command of one byte to the SSD1322 device.
				// Params:
				//   $data: Command to be transmitted.
				SSD1322& SendCommand(unsigned char cmd);
				// Method:
				//   Send data of one byte to the SSD1322 device.
				// Params:
				//   $data: Data to be transmitted.
				SSD1322& SendData(unsigned char data);
				// Method:
				//   Send a data sequence to the SSD1322 device.
				// Params:
				//   $field: The data sequence to be transmitted.
				//   $length: Length of $field.
				SSD1322& SendData(unsigned char* field, int length);


				// Auxiliary Drawing Methods


				// Method:
				//   Begin a drawing session to device.
				// Params:
				//   $~: Boundaries of drawable client area.
				SSD1322& BeginDrawing(int left, int top, int right, int bottom);
				// Method:
				//   End up a drawing session to device.
				//   Currently do nothing but sleep for 10ms.
				// Params:
				//   $~: Boundaries of drawable client area.
				SSD1322& EndDrawing();

				// Method:
				//   Fill the entire buffer with the specified color.
				// Params:
				//   $color: Color used to fill the buffer.
				SSD1322& FillScreen(unsigned char color);
			};
		}
	}
}