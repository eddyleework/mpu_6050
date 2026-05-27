#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "drv_mpu6050.h"
#include "drv_mpu6050_filter.h"
#include <stdio.h>
#include <math.h>

/* --- Private Variables --- */
int16_t acc[3], gyro[3];

/* --- Function Prototypes --- */
void SystemClock_Config(void);

/* --- UART Redirect for Printf --- */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 10);
    return ch;
}

int main(void) {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    MPU6050_Init(&hi2c1);

    IMU_Angle_t my_angle = { 0 };

    while (1) {
        if (MPU6050_Read_Raw(&hi2c1, acc, gyro) == 0) {

            Apply_Complementary_Filter(acc, gyro, &my_angle, 0.02f);

            int accel_p_x10 = (int)(my_angle.pitch * 10);
            int accel_r_x10 = (int)(my_angle.roll * 10);
            int filter_p_x10 = (int)(my_angle.f_pitch * 10);
            int filter_r_x10 = (int)(my_angle.f_roll * 10);

            printf("Accel   | P: %4d | R: %4d\r\n", accel_p_x10, accel_r_x10);
            printf("Filtered| P: %4d | R: %4d\r\n", filter_p_x10, filter_r_x10);
            printf("-----------------------------\r\n");

        } else {
            printf("Sensor Error!\r\n");
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }

        HAL_Delay(20);
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    if (HAL_PWREx_EnableOverDrive() != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) Error_Handler();
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
