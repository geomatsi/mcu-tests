#![no_main]
#![no_std]

use cortex_m_rt::entry;
use hal::prelude::*;
use nb::block;
use panic_semihosting as _;
use stm32f4xx_hal as hal;
use stm32f4xx_hal::rcc::Config;

#[entry]
fn main() -> ! {
    let dp = hal::pac::Peripherals::take().unwrap();
    let mut rcc = dp
        .RCC
        .freeze(Config::hse(25.MHz()).sysclk(84.MHz()).pclk1(42.MHz()));

    let gpioc = dp.GPIOC.split(&mut rcc);
    let mut led = gpioc.pc13.into_push_pull_output();
    let mut tmr = dp.TIM3.counter_hz(&mut rcc);
    tmr.start(10.Hz()).unwrap();

    loop {
        led.set_high();
        block!(tmr.wait()).ok();
        led.set_low();
        block!(tmr.wait()).ok();
    }
}
