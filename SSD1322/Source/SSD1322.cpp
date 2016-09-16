#include "../Include/SSD1322.hpp"
#include <thread>
#include <chrono>
#include <iostream>

// The return value will be false if error occurred.
#define _L_MAKE_RV bool rv = false
#define _L_CHECK(x) rv |= !(x)
#define _L_RETURN_ERR return !rv

#define _L_MAKEBYTE(h, l) (h << 4) | (l & 0x0F)

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
				_L_CHECK(SendData(_Bitmap, _Width * _Height));
				EndDrawing();
				
				_L_RETURN_ERR;
			}

			void SSD1322::FillScreen(unsigned char color)
			{
				size_t length = _Width * _Height;
				while (length > 0) _Bitmap[--length] = color;
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
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::Low);
				bool rv = _Spi->Transmit(&cmd, nullptr, 1) >= 0;
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				return rv;
			}

			bool SSD1322::SendData(unsigned char data)
			{
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::Low);
				_Spi->SetPinVoltage(_Info.DcPinId, Spi::PinVoltage::High);
				bool rv = _Spi->Transmit(&data, nullptr, 1) >= 0;
				_Spi->SetPinVoltage(_Info.CsPinId, Spi::PinVoltage::High);
				return rv;
			}
			bool SSD1322::SendData(unsigned char* field, int length)
			{
				_L_MAKE_RV;

				for (int x = 0; x < length;)
					_L_CHECK(SendData(_L_MAKEBYTE(field[x++], field[x++])));

				_L_RETURN_ERR;
			}

			// Private

			void SSD1322::Launch()
			{
				SendCommand(SSD1322_SETCOMMANDLOCK); SendData(0x12);
				SendCommand(SSD1322_DISPLAYOFF);
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