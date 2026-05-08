#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt::{ExceptionFrame, entry, exception};
use cortex_m_semihosting::hprintln;
use panic_semihosting as _;
use stm32f1xx_hal::{pac, prelude::*, rcc};

#[entry]
fn main() -> ! {
    let p = pac::Peripherals::take().unwrap();
    let mut flash = p.FLASH.constrain();
    let mut rcc = p.RCC.freeze(
        rcc::Config::hse(8.MHz())
            .sysclk(56.MHz())
            .pclk1(28.MHz())
            .adcclk(14.MHz()),
        &mut flash.acr,
    );

    hprintln!("SYSCLK: {} Hz ...", rcc.clocks.sysclk());
    hprintln!("ADCCLK: {} Hz ...", rcc.clocks.adcclk());

    // ADC setup
    let mut adc = p.ADC1.adc(&mut rcc);

    loop {
        // Ambient temperature
        let temp = adc.read_temp();

        hprintln!("Temp: {} C", temp);

        delay(10000);
    }
}

fn delay(count: u32) {
    for _ in 0..count {
        cm::asm::nop();
    }
}

#[exception]
unsafe fn HardFault(ef: &ExceptionFrame) -> ! {
    panic!("HardFault at {:#?}", ef);
}

#[exception]
unsafe fn DefaultHandler(irqn: i16) {
    panic!("Unhandled exception (IRQn = {})", irqn);
}
