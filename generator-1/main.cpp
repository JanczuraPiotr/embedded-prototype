#include "FreeRTOS.h"
#include "task.h"
#include "stm32f767xx.h"

extern "C" void vAssertCalled(const char *const pcFileName, unsigned long ulLine)
{
    (void) pcFileName;
    (void) ulLine;

    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

void SystemClock_Config_216MHz(void) {
    RCC->CR |= RCC_CR_HSION;
    while((RCC->CR & RCC_CR_HSIRDY) == 0);

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

    // Cel: 216 MHz. Wzór: SYSCLK = (HSI / M) * N / P
    // M = 16 -> (16MHz / 16) = 1 MHz (zalecana częstotliwość wejściowa PLL)
    // N = 432 -> 1 MHz * 432 = 432 MHz
    // P = 2 -> 432 MHz / 2 = 216 MHz
    // Poprawna konfiguracja PLL dla 216 MHz z wewnętrznego zegara HSI (16MHz):
    // M=8 (16/8 = 2MHz wejście PLL), N=216 (2*216 = 432MHz), P=2 (432/2 = 216MHz)
    RCC->PLLCFGR = (8 << RCC_PLLCFGR_PLLM_Pos)   |
                   (216 << RCC_PLLCFGR_PLLN_Pos) |
                   (0 << RCC_PLLCFGR_PLLP_Pos)   |
                   (RCC_PLLCFGR_PLLSRC_HSI);


    RCC->CR |= RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLLRDY) == 0);

    PWR->CR1 |= PWR_CR1_ODEN;
    while((PWR->CSR1 & PWR_CSR1_ODRDY) == 0);

    PWR->CR1 |= PWR_CR1_ODSWEN;
    while((PWR->CSR1 & PWR_CSR1_ODSWRDY) == 0);

    FLASH->ACR = FLASH_ACR_LATENCY_7WS | FLASH_ACR_PRFTEN | FLASH_ACR_ARTEN;

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    SystemCoreClockUpdate();
}

void Hardware_Init()
{
    NVIC_SetPriorityGrouping(4);
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    __asm("DSB");

    GPIOC->MODER &= ~(3U << (8 * 2));
    GPIOC->MODER |= (1U << (8 * 2));
    GPIOC->MODER &= ~(3U << (13 * 2));
}

void vGpioControlTask(void *pvParameters)
{
    (void) pvParameters;

    bool isHigh = false;
    const TickType_t period = pdMS_TO_TICKS(1);
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (isHigh) {
            GPIOC->BSRR = (1U << (8 + 16));
        } else {
            GPIOC->BSRR = (1U << 8);
        }

        isHigh = !isHigh;

        vTaskDelayUntil(&lastWakeTime, period);
    }
}


int main()
{
    SystemClock_Config_216MHz();
    Hardware_Init();

    xTaskCreate(
        vGpioControlTask,
        "synch",
        256,
        nullptr,
        tskIDLE_PRIORITY + 2,
        nullptr
    );

    vTaskStartScheduler();

    while (true);
}
