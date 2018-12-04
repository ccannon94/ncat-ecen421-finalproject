/******************************************************************************************
 * This code works on an MSP432 with an Analog Discovery device and LaunchPad Educational
 * BoosterPack to identify if the electrical component hooked to the system is a
 * capacitor, diode, resistor, or inductor. It reads in outputs from the
 * Analog Discovery using digital I/O pins P3.0, P3.2, P3.3, and P3.6 and uses this to
 * identify the component. It Displays the results on the LCD screen of the LaunchPad
 * Educational BoosterPack.
 *
 * Coded by Christopher Cannon and Abbigail Waddell on 12/03/2018
 ******************************************************************************************/
#include "msp.h"
#include <driverlib.h>
#include <grlib.h>
#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>

/* Graphic library context */
Graphics_Context g_sContext;

int device = 0;
int main(void)
{
    // Hold the watchdog

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;


   /* Halting WDT and disabling master interrupts */
   MAP_WDT_A_holdTimer();
   MAP_Interrupt_disableMaster();

   /* Set the core voltage level to VCORE1 */
   MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

   /* Set 2 flash wait states for Flash bank 0 and 1*/
   MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
   MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

   /* Initializes Clock System */
   MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
   MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
   MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
   MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
   MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

   /* Initializes display */
   Crystalfontz128x128_Init();

   /* Set default screen orientation */
   Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

   /* Initializes graphics context */
   Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
   Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
   Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
   GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
   Graphics_clearDisplay(&g_sContext);
   Graphics_drawStringCentered(&g_sContext,
                                   "Component:",
                                   AUTO_STRING_LENGTH,
                                   64,
                                   30,
                                   OPAQUE_TEXT);

   /* Configures Pin 6.0 and 4.4 as ADC input */
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);

  /*Configures P3.0, P3.2, P3.3, P3.6 as input pins with pull up resistors */
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN0);
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN2);
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN3);
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN6);

   /* Initializing ADC (ADCOSC/64/8) */
   MAP_ADC14_enableModule();
   MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

   /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM1 (A15, A9)  with repeat)
        * with internal 2.5v reference */
   MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
   MAP_ADC14_configureConversionMemory(ADC_MEM0,
           ADC_VREFPOS_AVCC_VREFNEG_VSS,
           ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);


   /* Enabling the interrupt when a conversion on channel 1 (end of sequence)
    *  is complete and enabling conversions */
   MAP_ADC14_enableInterrupt(ADC_INT1);

   /* Enabling Interrupts */
   MAP_Interrupt_enableInterrupt(INT_ADC14);
   MAP_Interrupt_enableMaster();

   /* Setting up the sample timer to automatically step through the sequence
    * convert.*/
   MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

   /* Triggering the start of the sample */
   MAP_ADC14_enableConversion();
   MAP_ADC14_toggleConversionTrigger();


    while(1){
    	MAP_PCM_gotoLPM0();
    	ADC14_IRQHandler();
    }

}



void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    /* ADC_MEM1 conversion completed */
    if(status & ADC_INT1)
    {

        //Pin3.3 is high, Capacitor
        if(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN3) == GPIO_INPUT_PIN_HIGH){
             device = 1;
         }
         else if(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN6) == GPIO_INPUT_PIN_HIGH){
             //Pin 3.6 is high, Diode
             device =2;
         }
         else if(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN0) == GPIO_INPUT_PIN_HIGH){
             //Pin 3.0 is high, Resister
             device = 3;
         }
         else if(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN2) == GPIO_INPUT_PIN_HIGH){
             //Pin 3.2 is high, Inductor
             device = 4;
         }
         else{
             device = 0;
         }

        //Displays Capacitor on the LCD screen
        if(device == 1){
        char string[8];
        sprintf(string,"Capacitor");
        Graphics_drawStringCentered(&g_sContext,
                                    (int8_t *)string,
                                    AUTO_STRING_LENGTH,
                                    64,
                                    50,
                                    OPAQUE_TEXT);
        }
        //Displays Diode on the LCD screen
        else if(device == 2){
         char string[8];
         sprintf(string,"   Diode   ");
         Graphics_drawStringCentered(&g_sContext,
                                     (int8_t *)string,
                                     AUTO_STRING_LENGTH,
                                     64,
                                     50,
                                     OPAQUE_TEXT);
        }
        //Displays Resistor on the LCD screen
         else if(device == 3){
          char string[8];
          sprintf(string," Resistor ");
          Graphics_drawStringCentered(&g_sContext,
                                      (int8_t *)string,
                                      AUTO_STRING_LENGTH,
                                      64,
                                      50,
                                      OPAQUE_TEXT);
         }
        //Displays Inductor on the LCD screen
          else if(device == 4){
           char string[8];
           sprintf(string," Inductor ");
           Graphics_drawStringCentered(&g_sContext,
                                       (int8_t *)string,
                                       AUTO_STRING_LENGTH,
                                       64,
                                       50,
                                       OPAQUE_TEXT);
          }
        //Displays the word Nothing on the LCD screen
           else if(device == 0){
            char string[8];
            sprintf(string,"Nothing");
            Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)string,
                                        AUTO_STRING_LENGTH,
                                        64,
                                        50,
                                        OPAQUE_TEXT);
           }
        }
    }

