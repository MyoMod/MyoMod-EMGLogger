#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/multicore.h"

#include "comInterface.h"
#include "max11254.h"
#include "debug_pins.h"

//include memcpy
#include <string.h>
#include "math.h"

// Defines

#define ADC_CLK_PIN 10
#define ADC_MOSI_PIN 11
#define ADC_MISO_PIN 8
#define ADC_CS_PIN 9
#define ADC_RDYB_PIN 6
#define ADC_RESET_PIN 7

#define POWER_SAVING_PIN 23

#define USE_ONBOARD_ADC 0

#define LED_R_PIN 22
#define LED_G_PIN 21
#define LED_B_PIN 20

#define JUMPER1_PIN 3
#define JUMPER2_PIN 2
#define JUMPER3_PIN 5
#define JUMPER4_PIN 4
#define JUMPER5_PIN 28
#define JUMPER6_PIN 26
#define JUMPER7_PIN 27

//variables
static uint16_t sampleRate = 12'000; //Hz

static MAX11254 *g_adc;


// private function prototypes
void setup();
void sendSampleBuffer(void* buffer, uint32_t length);

int main(){
    setup();

    while(1)
    {
        //MAX11254_STAT status = g_adc->getStatus();

        // check if a conversion is finished
        if(g_adc->dataAvailable())
        {
            g_adc->IRQ_handler();
        }
    }

    return 0;
}

void setup()
{
    stdio_init_all();

    // Initialise UART 0
    uart_init(uart0, 921600);

    // Initialise DEBUG_PINs as outputs
    gpio_init(DEBUG_PIN1);
    gpio_init(DEBUG_PIN2);
    gpio_init(DEBUG_PIN3);
    gpio_set_dir(DEBUG_PIN1, GPIO_OUT);
    gpio_set_dir(DEBUG_PIN2, GPIO_OUT);
    gpio_set_dir(DEBUG_PIN3, GPIO_OUT);

    // Setup up ADC 0
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    // Init MAX11254
    // gpio_init(POWER_SAVING_PIN);
    // gpio_set_dir(POWER_SAVING_PIN, GPIO_OUT);
    // gpio_put(POWER_SAVING_PIN, 1); // turn off power saving

    spi_inst_t *spiADC = spi1;
    gpio_set_function(ADC_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(ADC_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(ADC_MISO_PIN, GPIO_FUNC_SPI);

    volatile uint32_t actualBaudRate = 0;
    actualBaudRate = spi_init(spiADC, 8000000);
    printf("Actual baud rate: %d\n", actualBaudRate);

    g_adc = new MAX11254(spiADC, ADC_CS_PIN, ADC_RDYB_PIN, ADC_RESET_PIN, comInterfaceAddSample);
    g_adc->startConversion(true);

    g_adc->setSampleRate(sampleRate);
    
    //init comInterface
    comInterfaceInit(g_adc);

    g_adc->setChannels(0x01);
}
    
