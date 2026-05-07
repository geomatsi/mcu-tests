#![deny(unsafe_code)]
#![no_main]
#![no_std]

use cortex_m_rt::entry;
use mfrc522::{comm::eh02::spi::SpiInterface, Mfrc522};
use panic_rtt_target as _;
use rtt_target::{rprintln, rtt_init_print};
use stm32f1xx_hal::{
    pac,
    prelude::*,
    spi::{Mode, Phase, Polarity, Spi},
};

pub const MODE: Mode = Mode {
    polarity: Polarity::IdleLow,
    phase: Phase::CaptureOnFirstTransition,
};

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let mut rcc = dp.RCC.constrain();

    rtt_init_print!();

    let mut gpioa = dp.GPIOA.split(&mut rcc);
    let mut gpioc = dp.GPIOC.split(&mut rcc);

    let mut led = gpioc.pc13.into_push_pull_output(&mut gpioc.crh);
    let _ = led.set_high();

    let sck = gpioa.pa5;
    let miso = gpioa.pa6;
    let mosi = gpioa.pa7;
    let spi = Spi::new(
        dp.SPI1,
        (Some(sck), Some(miso), Some(mosi)),
        MODE,
        1.MHz(),
        &mut rcc,
    );

    let nss = gpioa.pa4.into_push_pull_output(&mut gpioa.crl);
    let itf = SpiInterface::new(spi).with_nss(nss);
    let mut mfrc522 = Mfrc522::new(itf).init().unwrap();

    loop {
        if let Ok(atqa) = mfrc522.reqa() {
            if let Ok(uid) = mfrc522.select(&atqa) {
                rprintln!("* {:?}", uid.as_bytes());
            }
        }
    }
}
