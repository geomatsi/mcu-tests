#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt::entry;
use cortex_m_semihosting::hprintln;
use hal::prelude::*;
use panic_semihosting as _;
use stm32f4xx_hal as hal;
use stm32f4xx_hal::rcc::Config;

#[entry]
fn main() -> ! {
    let dp = hal::pac::Peripherals::take().unwrap();
    let cp = cm::Peripherals::take().unwrap();
    let mut rcc = dp
        .RCC
        .freeze(Config::hse(25.MHz()).sysclk(84.MHz()).pclk1(42.MHz()));

    let gpioc = dp.GPIOC.split(&mut rcc);
    let mut led = gpioc.pc13.into_push_pull_output();
    let mut delay = cp.SYST.delay(&rcc.clocks);

    loop {
        let _ = hprintln!("Hello World!");

        led.set_high();
        delay.delay_ms(2000u32);
        led.set_low();
        delay.delay_ms(500u32);
    }
}
