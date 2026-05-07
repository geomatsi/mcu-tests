#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt as rt;
use panic_rtt_target as _;
use rt::entry;
use rtt_target::{rprintln, rtt_init_print};
use stm32f1xx_hal::{pac, prelude::*, rcc};

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let mut rcc = dp.RCC.freeze(
        rcc::Config::hse(8.MHz())
            .sysclk(16.MHz())
            .pclk1(4.MHz())
            .adcclk(2.MHz()),
        &mut flash.acr,
    );
    let mut gpioc = dp.GPIOC.split(&mut rcc);
    let mut led = gpioc.pc13.into_push_pull_output(&mut gpioc.crh);

    rtt_init_print!();

    loop {
        rprintln!("Hello World!");

        let _ = led.set_high();
        delay(5000);
        let _ = led.set_low();
        delay(500);
    }
}

fn delay(count: u32) {
    for _ in 0..count {
        cm::asm::nop();
    }
}
