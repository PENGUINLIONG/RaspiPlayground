#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>
#include <iostream>

#define _L_MAKEBYTE(h, l) (h << 4) | (l & 0x0F)
#define _L_SELF return *this

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
				, _Brightness(1.0)
				, _Spi(new Spi(_Info.Channel, _Info.MaxClock))
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
				, _Brightness(0)
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

			SSD1322& SSD1322::ClearRam()
			{
				BeginDrawing(0, 0, 476, 127);
				auto i = 120 * 128 / 2;
				while (i-- > 0) SendData(0x00);
				EndDrawing();
				_L_SELF;
			}

			SSD1322& SSD1322::Flush()
			{

				BeginDrawing(0, 0, 255, 63);
				SendData(_Bitmap, _Width * _Height);
				EndDrawing();
				_L_SELF;
			}

			SSD1322& SSD1322::GetBitmap(unsigned char*& bitmap, int& width, int& height)
			{
				bitmap = _Bitmap;
				width = _Width;
				height = _Height;
				_L_SELF;
			}

			SSD1322& SSD1322::Reset()
			{
				using namespace std::chrono_literals;

				_Spi->SetPinVoltage(_Info.ResetPinId, Spi::PinVoltage::Low);
				std::this_thread::sleep_for(10ms);
				_Spi->SetPinVoltage(_Info.ResetPinId, Spi::PinVoltage::High);
				Launch();
				_L_SELF;
			}

			SSD1322& SSD1322::SendCommand(unsigned char cmd)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::Low);
				_Spi->Transmit(cmd);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_L_SELF;
			}

			SSD1322& SSD1322::SendData(unsigned char data)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::High);
				_Spi->Transmit(data);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_L_SELF;
			}
			SSD1322& SSD1322::SendData(unsigned char* field, int length)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::High);
				_Spi->Transmit(field, length);
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				_L_SELF;
			}


			// Auxiliary Drawing Methods


			SSD1322& SSD1322::BeginDrawing(int left, int top, int right, int bottom)
			{
				SendCommand(SSD1322_SETCOLUMNADDR);
				SendData(left / 4); SendData(right / 4);

				SendCommand(SSD1322_SETROWADDR);
				SendData(top); SendData(bottom);

				SendCommand(SSD1322_WRITERAM);
				_L_SELF;
			}

			SSD1322& SSD1322::EndDrawing()
			{
				std::this_thread::sleep_for(10ms);
				_L_SELF;
			}



			SSD1322& SSD1322::FillScreen(unsigned char color)
			{
				size_t length = _Width * _Height;
				while (length > 0) _Bitmap[--length] = color;
				_L_SELF;
			}


			// Private
			

			void SSD1322::Launch()
			{
				SendCommand(SSD1322_SETCOMMANDLOCK); SendData(0x12);
				SendCommand(SSD1322_DISPLAYOFF);
				SendCommand(SSD1322_SELECTDEFAULTGRAYSCALE);
				// Front clock is devided by 2, oscilator frequancy is 1001b.
				SendCommand(SSD1322_SETCLOCKDIVIDER); SendData(0x91);
				// SendCommand(SSD1322_SETMUXRATIO); SendData(0x3F); // Duty = 1/64.
				// Disable GPIO Pins Input.
				SendCommand(SSD1322_SETGPIO); SendData(0x00);
				SendCommand(SSD1322_SETMUXRATIO); SendData(_Height - 1);
				// Enables the external VSL, enhance low GS display quality.
				SendCommand(SSD1322_DISPLAYENHANCE); SendData(0xA0); SendData(0xFD);
				SendCommand(SSD1322_MASTERCURRENTCONTROL); SendData((unsigned char)(_Brightness * 255.0));
				SendCommand(SSD1322_SETPHASELENGTH); SendData(0xE2);
				// Precharge voltage: 0.6 x Vcc.
				SendCommand(SSD1322_SETPRECHARGEVOLTAGE); SendData(0x1F);
				SendCommand(SSD1322_SETSECONDPRECHARGEPERIOD); SendData(0x08);
				// High voltage level of common pins: 0.86 x Vcc.
				SendCommand(SSD1322_SETVCOMH); SendData(0x07);
				SendCommand(SSD1322_NORMALDISPLAY);
				SendCommand(SSD1322_EXITPARTIALDISPLAY);
				// Clear down image ram before opening display 
				FillScreen(0x00);
				SendCommand(SSD1322_DISPLAYON);
			}
		}
	}
}