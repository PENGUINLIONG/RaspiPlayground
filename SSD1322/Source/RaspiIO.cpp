#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <memory>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../Include/RaspiIO.hpp"

/*
*  GPIO setup macros. Always use IN_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
*/
#define IN_GPIO(g) *(_Gpio + ((g) / 10)) &= ~(7 << (((g) % 10) * 3))
#define OUT_GPIO(g) *(_Gpio + ((g) / 10)) |=  (1 << (((g) % 10) * 3))
#define SET_GPIO_ALT(g,a) *(_Gpio + (((g) / 10))) |= (((a) <= 3 ? (a) + 4 : (a) == 4 ? 3 : 2) << (((g) % 10) * 3))

/* sets bits which are 1 ignores bits which are 0 */
#define GPIO_SET *(_Gpio + 7)
/* clears bits which are 1 ignores bits which are 0 */
#define GPIO_CLR *(_Gpio + 10)

namespace LiongStudio
{
	namespace RaspiPlayground
	{
		const auto BCM2835_PERI_BASE = 0x20000000;
		const auto BCM2835_GPIO_BASE = (BCM2835_PERI_BASE + 0x200000); /* GPIO controller */
		const auto BCM2835_SPI_BASE = (BCM2835_PERI_BASE + 0x204000); /* SPI controller */

		const auto BLOCK_SIZE = (4 * 1024);

		volatile unsigned int* Spi::_Gpio = nullptr;
		
		Spi::Spi(int deviceId, int maxClock)
		{
			if (_Gpio == nullptr) InitGpio();

			SetPinMode(_ClockPinId, PinMode::Output);
			SetPinVoltage(_ClockPinId, PinVoltage::High);
			SetPinMode(_MosiPinId, PinMode::Output);
			SetPinVoltage(_MosiPinId, PinVoltage::Low);
		}
		Spi::Spi(Spi&& instance)
		{
			swap(*this, instance);
		}
		Spi::~Spi()
		{
		}

		void Spi::InitGpio()
		{
			int  mem_fd;
			unsigned int* gpio_map;

			/* open /dev/mem */
			if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) throw std::runtime_error("Unable to open /dev/mem.");

			/* mmap GPIO */
			gpio_map = reinterpret_cast<unsigned int*>(mmap(
				nullptr, /* Any adddress in our space will do */
				BLOCK_SIZE, /* Map length */
				PROT_READ | PROT_WRITE, /* Enable reading & writting to mapped memory */
				MAP_SHARED, /* Shared with other processes */
				mem_fd, /* File to map */
				(off_t)BCM2835_GPIO_BASE /* Offset to GPIO peripheral */
			));

			close(mem_fd);

			if (gpio_map == MAP_FAILED) throw std::runtime_error("Unable to map memory: " + std::to_string((size_t)gpio_map + '.'));

			Spi::_Gpio = (volatile unsigned int*)gpio_map;
		}

		void Spi::Transmit(unsigned char data)
		{
			size_t i = 0;
			while (i < 8)
			{
				SetPinVoltage(_ClockPinId, PinVoltage::Low);
				SetPinVoltage(_MosiPinId, (PinVoltage)(data & (1 << i++)));
				SetPinVoltage(_ClockPinId, PinVoltage::High);
			}
		}
		void Spi::Transmit(unsigned char* data, size_t length)
		{
			auto end = data + length;
			while (data < end)
				Transmit(*(data++));
		}

		void Spi::SetPinMode(int pinId, Spi::PinMode mode)
		{
			IN_GPIO(pinId);
			if (mode == Spi::PinMode::Output) OUT_GPIO(pinId);
		}

		void Spi::SetPinVoltage(int pinId, Spi::PinVoltage voltage)
		{
			if (voltage == Spi::PinVoltage::Low) GPIO_CLR = 1 << pinId;
			else GPIO_SET = 1 << pinId;
		}
	}
}
