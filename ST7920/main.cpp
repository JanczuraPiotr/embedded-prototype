#include <stdio.h>
#include <sys/types.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "../lib/ST7920.hpp"
#include "../lib/FreeRTOS.hpp"


extern "C" {

// void vAssertCalled(const char *const pcFileName, unsigned long ulLine);
// void vApplicationMallocFailedHook(void);
void hardware_init();
void SystemClock_Config_100MHz();
void GPIO_SPI1_Register_Init();
void Hardware_Init();
void vLcdTask(void *pvParameters);
void StartLcdTask(void *argument);
void vGpioControlTask(void *pvParameters);

}

pj::FreeRTOS system;
pj::ST7920 lcd(system,
    SPI1,
    []() { GPIOB->BSRR = GPIO_BSRR_BS6; },
    []() { GPIOB->BSRR = GPIO_BSRR_BR6; }
    );


int main() {

    SystemClock_Config_100MHz();
    Hardware_Init();


    xTaskCreate(vLcdTask, "lcdTask", 128 * 4,nullptr, (tskIDLE_PRIORITY + 1UL), nullptr);

    vTaskStartScheduler();

    while (true) {}
}


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

void Hardware_Init()
{
    NVIC_SetPriorityGrouping(4);
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    __asm("DSB");

    GPIOA->MODER &= ~3U;
    GPIOA->MODER |= 1U;

}


void MojeFormatowanie(char* wyjscie, uint32_t liczba) {
    wyjscie[0] = 'C';
    wyjscie[1] = 'n';
    wyjscie[2] = 't';
    wyjscie[3] = '=';
    wyjscie[4] = ' ';

    for(int i = 13; i >= 5; i--) {
        wyjscie[i] = '0' + (liczba % 10);
        liczba /= 10;
    }
    wyjscie[14] = '\0';
}

void vLcdTask(void *argument) {
    (void) argument;

    GPIO_SPI1_Register_Init();
    lcd.init();

    uint32_t counter = 0;
    char buf[17];

    for(;;) {
        lcd.clear();

        uint32_t temp = counter;

        sprintf(buf, "%08lu", (unsigned long)temp);
        lcd.sendString(0,4, buf);

        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void SystemClock_Config_100MHz(void)
{
    // NOTE AI

    /* 1. Włączenie zasilania (Power Interface Clock) aby zmienić napięcie rdzenia */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS_0 | PWR_CR_VOS_1; // Scale 1 (wymagane dla > 84 MHz)

    /* 2. Konfiguracja czasu dostępu do pamięci Flash (3 cykle oczekiwania dla 100 MHz przy 3.3V) */
    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_3WS;

    /* 3. Konfiguracja pętli PLL na podstawie wewnętrznego oscylatora HSI (16 MHz)
       Wzór: f_PLL = (f_HSI / M) * N / P
       M = 16 (16MHz / 16 = 1MHz wejście dla PLL)
       N = 400 (1MHz * 400 = 400MHz częstotliwość VCO)
       P = 4 (400MHz / 4 = 100MHz zegar systemowy) */
    RCC->PLLCFGR = (16 << RCC_PLLCFGR_PLLM_Pos) |
                   (400 << RCC_PLLCFGR_PLLN_Pos) |
                   (1 << RCC_PLLCFGR_PLLP_Pos) | // 1 oznacza dzielenie przez 4
                   RCC_PLLCFGR_PLLSRC_HSI;

    /* 4. Włączenie PLL i oczekiwanie na stabilizację */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* 5. Konfiguracja dzielników magistral (AHB = 100MHz, APB1 = 50MHz, APB2 = 100MHz)
       APB1 nie może przekraczać 50 MHz, stąd dzielnik przez 2. */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    /* 6. Przełączenie źródła zegara systemowego na PLL */
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    /* 7. Aktualizacja globalnej zmiennej CMSIS (FreeRTOS jej potrzebuje!) */
    SystemCoreClock = 100000000;
}


extern "C" void GPIO_SPI1_Register_Init(void)
{
    // NOTE AI
    /* 1. Włączenie zegarów dla portów GPIOA, GPIOB oraz peryferium SPI1 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    __asm("DSB"); // Wymuszenie zakończenia zapisu w pamięci

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* 2. Konfiguracja pinów PA5 (SCK) i PA7 (MOSI) jako czyste Funkcje Alternatywne */
    GPIOA->MODER &= ~((3U << 10) | (3U << 14)); // Wyczyszczenie pól dla PA5 i PA7
    GPIOA->MODER |= (2U << 10) | (2U << 14);     // Ustawienie trybu AF (10b) dla obu pinów

    // Maksymalna prędkość wyjść (Very High Speed) dla ostrych zboczy sygnału
    GPIOA->OSPEEDR |= (3U << 10) | (3U << 14);

    /* 3. Precyzyjne mapowanie AF5 (SPI1) w rejestrze AFR */
    GPIOA->AFR[0] &= ~((0xFU << 20) | (0xFU << 28)); // Czyste zerowanie pól maską
    GPIOA->AFR[0] |= (5U << 20) | (5U << 28);      // Wpisanie wartości 5 (AF5)

    /* 4. Konfiguracja pinu PB6 (Software CS) jako czyste Wyjście Cyfrowe */
    GPIOB->MODER &= ~(3U << 12); // Czyszczenie pola pinu PB6
    GPIOB->MODER |= (1U << 12);  // Ustawienie jako wyjście ogólnego przeznaczenia (01b)
    GPIOB->OSPEEDR |= (3U << 12); // High speed dla ostrego zbocza CS
    GPIOB->BSRR = GPIO_BSRR_BR6; // CS na starcie w stan niski

    /* 5. Konfiguracja rejestru SPI1->CR1 w trybie Klasycznego Full-Duplex Master */
    SPI1->CR1 = 0; // Pełny reset rejestru konfiguracyjnego!

    // Ustawienia:
    // MSTR - Tryb Master
    // SSM i SSI - Programowe zarządzanie wewnętrznym flitem NSS
    // Dzielnik /128 (6U << SPI_CR1_BR_Pos) daje wolny, idealny sygnał testowy ~780 kHz
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | (6U << SPI_CR1_BR_Pos);

    /* 6. Uruchomienie peryferium SPI1 */
    SPI1->CR1 |= SPI_CR1_SPE;
}

extern "C" {

extern uint8_t _end;

caddr_t _sbrk(int incr) {
    uint8_t *heap_end = nullptr;
    uint8_t *prev_heap_end;

    if (heap_end == nullptr) {
        heap_end = &_end;
    }

    prev_heap_end = heap_end;

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}
}