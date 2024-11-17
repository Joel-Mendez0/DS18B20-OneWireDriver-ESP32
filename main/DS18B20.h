/*
 * DS18B20.h
 *
 *  Created on: Nov 9, 2024
 *      Author: Joel
 */

#ifndef MAIN_DS18B20_H_
#define MAIN_DS18B20_H_

#include "driver/gpio.h"
#include "unistd.h"
#include <string.h>
#include <math.h>

#define MAX_DEVICES 10

// DQ Pin (Input/Output OD, 5K Resistor to VDD (5V))
#define DQ_PIN GPIO_NUM_27

// 1 -> Release HIGH, 0 -> Pull LOW
#define HIGH 1
#define LOW 0

// DS18B20 ROM Commands
#define SEARCH_ROM_CMD 0xF0
#define READ_ROM_CMD 0x33
#define MATCH_ROM_CMD 0x55
#define SKIP_ROM_CMD 0xCC
#define ALARM_SEARCH_CMD 0xEC

// DS18B20 Function Commands
#define CONVERT_T_CMD 0x44
#define WRITE_SCRATCHPAD_CMD 0x4E
#define READ_SCRATCHPAD_CMD 0xBE
#define COPY_SCRATCHPAD_CMD 0x48
#define RECALL_E2_CMD 0xB8
#define READ_POWER_SUPPLY_CMD 0xB4

#define CRC_POLYNOMIAL 0x8C  // CRC-8 polynomial


extern uint8_t FOUND_ROMS[MAX_DEVICES][8];
extern uint8_t SCRATCH_PAD[9][8];

// GPIO Configuration
extern gpio_config_t DQconfig;

void DS18B20_INIT(void);
uint8_t DS18B20_RESET_PRESENCE_PULSE(void);
void DS18B20_WRITE_LOGIC_LOW(void);
void DS18B20_WRITE_LOGIC_HIGH(void);
void DS18B20_WRITE_BYTE(uint8_t byte);
uint8_t DS18B20_READ_TIME_SLOTS(void);

// ROM COMMANDS
uint8_t SEARCH_ROM(void); // Search ROM Algorithm
void READ_ROM(void); // Returns ROM Address of slave on bus, only works if 1 slave is on the bus
void MATCH_ROM(uint8_t* ROM_CODE); // Takes in an array where each element is 8-bit part of the total 64 bit ROM code
void SKIP_ROM(void); // Skips ROM, all devices respond simultaneously to next function command 
uint8_t ALARM_SEARCH(void);

// Function Commands
void READ_SCRATCHPAD(void);
void CONVERT_T(void);

// Verification CRC
uint8_t DS18B20_CalculateCRC(uint8_t *data, uint8_t length);
uint8_t DS18B20_VERIFY_CRC(void);


float GetTemperature(void);


#endif /* MAIN_DS18B20_H_ */
