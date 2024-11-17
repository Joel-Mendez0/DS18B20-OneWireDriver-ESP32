/*
 * DS18B20.c
 *
 *  Created on: Nov 9, 2024
 *      Author: Joel
 */

#include "DS18B20.h"

gpio_config_t DQconfig;

uint8_t SCRATCH_PAD[9][8];

uint8_t FOUND_ROMS[MAX_DEVICES][8];
uint8_t LAST_DISCREPANCY = 0;
uint8_t DEVICE_COUNT = 0;

void DS18B20_INIT(void){
	
	// Configure DQ to INPUT_OUTPUT_OD (external 5K Resistor to VDD (5V))
	
	DQconfig = (gpio_config_t){
		.pin_bit_mask = (1ULL << DQ_PIN),
		.mode = GPIO_MODE_INPUT_OUTPUT_OD,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE 
	};
	
	gpio_set_level(DQ_PIN,HIGH);
	gpio_config(&DQconfig);
	
}

uint8_t DS18B20_RESET_PRESENCE_PULSE(void){
	// Master transmits the reset pulse by pulling the 1-Wire bus low for a minimum of 480us
	gpio_set_level(DQ_PIN,LOW);
	usleep(480);
	gpio_set_level(DQ_PIN,HIGH);
	// When the DS18B20 Detects the Rising Edge it waits 15us to 60us and then transmits a presence pulse by pulling LOW for 60us to 240us
	usleep(60);
	uint8_t PRESENCE_DETECTED = !gpio_get_level(DQ_PIN);
	usleep(405); // guarantee 480us have passed 480 - (15 + 60) 
	return PRESENCE_DETECTED;
}

void DS18B20_WRITE_LOGIC_LOW(void){
	
	// Pull bus low to start the write slot, each slot should be a minimum of 60us in duration with a 1us recovery time between slots
	gpio_set_level(DQ_PIN,LOW);
	
	usleep(60);
	
	gpio_set_level(DQ_PIN,HIGH);
	
	usleep(1);
	
}

void DS18B20_WRITE_LOGIC_HIGH(void){
	
	// Pull bus low to start the write slot, each slot should be a minimum of 60us in duration with a 1us recovery time between slots
	gpio_set_level(DQ_PIN,LOW);
	
	usleep(10);
	
	gpio_set_level(DQ_PIN,HIGH);
	
	usleep(50);
	
	usleep(1);
}

void DS18B20_WRITE_BYTE(uint8_t byte){
	
	for(uint8_t i = 0; i < 8; i++){
		
		if(((byte >> i) & 1) == 1){
			DS18B20_WRITE_LOGIC_HIGH();
		}
		else{
			DS18B20_WRITE_LOGIC_LOW();
		}
	}
	
}

uint8_t DS18B20_READ_TIME_SLOTS(void){
	// minimum of 60µs in duration with a minimum of a 1µs recovery time between slots
	// Read time slot is initiated by master pulling the bus low for a minimum of 1us and then releasing the bus.
	gpio_set_level(DQ_PIN,LOW);
	usleep(1);
	gpio_set_level(DQ_PIN,HIGH);
	usleep(5);
	
	if(gpio_get_level(DQ_PIN)){
		usleep(55);
		return 1;
	}
	else{
		usleep(55);
		return 0;
	}
}

