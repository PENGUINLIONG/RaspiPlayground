#pragma once

namespace LiongStudio
{
	namespace RaspiPlayground
	{
		using std::swap;

		class Spi
		{
		private:
			const int _MosiPinId = 19, _ClockPinId = 23;

			static volatile unsigned int* _Gpio;

			static void InitGpio();

		public:
			enum class PinMode : unsigned char { Input, Output, None };
			enum class PinVoltage { Low = 0, High = 1 };

			Spi(int deviceId);
			Spi(const Spi& instance) = delete;
			~Spi();

			/*
			 * Method:
			 *   Send data to device.
			 * Params:
			 *   $output: Buffer of data to be transmitted.
			 *   $input: Buffer of data to be received.
			 *   $length: The length of $output and $input.
			 * Return: Error code of iotcl(~).
			 * Note:
			 *   The length of these two buffer must be the same.
			 *   $input can be the same buffer as $output.
			 */
			void Transmit(unsigned char data);
			void Transmit(unsigned char* field, size_t length);

			static void SetPinMode(int pinId, Spi::PinMode mode);
			static void SetPinVoltage(int pinId, Spi::PinVoltage voltage);
		};
	}
}
