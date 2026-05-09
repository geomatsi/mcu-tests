//
// I2C bitbang for LM75A temperature sensor
//

#![no_std]
#![no_main]

use bitbang_hal;
use cortex_m as cm;
use cortex_m_rt as rt;
use cortex_m_semihosting::hprintln;
use lm75::{Address, Lm75};
use panic_semihosting as _;
use rt::entry;
use stm32f1xx_hal::{pac, prelude::*, rcc};

#[path = "../bitbang_i2c_compat.rs"]
mod bitbang_i2c_compat;

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let mut rcc = dp.RCC.freeze(
        rcc::Config::hse(8.MHz()).sysclk(32.MHz()).pclk1(16.MHz()),
        &mut flash.acr,
    );
    let mut gpioa = dp.GPIOA.split(&mut rcc);
    let mut tmr = dp.TIM3.counter_hz(&mut rcc);
    tmr.start(200.kHz()).unwrap();
    let scl = gpioa.pa1.into_open_drain_output(&mut gpioa.crl);
    let sda = gpioa.pa2.into_open_drain_output(&mut gpioa.crl);

    let i2c = bitbang_i2c_compat::Eh1BitBangI2c::new(bitbang_hal::i2c::I2cBB::new(scl, sda, tmr));
    let mut sensor = Lm75::new(i2c, Address::default());

    loop {
        let temp = sensor.read_temperature().unwrap();
        hprintln!("T: {}", temp);
        delay(5000);
    }
}

fn delay(count: u32) {
    for _ in 0..count {
        cm::asm::nop();
    }
}
