#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt::entry;
use cortex_m_semihosting::hprintln;
use nb::block;
use panic_semihosting as _;
use stm32f1xx_hal::{pac, prelude::*, rcc, serial::Config};

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let cp = cm::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let mut rcc = dp.RCC.freeze(
        rcc::Config::hse(8.MHz()).sysclk(32.MHz()).pclk1(16.MHz()),
        &mut flash.acr,
    );
    let mut gpiob = dp.GPIOB.split(&mut rcc);
    let mut delay = cp.SYST.delay(&rcc.clocks);

    let tx = gpiob.pb10.into_alternate_push_pull(&mut gpiob.crh);
    let rx = gpiob.pb11;

    let mut serial = dp
        .USART3
        .serial((tx, rx), Config::default().baudrate(115_200.bps()), &mut rcc);

    loop {
        hprintln!("Hello World!");
        for byte in b"Hello, World!\r\n" {
            block!(serial.tx.write_u8(*byte)).unwrap();
        }
        delay.delay_ms(1_000u16);
    }
}
