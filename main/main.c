#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "DS18B20.h"

void app_main(void)
{
	DS18B20_INIT();
	
	// Both Search ROM and Read ROM would work since 1 device is on the bus
	//SEARCH_ROM();
	READ_ROM();

	while(1){
				
		float temp = GetTemperature();
		printf("Temperature: %fC\n",temp);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
