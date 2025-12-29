#include "App.h"
#include "cmsis_os.h"
#include <stdio.h>


uint32_t buttonPressedCounter = 0;

extern osSemaphoreId_t buttonSemHandle;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

static void ToggleTemperatureUnits(void);
static void InitializeVariables(void);
static void InitalizePeripherals(void);
static float CelsiusToFahrenheit(float celsius);


System_Peripherals_Status_t systemStatus;
Sensor_Reading_t sensor;
Oled_t display;

sht3x_handle_t handle =
{
    .i2c_handle = &hi2c1,
    .device_address = SHT3X_I2C_DEVICE_ADDRESS_ADDR_PIN_LOW
};


/**
 * Button Task Thread
 */
	uint32_t testCounter = 0;
void Button_Task(void *args)
{
	uint32_t lastPressedTime = 0;	//!< Time in Ticks
	uint32_t realTimeInTicks = 0;
	uint32_t lastInterruptCounter = 0;
	uint32_t currentInterruptCounter = 0;
	osDelay(50);
	while(1)
	{
		if (osSemaphoreAcquire(buttonSemHandle, osWaitForever) == osOK)
		{
			uint32_t now = osKernelGetTickCount();

			if ((now - lastPressedTime) > BUTTON_DEBOUNCING_THRESHOLD)
			{
				lastPressedTime = now;
				ToggleTemperatureUnits();
			}

			osDelay(BUTTON_DEBOUNCING_THRESHOLD);

			// Re-enable button interrupt
			HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		}
		// Wait for the ISR to set the flag
//		currentInterruptCounter = GetButtonInterruptCounter();
//		if(currentInterruptCounter != lastInterruptCounter)
//		{
//			realTimeInTicks = osKernelGetTickCount();
////
////			// ignore deboucing
//			if((realTimeInTicks - lastPressedTime) > BUTTON_DEBOUNCING_THRESHOLD)
//			{
//				lastPressedTime = realTimeInTicks;
//				ToggleTemperatureUnits();
//			}
//			lastInterruptCounter = currentInterruptCounter;
//		}
		osThreadYield();
	}
}



/**
 * Sensor Monitoring thread
 */
void SensorMonitoring_Task(void *args)
{
	InitializeVariables();
	InitalizePeripherals();
	while(1)
	{
		// Check if there is any system error
		if(systemStatus.isSHTInError)
		{
			SSD1306_Clear();
			SSD1306_GotoXY (10,10); // goto 10, 10
			SSD1306_Puts ("Sensor ERROR", &Font_11x18, 1); // print Hello
			SSD1306_UpdateScreen(); // update screen
			osDelay(1000);
			// trigger hardfault
			HardFault_Handler();
		}
		sht3x_read_temperature_and_humidity(&handle, &sensor.temperature, &sensor.humidity);
		// Update the screen
//		SSD1306_Clear();

		SSD1306_DrawRectangle(0, 10, 128, 20, 0);  // clear T line
		SSD1306_DrawRectangle(0, 35, 128, 20, 0);  // clear H line
		if(sensor.temperatureUnit == DEGREE_CELSIUS)
		{
			snprintf(display.firstLine, OLED_EACH_LINE_MAX_COUNT, "T: %.1f C", sensor.temperature);
		}
		else
		{
			snprintf(display.firstLine, OLED_EACH_LINE_MAX_COUNT, "T: %.1f F", CelsiusToFahrenheit(sensor.temperature));
		}
		snprintf(display.secondLine, OLED_EACH_LINE_MAX_COUNT, "H: %.1f %%", sensor.humidity);

		// Print Temperature
		SSD1306_GotoXY(5, 10);
		SSD1306_Puts(display.firstLine, &Font_11x18, 1);

		// Print Humidity
        SSD1306_GotoXY(5, 35);
        SSD1306_Puts(display.secondLine, &Font_11x18, 1);

        SSD1306_UpdateScreen();


		osDelay(1000);	// Read the readings every 1 seconds
	}
}

static void InitalizePeripherals(void)
{
	if (!sht3x_init(&handle))
	{
		systemStatus.isSHTInError = true;
	}
	if(SSD1306_Init () != 1) // initialise the display
	{
//		systemStatus.isOledInError = true;
	}
	SSD1306_Clear();
	SSD1306_UpdateScreen();
	sht3x_read_temperature_and_humidity(&handle, &sensor.temperature, &sensor.humidity);
	sht3x_set_header_enable(&handle, true);
	osDelay(2000);
	sht3x_set_header_enable(&handle, false);
}

static void InitializeVariables(void)
{
	// Set everything to 0
	memset(&sensor, 0, sizeof(Sensor_Reading_t));
	memset(&display, 0, sizeof(Oled_t));
}

static void ToggleTemperatureUnits(void)
{
	buttonPressedCounter++;
	sensor.temperatureUnit = (sensor.temperatureUnit == DEGREE_CELSIUS) ? DEGREE_FAHRENHEIT : DEGREE_CELSIUS;
}


static float CelsiusToFahrenheit(float celsius)
{
    return (celsius * 9.0f / 5.0f) + 32.0f;
}
