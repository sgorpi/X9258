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
#include "X9258.h"

/*
 * Device has a two-wire bus interface (TWI), but the arduino 'Wire' library doesn't seem to support its mode of operation.
 * Thus, we implement it in software.
 * The datasheet mentions the following read sequence, which the arduino 'Wire' lib can't do:
 * write(address); write(instruction); read(value)
 */

X9258::X9258(const uint8_t i2c_scl, const uint8_t i2c_sda) : X9258_SCL_PIN_NUM(i2c_scl), X9258_SDA_PIN_NUM(i2c_sda)
{
	setAddress(0);
}
X9258::X9258(const uint8_t i2c_scl, const uint8_t i2c_sda, uint8_t device_address) : X9258_SCL_PIN_NUM(i2c_scl), X9258_SDA_PIN_NUM(i2c_sda)
{
	setAddress(device_address);
}

X9258::~X9258() {}


void X9258::setup() {
	pinMode(X9258_SCL_PIN_NUM, OUTPUT);
	pinMode(X9258_SDA_PIN_NUM, OUTPUT);

	digitalWriteFast(X9258_SCL_PIN_NUM, LOW);
	digitalWriteFast(X9258_SDA_PIN_NUM, HIGH);
}


uint8_t X9258::wait_for_ack() {
	uint8_t num = 0;
	
	pinMode(X9258_SDA_PIN_NUM, INPUT);
	
	digitalWriteFast(X9258_SCL_PIN_NUM, HIGH); // clk

	num = digitalReadFast(X9258_SDA_PIN_NUM); // read
	
	digitalWriteFast(X9258_SCL_PIN_NUM, LOW); // clk
	
	pinMode(X9258_SDA_PIN_NUM, OUTPUT);
	
	return (num == LOW);
}


void X9258::send_clock() {
	digitalWriteFast(X9258_SCL_PIN_NUM, HIGH);
	digitalWriteFast(X9258_SCL_PIN_NUM, LOW);
}


void X9258::send_start() {
	pinMode(X9258_SCL_PIN_NUM, OUTPUT);
	pinMode(X9258_SDA_PIN_NUM, OUTPUT);

	digitalWriteFast(X9258_SDA_PIN_NUM, HIGH);
	digitalWriteFast(X9258_SCL_PIN_NUM, HIGH);
	//600 ns
	digitalWriteFast(X9258_SDA_PIN_NUM, LOW);
}


void X9258::send_stop() {
	digitalWriteFast(X9258_SDA_PIN_NUM, LOW);
	digitalWriteFast(X9258_SCL_PIN_NUM, HIGH);
	//600 ns
	digitalWriteFast(X9258_SDA_PIN_NUM, HIGH);
	digitalWriteFast(X9258_SCL_PIN_NUM, LOW);
}


uint8_t X9258::send_data(uint8_t data) {
	digitalWriteFast(X9258_SCL_PIN_NUM, LOW);
	
	for (uint8_t mask = 0x80; mask; mask >>= 1) {
		digitalWriteFast(X9258_SDA_PIN_NUM, (data & mask) ? HIGH : LOW );
		
		send_clock();
	}
	
	return wait_for_ack();
}


void X9258::send_ack() {
	digitalWriteFast(X9258_SDA_PIN_NUM, LOW);

	send_clock();
}


uint8_t X9258::recv_data() {
	uint8_t data = 0;
	
	pinMode(X9258_SDA_PIN_NUM, INPUT);
	asm volatile("nop"); // wait a bit for the state transition

	digitalWriteFast(X9258_SCL_PIN_NUM, LOW);
	for (uint8_t i = 0; i < 8; i++) {
		data <<= 1;
		data |= (digitalReadFast(X9258_SDA_PIN_NUM)==HIGH ? 0x01 : 0x00);
		
		send_clock();
	}

	pinMode(X9258_SDA_PIN_NUM, OUTPUT);
	
	// send ack
	send_ack();
	
	return data;
}


void X9258::write_instruction(const uint8_t instruction) {
	noInterrupts(); // temporarily disable any interrupt

	// send start
	send_start();

	// send address
	if (!send_data(device_address)) {
		send_stop();
		return;
	}

	// send instruction
	if (!send_data(instruction)) {
		send_stop();
		return;
	}

	// send stop
	send_stop();
	
	interrupts(); // re-enable interrupt
}


void X9258::write_value(const uint8_t instruction, const uint8_t value) {
	noInterrupts(); // temporarily disable any interrupt

	// send start
	send_start();


	// send address
	if (!send_data(device_address)) {
		send_stop();
		return;
	}

	// send instruction
	if (!send_data(instruction)) {
		send_stop();
		return;
	}

	// send value
	if (!send_data(value)) {
		send_stop();
		return;
	}

	// send stop
	send_stop();
	
	interrupts(); // re-enable interrupt
}


uint8_t X9258::read_value(const uint8_t instruction) {
	uint8_t value = 0xff;
	
	noInterrupts(); // temporarily disable any interrupt

	// send start
	send_start();

	// send address
	if (!send_data(device_address)) {
		send_stop();
		return value;
	}

	// send instruction
	if (!send_data(instruction)) {
		send_stop();
		return value;
	}

	// read data
	value = recv_data();
	
	// send stop
	send_stop();
	
	interrupts(); // re-enable interrupt

	return value;
}


/**
 * lower 4 bits of device_address together with X9258_DEVICE_TYPE is the device address
 */
void X9258::setAddress(const uint8_t dev_address) {
	device_address = X9258_DEVICE_TYPE | (dev_address & 0x0F);
}


uint8_t X9258::getWiper(const uint8_t wiper) {
	return read_value( X9258_READ_WIPER | (wiper & 0x03) );
}


void X9258::setWiper(const uint8_t wiper, const uint8_t value) {
	write_value(( X9258_WRITE_WIPER | (wiper & 0x03) ), value);
}


uint8_t X9258::getMemory(const uint8_t wiper, const uint8_t memory) {
	return read_value(( X9258_READ_MEMORY | ((memory & 0x03) << 2) | (wiper & 0x03) ));
}


void X9258::setMemory(const uint8_t wiper, const uint8_t memory, const uint8_t value) {
	write_value(( X9258_WRITE_MEMORY | ((memory & 0x03) << 2) | (wiper & 0x03)  ), value);
}


void X9258::xfrMemoryToWiper(const uint8_t wiper, const uint8_t memory) {
	write_instruction( X9258_XFR_MEMORY_TO_WIPER | ((memory & 0x03) << 2) | (wiper & 0x03)  );
}


void X9258::xfrWiperToMemory(const uint8_t wiper, const uint8_t memory) {
	write_instruction( X9258_XFR_WIPER_TO_MEMORY | ((memory & 0x03) << 2) | (wiper & 0x03)  );
}


void X9258::globalXfrMemoryToWiper(const uint8_t memory) {
	write_instruction( X9258_GLOBAL_XFR_MEMORY_TO_WIPER | ((memory & 0x03) << 2) );
}


void X9258::globalXfrWiperToMemory(const uint8_t memory) {
	write_instruction( X9258_GLOBAL_XFR_WIPER_TO_MEMORY | ((memory & 0x03) << 2) );
}


void X9258::enableIncDecWiper(const uint8_t wiper, const uint8_t inc_when_high) {
	// I don't get the use of this inc/dec function much from the datasheet...
	write_value(( X9258_INC_DEC_WIPER | (wiper & 0x03) ), inc_when_high );
}



