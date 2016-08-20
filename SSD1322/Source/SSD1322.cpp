#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>

namespace LiongStudio
{
	namespace RaspiPlayground
	{
		namespace Devices
		{
			SSD1322::SSD1322(SSD1322Info info, int width, int height)
				: _Info(info)
				, _Width(width)
				, _Height(height)
				, _Bitmap(new unsigned char[width * height / 2])
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

			void SSD1322::Flush()
			{
				SendCommand(SSD1322_SETCOLUMNADDR);
				SendData(MIN_SEG);
				SendData(MAX_SEG);

				SendCommand(SSD1322_SETROWADDR);
				SendData(0);
				SendData(0x7F);

				SendCommand(SSD1322_WRITERAM);
				SendData(_Bitmap, _Width * _Height / 2);
			}

			void SSD1322::FillScreen(unsigned char color)
			{
				uint8_t x, y;

				SendCommand(SSD1322_SETCOLUMNADDR);
				SendData(MIN_SEG);
				SendData(MAX_SEG);

				SendCommand(SSD1322_SETROWADDR);
				SendData(0);
				SendData(0x7F);

				color = (color & 0x0F) | (color << 4);

				SendCommand(SSD1322_WRITERAM);

				for (y = 0; y < 128; y++)
				{
					for (x = 0; x < 64; x++)
					{
						SendData(color);
						SendData(color);
					}
				}
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
			}

			void SSD1322::SendCommand(unsigned char cmd)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::Low);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->Transmit(&cmd, nullptr, 1);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
			}

			void SSD1322::SendData(unsigned char data)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->Transmit(&data, nullptr, 1);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
			}
			void SSD1322::SendData(unsigned char* field, int length)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::High);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->Transmit(field, nullptr, length);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
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
				SendData(0x14); //Horizontal address increment, Disable Column Address Re-map, Enable Nibble Re-map, Scan from COM[N-1] to COM0, Disable COM Split Odd Even 
				SendData(0x11); //Enable Dual COM mode 

				SendCommand(SSD1322_SETGPIO); // 0xB5 
				SendData(0x00); // Disable GPIO Pins Input 

				SendCommand(SSD1322_FUNCTIONSEL); // 0xAB 
				SendData(0x01); // selection external vdd 

				SendCommand(SSD1322_DISPLAYENHANCE); // 0xB4 
				SendData(0xA0); // enables the external VSL 
				SendData(0xFD); // 0xfFD,Enhanced low GS display quality;default is 0xb5(normal), 

				SendCommand(SSD1322_SETCONTRASTCURRENT); // 0xC1 
				SendData(0xFF); // 0xFF - default is 0x7f 

				SendCommand(SSD1322_MASTERCURRENTCONTROL); // 0xC7 
				SendData(0x0F); // default is 0x0F 

				// Set grayscale 
				SendCommand(SSD1322_SELECTDEFAULTGRAYSCALE); // 0xB9 

				SendCommand(SSD1322_SETPHASELENGTH); // 0xB1 
				SendData(0xE2); // default is 0x74 

				SendCommand(SSD1322_DISPLAYENHANCEB);// 0xD1 
				SendData(0x82); // Reserved; default is 0xa2(normal) 
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
				FillScreen(0x7F);

				SendCommand(SSD1322_DISPLAYON); // 0xAF 
			}

		}
	}
}