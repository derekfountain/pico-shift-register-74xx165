/*

MIT License

Copyright (c) 2023 Derek Fountain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"

/*
 * I'm working off 2 datasheets for the 74xx165 device. The Texas Instruments
 * one is better written and easier to understand, but it doesn't have the
 * timing diagrams of the Nexperia one. They use different naming conventions.
 *
 * Nexperia                              TI
 * ========                              ==
 * PL                                    SH/LD
 * CP                                    CLK
 * DS                                    SER
 * CE                                    CLK_INH
 *
 * I chose the names from the TI datasheet/
 */

/* Input from the shift register */
const uint32_t Q_GP     = 10;

/* Outputs to the shift register */
const uint32_t CLK_GP   = 13;
const uint32_t SH_LD_GP = 14;

/*
 * Nexperia calls pin 15 on the 165 /CE. TI calls it CLK_INH (inhibit).
 * Just tie it to GND.
 */

void main( void )
{
  stdio_init_all();

  gpio_init( CLK_GP );   gpio_set_dir( CLK_GP, GPIO_OUT );   gpio_put( CLK_GP, 0 );
  gpio_init( SH_LD_GP ); gpio_set_dir( SH_LD_GP, GPIO_OUT ); gpio_put( SH_LD_GP, 1 );
  gpio_init( Q_GP );     gpio_set_dir( Q_GP, GPIO_IN );

  /*
   * Timings come from the Nexperia datasheet, which is more thorough.
   *
   * Data has to be on the inputs for "setup" time, tSU. That's 8.5ns at 3V3,
   * according to the Nexperia data sheet. Then the LD can be activated
   * to latch the data.
   */
  sleep_ms(100);
  
  while( true ) 
  {
    /*
     * Latch the data. It happens on the rising edge. The delay is the "pulse
     * width" which is tW in Fig8. This sleep represents 9.0ns.
     */
    gpio_put( SH_LD_GP, 0 );
    sleep_ms(1);
    gpio_put( SH_LD_GP, 1 );

    /*
     * Wait for the D7 value to propagate to Q7 as shown in Nexperia Fig8.
     * It's marked in that figure as tPHL, which is tpd (time propagation
     * delay). At 3V3 that's typically 10.0ns, max 22ns.
     */
    sleep_ms(10);

    /*
     * The output is loaded with the first bit as the data is latched.
     * The Nexperia datasheet implies this in table 3 and shows it in
     * the timing diagram Fig 6. i.e. it shows that Q7 goes to match
     * D7 on PL going low.
     * In the TI datasheet the timing diagram implies that's what
     * happens, but it's not stated anywhere that I can see. This
     * puzzled me for a while.
     *
     * Anyway, the loop reads the MSB out, then does 7 shifts. The
     * final bit doesn't need a following shift so that's read
     * outside the loop.
     */
    printf("76543210\n");
    for( uint8_t i=0; i < 7; i++ )
    {
      uint8_t q = gpio_get( Q_GP );
      printf("%d", q);

      /*
       * Clock the shifter. The delay is the "pulse width" which is tW
       * in Fig7. This sleep represents typical 9.0ns (max 21.5ns )and
       * the data is available after that period.
       */
      gpio_put( CLK_GP, 1 );
      sleep_ms(1);

      /*
       * The data is ready to read at this point, but I need to complete
       * the clock cycle. Another tW.
       */
      gpio_put( CLK_GP, 0 );
      sleep_ms(1);
    }

    uint8_t q = gpio_get( Q_GP );    
    printf("%d\n\n", q);

    sleep_ms(1000);
  }
}
