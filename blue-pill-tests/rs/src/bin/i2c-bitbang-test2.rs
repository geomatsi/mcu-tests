//
// I2C bitbang for AT24 flash
//

#![no_std]
#![no_main]

use bitbang_hal;
use cortex_m_rt as rt;
use cortex_m_semihosting::hprintln;
use eeprom24x;
use eeprom24x::Eeprom24x;
use eeprom24x::SlaveAddr;
use nb::block;
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
    let mut delay = dp.TIM2.counter_hz(&mut rcc);
    delay.start(10.Hz()).unwrap();
    let mut tmr = dp.TIM3.counter_hz(&mut rcc);
    tmr.start(200.kHz()).unwrap();
    let scl = gpioa.pa1.into_open_drain_output(&mut gpioa.crl);
    let sda = gpioa.pa2.into_open_drain_output(&mut gpioa.crl);

    let i2c = bitbang_i2c_compat::Eh1BitBangI2c::new(bitbang_hal::i2c::I2cBB::new(scl, sda, tmr));
    let mut eeprom = Eeprom24x::new_24x04(i2c, SlaveAddr::default());

    // check high memory addresses: 1 bit passed as a part of i2c addr
    let addrs1: [u32; 4] = [0x100, 0x10F, 0x1F0, 0x1EE];
    let byte_w1 = 0xe5;
    let addrs2: [u32; 4] = [0x00, 0x0F, 0xF0, 0xEE];
    let byte_w2 = 0xaa;

    for addr in addrs1.iter() {
        eeprom.write_byte(*addr, byte_w1).unwrap();
        // need to wait before next write
        block!(delay.wait()).ok();
    }

    for addr in addrs2.iter() {
        eeprom.write_byte(*addr, byte_w2).unwrap();
        // need to wait before next write
        block!(delay.wait()).ok();
    }

    loop {
        for addr in addrs1.iter() {
            let byte_r = eeprom.read_byte(*addr).unwrap();
            hprintln!("w1[{}] r[{}]", byte_w1, byte_r);
            block!(delay.wait()).ok();
        }

        for addr in addrs2.iter() {
            let byte_r = eeprom.read_byte(*addr).unwrap();
            hprintln!("w1[{}] r[{}]", byte_w2, byte_r);
            block!(delay.wait()).ok();
        }
    }
}