uint8_t SEARCH_ROM(void) {
	
	// Algorithm Flags
	uint8_t ID_BIT, CMP_ID_BIT;
	uint8_t ROM_BYTE_MASK = 1;
	uint8_t SEARCH_DIRECTION;
	uint8_t NEW_DISCREPANCY = 0;
	uint8_t ROM[8];
	
	// Reset Device count and clear array for new search
	DEVICE_COUNT = 0;
	memset(FOUND_ROMS,0,sizeof(FOUND_ROMS));
	
	if(!DS18B20_RESET_PRESENCE_PULSE()){
		return 0;
	}	
	// Send the search rom command using the one wire search algorithm
	DS18B20_WRITE_BYTE(SEARCH_ROM_CMD);
	
	do{
		for(uint8_t ROM_BYTE_NUMBER = 0; ROM_BYTE_NUMBER < 8; ROM_BYTE_NUMBER++){
			for(uint8_t BIT_INDEX = 0; BIT_INDEX < 8; BIT_INDEX++){
				// Read a bit and its complement from all the devices simultaneously
				ID_BIT = DS18B20_READ_TIME_SLOTS();
				CMP_ID_BIT = DS18B20_READ_TIME_SLOTS();
				
				if(ID_BIT == 1 && CMP_ID_BIT == 1){
					return DEVICE_COUNT; // 0, No Devices were found
				}
				else if(ID_BIT != CMP_ID_BIT){
					SEARCH_DIRECTION = ID_BIT; // Take the detected path
				}
				else{
					// A discrepancy was found
					if(ROM_BYTE_MASK < LAST_DISCREPANCY){
						SEARCH_DIRECTION = ((ROM[ROM_BYTE_NUMBER] & ROM_BYTE_MASK) > 0); 
					}
					else{
						SEARCH_DIRECTION = (ROM_BYTE_MASK == LAST_DISCREPANCY);
					}
					if(SEARCH_DIRECTION == 0){
						NEW_DISCREPANCY = ROM_BYTE_MASK;
					}
				}
				// Set or clear the bit in the ROM
				if(SEARCH_DIRECTION == 1){
					ROM[ROM_BYTE_NUMBER] |= ROM_BYTE_MASK;
				}
				else{
					ROM[ROM_BYTE_NUMBER] &= ~ROM_BYTE_MASK;
				}
				// Write the bit value of search direction to the bus
				if(SEARCH_DIRECTION == 1){
					DS18B20_WRITE_LOGIC_HIGH();
				}
				else{
					DS18B20_WRITE_LOGIC_LOW();
				}
				// Increment the mask to the next position
				ROM_BYTE_MASK <<= 1;
				
				if(ROM_BYTE_MASK == 0){
					ROM_BYTE_MASK = 1;
				}
				
			}
			
		}
		
		if(DEVICE_COUNT < MAX_DEVICES){
			memcpy(FOUND_ROMS[DEVICE_COUNT],ROM,8);
			DEVICE_COUNT++;
		}
		
		LAST_DISCREPANCY = NEW_DISCREPANCY;		
		
	}while(LAST_DISCREPANCY != 0);
	
	
	return DEVICE_COUNT;
}

void READ_ROM(void) {
    uint8_t PRESENCE_DETECTED = DS18B20_RESET_PRESENCE_PULSE();
    
    if (PRESENCE_DETECTED == 1) {
        DS18B20_WRITE_BYTE(READ_ROM_CMD);
        
        for (uint8_t ROM_NUMBER = 0; ROM_NUMBER < 8; ROM_NUMBER++) {
            uint8_t ROM_BYTE = 0;
            for (uint8_t i = 0; i < 8; i++) {
				
                ROM_BYTE >>= 1;

                if (DS18B20_READ_TIME_SLOTS() == 1) {
                    ROM_BYTE |= 0x80;
                }
            }
            FOUND_ROMS[0][ROM_NUMBER] = ROM_BYTE;
        }
    }
    
    return;
}

void MATCH_ROM(uint8_t* ROM_CODE){
	uint8_t PRESENCE_DETECTED = DS18B20_RESET_PRESENCE_PULSE();
	
	if(PRESENCE_DETECTED == 1){
		DS18B20_WRITE_BYTE(MATCH_ROM_CMD);
		
		for(uint8_t ROM_NUMBER = 0; ROM_NUMBER < 8; ROM_NUMBER++){
			DS18B20_WRITE_BYTE(ROM_CODE[ROM_NUMBER]);
		}
	}
}

