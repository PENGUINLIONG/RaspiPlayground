#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <memory>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "../Include/RespiIO.hpp"

/*
*  GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
*/
#define INP_GPIO(g) *(_Gpio + ((g) / 10)) &= ~(7 << (((g) % 10) * 3))
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
		const auto SPICLKDIV = 32;              /* ~8 Mhz */

		const auto SPIBUFSIZE = 32;              /* SPI buffer size */
		const auto BUFSIZE(SPIBUFSIZE / 4);

		const auto BCM2835_PERI_BASE = 0x20000000;
		const auto BCM2835_GPIO_BASE = (BCM2835_PERI_BASE + 0x200000); /* GPIO controller */
		const auto BCM2835_SPI_BASE = (BCM2835_PERI_BASE + 0x204000); /* SPI controller */

		const auto BCM2835_GPFSEL0 = *(gpio);
		const auto BCM2835_GPFSEL1 = *(gpio + 1);
		const auto BCM2835_GPFSEL2 = *(gpio + 2);

		const auto BCM2835_SPICS = *(spi + 0);
		const auto BCM2835_SPIFIFO = *(spi + 1);
		const auto BCM2835_SPICLK = *(spi + 2);

		const auto SPI_CS_LEN_LONG = 0x02000000;
		const auto SPI_CS_DMA_LEN = 0x01000000;
		const auto SPI_CS_CSPOL2 = 0x00800000;
		const auto SPI_CS_CSPOL1 = 0x00400000;
		const auto SPI_CS_CSPOL0 = 0x00200000;
		const auto SPI_CS_RXF = 0x00100000;
		const auto SPI_CS_RXR = 0x00080000;
		const auto SPI_CS_TXD = 0x00040000;
		const auto SPI_CS_RXD = 0x00020000;
		const auto SPI_CS_DONE = 0x00010000;
		const auto SPI_CS_LEN = 0x00002000;
		const auto SPI_CS_REN = 0x00001000;
		const auto SPI_CS_ADCS = 0x00000800;
		const auto SPI_CS_INTR = 0x00000400;
		const auto SPI_CS_INTD = 0x00000200;
		const auto SPI_CS_DMAEN = 0x00000100;
		const auto SPI_CS_TA = 0x00000080;
		const auto SPI_CS_CSPOL = 0x00000040;
		const auto SPI_CS_CLEAR_RX = 0x00000020;
		const auto SPI_CS_CLEAR_TX = 0x00000010;
		const auto SPI_CS_CPOL = 0x00000008;
		const auto SPI_CS_CPHA = 0x00000004;
		const auto SPI_CS_CS_10 = 0x00000002;
		const auto SPI_CS_CS_01 = 0x00000001;

		const auto BLOCK_SIZE = (4 * 1024);

		volatile unsigned int* Spi::_Gpio = nullptr;
		
		Spi::Spi(std::string deviceName, int maxClock)
			: _Mode(0)
			, _BitsPerWord(8)
			, _Delay(0)
			, _MaxClock(maxClock)
			, _FileDevice(open(deviceName.c_str(), O_RDWR))
		{
			if (_Gpio == nullptr) InitGpio();
			if (_FileDevice < 0) throw std::runtime_error("Unable to open " + deviceName + ".");

			if ((ioctl(_FileDevice, SPI_IOC_WR_MODE, &_Mode) < 0) ||
				(ioctl(_FileDevice, SPI_IOC_RD_MODE, &_Mode) < 0) ||
				(ioctl(_FileDevice, SPI_IOC_WR_BITS_PER_WORD, &_BitsPerWord) < 0) ||
				(ioctl(_FileDevice, SPI_IOC_RD_BITS_PER_WORD, &_BitsPerWord) < 0) ||
				(ioctl(_FileDevice, SPI_IOC_WR_MAX_SPEED_HZ, &_MaxClock) < 0) ||
				(ioctl(_FileDevice, SPI_IOC_RD_MAX_SPEED_HZ, &_MaxClock) < 0))
				throw std::runtime_error("Unable to set up SPI.");
		}
		Spi::Spi(Spi&& instance)
		{
			swap(*this, instance);
		}
		Spi::~Spi()
		{
			close(_FileDevice);
		}

		void Spi::InitGpio()
		{
			int  mem_fd;
			unsigned int* gpio_map;

			/* open /dev/mem */
			if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) throw std::runtime_error("Unable to open /dev/mem.");

			/* mmap GPIO */
			gpio_map = mmap(
				nullptr, /* Any adddress in our space will do */
				BLOCK_SIZE, /* Map length */
				PROT_READ | PROT_WRITE, /* Enable reading & writting to mapped memory */
				MAP_SHARED, /* Shared with other processes */
				mem_fd, /* File to map */
				BCM2835_GPIO_BASE /* Offset to GPIO peripheral */
				);

			close(mem_fd);

			if (gpio_map == MAP_FAILED) throw std::runtime_error("Unable to map memory: " + std::to_string((size_t)gpio_map + '.'));

			Spi::_Gpio = (volatile unsigned int*)gpio_map;
		}

		int Spi::Transmit(unsigned char* output, unsigned char* input, size_t length)
		{
			spi_ioc_transfer spi;

			spi.tx_buf = (unsigned long)output;
			spi.rx_buf = (unsigned long)input;
			spi.len = length;
			spi.delay_usecs = _Delay;
			spi.speed_hz = _MaxClock;
			spi.bits_per_word = _BitsPerWord;

			return ioctl(_FileDevice, SPI_IOC_MESSAGE(1), &spi);
		}

		void Spi::SetPinMode(int pinId, Spi::PinMode mode)
		{
			INP_GPIO(pinId);
			if (mode == Spi::PinMode::Output) OUT_GPIO(pinId);
		}

		void Spi::SetPinVoltage(int pinId, Spi::PinVoltage voltage)
		{
			if (voltage == Spi::PinVoltage::Low) GPIO_CLR = 1 << pinId;
			else GPIO_SET = 1 << pinId;
		}



	}
}
