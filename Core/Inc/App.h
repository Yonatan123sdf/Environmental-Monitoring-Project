#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"
#include "sht3x.h"
#include "ssd1306.h"
#include "fonts.h"
#include <stdbool.h>
#define BUTTON_DEBOUNCING_THRESHOLD	600
#define OLED_EACH_LINE_MAX_COUNT	32

typedef struct System_Peripherals_Status_t
{
	bool isSystemError;
	bool isButtonTaskInError;
	bool isButtonISRInError;
	bool isSHTInError;
	bool isOledInError;
}System_Peripherals_Status_t;

typedef enum Sensor_Temperature_Unit_e
{
	DEGREE_CELSIUS = 0,
	DEGREE_FAHRENHEIT	= 1,
}Sensor_Temperature_Unit_e;



typedef struct Sensor_Reading_t
{
	float temperature;
	float humidity;
	Sensor_Temperature_Unit_e temperatureUnit;
}Sensor_Reading_t;

typedef struct Oled_t
{
	char firstLine[OLED_EACH_LINE_MAX_COUNT];
	char secondLine[OLED_EACH_LINE_MAX_COUNT];

}Oled_t;


extern System_Peripherals_Status_t systemStatus;
void Button_Task(void *args);
void SensorMonitoring_Task(void *args);
uint32_t GetButtonInterruptCounter(void);
#endif /* INC_APP_H_ */
