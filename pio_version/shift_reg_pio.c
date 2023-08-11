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
#include "pico/stdlib.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"

#include "shift_reg.pio.h"

const uint32_t CLK_GP   = 13;
const uint32_t SH_LD_GP = 14;
const uint32_t Q_GP     = 10;

const uint32_t TEST_GP  = 15;

uint dma_chan = 0;
volatile uint32_t value_from_pio = 0;


const uint LED_PIN = PICO_DEFAULT_LED_PIN;

void main( void )
{
  gpio_init( TEST_GP );  gpio_set_dir( TEST_GP, GPIO_OUT );
  gpio_put( TEST_GP, 0 );

  gpio_init( LED_PIN );  gpio_set_dir( LED_PIN, GPIO_OUT );
  gpio_put( LED_PIN, 1 );

  stdio_init_all();

  int  shift_reg_sm   = pio_claim_unused_sm(pio0, true);
  uint offset         = pio_add_program(pio0, &shift_reg_program);
  shift_reg_program_init(pio0, shift_reg_sm, offset, CLK_GP, Q_GP);

  dma_channel_config c = dma_channel_get_default_config(dma_chan);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, false);
  channel_config_set_dreq(&c, pio_get_dreq(pio0, shift_reg_sm, false));
  channel_config_set_transfer_data_size( &c, DMA_SIZE_8 );

  dma_channel_configure(dma_chan, &c,
			&value_from_pio,                    // Destination pointer (local var)
			&pio0->rxf[shift_reg_sm],           // Source pointer (receive from PIO FIFO)
			1,                                  // Number of transfers
			true                                // Start immediately
                        );

  pio_sm_set_enabled(pio0, shift_reg_sm, true);

  while( true ) 
  {
    gpio_put( TEST_GP, 1 );
    pio_sm_put_blocking(pio0, shift_reg_sm, 1);
    gpio_put( TEST_GP, 0 );

    gpio_put( TEST_GP, 1 );
    dma_channel_wait_for_finish_blocking( dma_chan );
    gpio_put( TEST_GP, 0 );

    printf("0x%08X\n\n", value_from_pio);

    sleep_ms(1000);

    dma_channel_start( dma_chan );
  }
}
