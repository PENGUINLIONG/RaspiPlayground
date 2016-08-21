#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>
#include <iostream>

#define _L_MAKE_RV bool rv = false
#define _L_CHECK(x) std::cout << !(rv |= !(x))
#define _L_RETURN_ERR return !rv

namespace LiongStudio
{
	namespace RaspiPlayground
	{
		namespace Devices
		{
			using namespace std::chrono_literals;
			
			SSD1322::SSD1322(SSD1322Info info, int width, int height)
				: _Info(info)
				, _Width(width)
				, _Height(height)
				, _Bitmap(new unsigned char[width * height])
				, _Spi(new Spi(("/dev/spidev0." + std::to_string(_Info.Channel)).c_str(), _Info.MaxClock))
			{
				_Spi->SetPinMode(_Info.ResetPinId, Spi::PinMode::Output);
				_Spi->SetPinVoltage(_Info.ResetPinId, Spi::PinVoltage::High);
				_Spi->SetPinMode(_Info.DcPinId, Spi::PinMode::Output);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::Low);
				Launch();
			}
			SSD1322::SSD1322(SSD1322&& instance)
				: _Info()
				, _Width(0)
				, _Height(0)
				, _Bitmap(nullptr)
				, _Spi(nullptr)
			{
				swap(*this, instance);
			}
			SSD1322::~SSD1322()
			{
				if (_Bitmap != nullptr)
				{
					delete[] _Bitmap;
					_Bitmap = nullptr;
				}
			}

			void SSD1322::ClearRam()
			{
				BeginDrawing(0, 0, 479, 127);
				for (int y = 0; y < 64; y++)
				{
					for (int x = 0; x < 128; x++)
					{
						SendData(0x00);
						SendData(0x00);
					}
				}
				EndDrawing();
			}

			bool SSD1322::Flush()
			{
				_L_MAKE_RV;

				BeginDrawing(0, 0, 255, 63);
				_L_CHECK(SendCommand(SSD1322_WRITERAM));
				_L_CHECK(SendData(_Bitmap, _Width * _Height));
				EndDrawing();
				
				_L_RETURN_ERR;
			}

			bool SSD1322::FillScreen(unsigned char color)
			{
				_L_MAKE_RV;

				BeginDrawing(0, 0, 255, 63);
				color = (color & 0x0F) | (color << 4);

				for (int y = 0; y < 64; y++)
				{
					for (int x = 0; x < 128; x++)
					{
						_L_CHECK(SendData(color));
					}
				}
				EndDrawing();

				_L_RETURN_ERR;
			}

			void SSD1322::GetBitmap(unsigned char*& bitmap, int& width, int& height)
			{
				bitmap = _Bitmap;
				width = _Width;
				height = _Height;
			}

			void SSD1322::Reset()
			{
				using namespace std::chrono_literals;

				_Spi->SetPinVoltage(_Info.ResetPinId, Spi::PinVoltage::Low);
				std::this_thread::sleep_for(10ms);
				_Spi->SetPinVoltage(_Info.ResetPinId, Spi::PinVoltage::High);
				Launch();
			}