void SKIP_ROM(void){
	uint8_t PRESENCE_DETECTED = DS18B20_RESET_PRESENCE_PULSE();
	
	if(PRESENCE_DETECTED == 1){
		DS18B20_WRITE_BYTE(SKIP_ROM_CMD);
	}
}
uint8_t ALARM_SEARCH(void) {
	
	// Algorithm Flags
	uint8_t ID_BIT, CMP_ID_BIT;
	uint8_t ROM_BYTE_MASK = 1;
	uint8_t SEARCH_DIRECTION;
	uint8_t NEW_DISCREPANCY = 0;
	uint8_t ROM[8];
	
	// Reset Device count and clear array for new search
	DEVICE_COUNT = 0;
	memset(FOUND_ROMS,0,sizeof(FOUND_ROMS));
	
	if(!DS18B20_RESET_PRESENCE_PULSE()){
		return 0;
	}	
	// Send the alarm search command using the one wire search algorithm
	DS18B20_WRITE_BYTE(ALARM_SEARCH_CMD);
	
	do{
		for(uint8_t ROM_BYTE_NUMBER = 0; ROM_BYTE_NUMBER < 8; ROM_BYTE_NUMBER++){
			for(uint8_t BIT_INDEX = 0; BIT_INDEX < 8; BIT_INDEX++){
				// Read a bit and its complement from all the devices simultaneously
				ID_BIT = DS18B20_READ_TIME_SLOTS();
				CMP_ID_BIT = DS18B20_READ_TIME_SLOTS();
				
				if(ID_BIT == 1 && CMP_ID_BIT == 1){
					return DEVICE_COUNT; // 0, No Devices were found
				}
				else if(ID_BIT != CMP_ID_BIT){
					SEARCH_DIRECTION = ID_BIT; // Take the detected path
				}
				else{
					// A discrepancy was found
					if(ROM_BYTE_MASK < LAST_DISCREPANCY){
						SEARCH_DIRECTION = ((ROM[ROM_BYTE_NUMBER] & ROM_BYTE_MASK) > 0); 
					}
					else{
						SEARCH_DIRECTION = (ROM_BYTE_MASK == LAST_DISCREPANCY);
					}
					if(SEARCH_DIRECTION == 0){
						NEW_DISCREPANCY = ROM_BYTE_MASK;
					}
				}
				// Set or clear the bit in the ROM
				if(SEARCH_DIRECTION == 1){
					ROM[ROM_BYTE_NUMBER] |= ROM_BYTE_MASK;
				}
				else{
					ROM[ROM_BYTE_NUMBER] &= ~ROM_BYTE_MASK;
				}
				// Write the bit value of search direction to the bus
				if(SEARCH_DIRECTION == 1){
					DS18B20_WRITE_LOGIC_HIGH();
				}
				else{
					DS18B20_WRITE_LOGIC_LOW();
				}
				// Increment the mask to the next position
				ROM_BYTE_MASK <<= 1;
				
				if(ROM_BYTE_MASK == 0){
					ROM_BYTE_MASK = 1;
				}
				
			}
			
		}
		
		if(DEVICE_COUNT < MAX_DEVICES){
			memcpy(FOUND_ROMS[DEVICE_COUNT],ROM,8);
			DEVICE_COUNT++;
		}
		
		LAST_DISCREPANCY = NEW_DISCREPANCY;		
		
	}while(LAST_DISCREPANCY != 0);
	
	
	return DEVICE_COUNT;
}

void READ_SCRATCHPAD(void){
	
	DS18B20_WRITE_BYTE(READ_SCRATCHPAD_CMD);
	
	for(uint8_t BYTE = 0; BYTE < 9; BYTE++){
		for(uint8_t BIT = 0; BIT < 8; BIT++){
			SCRATCH_PAD[BYTE][BIT] = DS18B20_READ_TIME_SLOTS();
		}
	}
}

void CONVERT_T(void){
	DS18B20_WRITE_BYTE(CONVERT_T_CMD);
}

float GetTemperature(void){
    float temp = 0.0f;

    //DS18B20_RESET_PRESENCE_PULSE();
    MATCH_ROM(FOUND_ROMS[0]);

    READ_SCRATCHPAD();
    
	for (uint8_t k = 0; k < 16; k++) {
		if(k == 11){
			break;
		}
    	temp += SCRATCH_PAD[k / 8][k % 8] * pow(2,k-4);
	}


    return temp;
}

// Function to compute CRC for the data
uint8_t DS18B20_CalculateCRC(uint8_t *data, uint8_t length) {
	// TODO
	uint8_t crc = 0;
	return crc;
}

// Function to verify CRC on the Scratchpad
uint8_t DS18B20_VERIFY_CRC(void) {
    // TODO
    return 1;
}

