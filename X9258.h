/*
 * Copyright 2017 Hedde Bosman
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __X9258_H__
#define __X9258_H__

#if (ARDUINO >= 100)
	#include <Arduino.h>
#else
	#include <WProgram.h>
	#include <pins_arduino.h>
#endif


#define X9258_DEVICE_TYPE	0x50

#define X9258_READ_WIPER					0x90
#define X9258_WRITE_WIPER					0xA0
#define X9258_READ_MEMORY					0xB0
#define X9258_WRITE_MEMORY					0xC0
#define X9258_XFR_MEMORY_TO_WIPER			0xD0
#define X9258_XFR_WIPER_TO_MEMORY			0xE0
#define X9258_GLOBAL_XFR_MEMORY_TO_WIPER	0x10
#define X9258_GLOBAL_XFR_WIPER_TO_MEMORY	0x80
#define X9258_INC_DEC_WIPER					0x20


class X9258 {
	private:
		const uint8_t X9258_SCL_PIN_NUM;
		const uint8_t X9258_SDA_PIN_NUM;

		uint8_t device_address; /**< The device address (4 bits) */
		
		// functions for software twi
		/**
		 * \brief  send a clock signal
		 */
		void send_clock();

		/**
		 * \brief  wait for an ACK from the slave
		 */
		uint8_t wait_for_ack();

		/**
		 * \brief  Send the start condition
		 */
		void send_start();

		/**
		 * \brief  Send the stop condition
		 */
		void send_stop();

		/**
		 * \brief  Send data
		 */
		uint8_t send_data(const uint8_t data);

		/**
		 * \brief  Send an ACK
		 */
		void send_ack();

		/**
		 * \brief  Receive data
		 */
		uint8_t recv_data();


		/**
		 * \brief  Write an instruction
		 */
		void write_instruction(const uint8_t instruction);

		/**
		 * \brief Write a value with the given instruction (data register)
		 */
		void write_value(const uint8_t instruction, const uint8_t value);

		/**
		 * \brief Read a value from the given instruction (data register)
		 */
		uint8_t read_value(const uint8_t instruction);
		
	public:
#if defined(X9258_SCL_PIN_NUM) && defined(X9258_SDA_PIN_NUM)
		X9258();
		X9258(uint8_t device_address);
#else
		X9258(const uint8_t i2c_scl, const uint8_t i2c_sda);
		X9258(const uint8_t i2c_scl, const uint8_t i2c_sda, uint8_t device_address);
#endif		
		~X9258();
		

		/**
		 * \brief  Setup the pin modes correctly
		 */
		void setup();

		/**
		 * \brief  Set the device addres. The lower 4 bits of device_address together with X9258_DEVICE_TYPE is the device address
		 */
		void setAddress(const uint8_t device_address); 
		

		/**
		 * \brief Get the setting/value of given wiper
		 */
		uint8_t getWiper(const uint8_t wiper);

		/**
		 * \brief Set the setting/value of given wiper to value
		 */
		void setWiper(const uint8_t wiper, const uint8_t value);
		
		/*
		 * Each potmeter has 4 non-volatile data registers.
		 * All operations changing data can take a maximum of 10 ms
		 */

		/**
		 * \brief Get memory (0-3) for the given wiper
		 */
		uint8_t getMemory(const uint8_t wiper, const uint8_t memory);

		/**
		 * \brief Set memory (0-3) for the given wiper
		 */
		void setMemory(const uint8_t wiper, const uint8_t memory, const uint8_t value);
		

		/**
		 * \brief Move memory value to wiper
		 */
		void xfrMemoryToWiper(const uint8_t wiper, const uint8_t memory);

		/**
		 * \brief Move wiper value to memory
		 */
		void xfrWiperToMemory(const uint8_t wiper, const uint8_t memory);
		

		/**
		 * \brief Move memory value for all wipers to their respective wipers
		 */
		void globalXfrMemoryToWiper(const uint8_t memory);

		/**
		 * \brief Move wiper value fro all wipers to their respective memory
		 */
		void globalXfrWiperToMemory(const uint8_t memory);
		

		/**
		 * \brief Increment/decrement the wiper value (untested)
		 */
		void enableIncDecWiper(const uint8_t wiper, const uint8_t inc_when_high);
};

#endif