			bool SSD1322::SendCommand(unsigned char cmd)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::Low);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				bool rv = _Spi->Transmit(&cmd, nullptr, 1) < 0;
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				return rv;
			}

			bool SSD1322::SendData(unsigned char data)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				bool rv = _Spi->Transmit(&data, nullptr, 1) < 0;
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				return rv;
			}
			bool SSD1322::SendData(unsigned char* field, int length)
			{
				_L_MAKE_RV;

				for (int x = 0; x < length;) _L_CHECK(SendData(field[x++]));
								
				_L_RETURN_ERR;
			}

			// Private

			void SSD1322::Launch()
			{
				SendCommand(SSD1322_SETCOMMANDLOCK); // 0xFD 
				SendData(0x12); // Unlock OLED driver IC 

				SendCommand(SSD1322_DISPLAYOFF); // 0xAE 

				SendCommand(SSD1322_SETCLOCKDIVIDER); // 0xB3 
				SendData(0x91); // 0xB3 

				SendCommand(SSD1322_SETMUXRATIO); // 0xCA 
				SendData(0x3F); // duty = 1/64 

				SendCommand(SSD1322_SETDISPLAYOFFSET); // 0xA2 
				SendData(0x00);

				SendCommand(SSD1322_SETSTARTLINE); // 0xA1 
				SendData(0x00);

				SendCommand(SSD1322_SETREMAP); // 0xA0 
				SendData(0x14); // Horizontal address increment, Disable Column Address Re-map, Enable Nibble Re-map, Scan from COM[N-1] to COM0, Disable COM Split Odd Even 
				SendData(0x11); // Enable Dual COM mode 

				SendCommand(SSD1322_SETGPIO); // 0xB5 
				SendData(0x00); // Disable GPIO Pins Input 

				SendCommand(SSD1322_FUNCTIONSEL); // 0xAB 
				SendData(0x01); // selection external vdd 

				SendCommand(SSD1322_DISPLAYENHANCE); // 0xB4 
				SendData(0xA0); // enables the external VSL 
				SendData(0xF8); // 0xfFD,Enhanced low GS display quality; default is 0xb5(normal), 

				SendCommand(SSD1322_SETCONTRASTCURRENT); // 0xC1 
				SendData(0xEF); // 0xFF - default is 0x7f 

				SendCommand(SSD1322_MASTERCURRENTCONTROL); // 0xC7 
				SendData(0x0F); // default is 0x0F 

				// Set grayscale 
				// SendCommand(SSD1322_SELECTDEFAULTGRAYSCALE); // 0xB9 
				SendCommand(SSD1322_SETGRAYSCALETABLE); // Set Gray Scale Table 
				SendData(0x0C);
				SendData(0x18);
				SendData(0x24);
				SendData(0x30);
				SendData(0x3C);
				SendData(0x48);
				SendData(0x54);
				SendData(0x60);
				SendData(0x6C);
				SendData(0x78);
				SendData(0x84);
				SendData(0x90);
				SendData(0x9C);
				SendData(0xA8);
				SendData(0xB4);
 				SendCommand(SSD1322_ENABLEGRAYSCALETABLE);

				SendCommand(SSD1322_SETPHASELENGTH); // 0xB1 
				SendData(0xE2); // default is 0x74 

				SendCommand(SSD1322_DISPLAYENHANCEB);// 0xD1 
				SendData(0xA2); // Reserved; default is 0xa2(normal) 
				SendData(0x20); // 

				SendCommand(SSD1322_SETPRECHARGEVOLTAGE); // 0xBB 
				SendData(0x1F);// 0.6xVcc 

				SendCommand(SSD1322_SETSECONDPRECHARGEPERIOD); // 0xB6 
				SendData(0x08); // default 

				SendCommand(SSD1322_SETVCOMH); // 0xBE 
				SendData(0x07); // 0.86xVcc; default is 0x04 

				SendCommand(SSD1322_NORMALDISPLAY); // 0xA6 

				SendCommand(SSD1322_EXITPARTIALDISPLAY); // 0xA9
				
				// Clear down image ram before opening display 
				FillScreen(0x00);

				SendCommand(SSD1322_DISPLAYON); // 0xAF 
			}

			void SSD1322::BeginDrawing(int left, int top, int right, int bottom)
			{
				SendCommand(SSD1322_SETCOLUMNADDR);
				SendData(left / 4); SendData(right / 4);

				SendCommand(SSD1322_SETROWADDR);
				SendData(top); SendData(bottom);

				SendCommand(SSD1322_WRITERAM);
			}

			void SSD1322::EndDrawing()
			{
				std::this_thread::sleep_for(10ms);
			}
		}
	}
}