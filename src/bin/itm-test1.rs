#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt::entry;
use hal::time::Hertz;
use panic_itm as _;
use stm32f1xx_hal as hal;
use stm32f1xx_hal::{pac, prelude::*, rcc};

use core::ptr;
use itm_logger as itm;

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let mut cp = cm::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let rcc = dp.RCC.freeze(
        rcc::Config::hse(8.MHz()).sysclk(32.MHz()).pclk1(16.MHz()),
        &mut flash.acr,
    );
    let mut delay = cp.SYST.delay(&rcc.clocks);
    let sysclk: Hertz = rcc.clocks.sysclk();

    cp.DCB.enable_trace();
    //cp.DCB.disable_trace();

    // enable ITM: from f3 examples
    unsafe {
        // enable TPIU and ITM: done by DCB API
        // cp.DCB.demcr.modify(|r| r | (1 << 24));

        // prescaler: done by itm_logger
        // let swo_freq = 2_000_000;
        // cp.TPIU.acpr.write((clocks.sysclk().0 / swo_freq) - 1);

        // SWO NRZ
        cp.TPIU.sppr.write(2);

        cp.TPIU.ffcr.modify(|r| r & !(1 << 1));

        // STM32 specific: enable tracing in the DBGMCU_CR register
        const DBGMCU_CR: *mut u32 = 0xe004_2004 as *mut u32;
        let r = ptr::read_volatile(DBGMCU_CR);
        ptr::write_volatile(DBGMCU_CR, r | (1 << 5));

        // unlock the ITM
        cp.ITM.lar.write(0xC5AC_CE55);

        cp.ITM.tcr.write(
            (0b00_0001 << 16) | // TraceBusID
            (1 << 3) | // enable SWO output
            (1 << 0), // enable the ITM
        );

        // enable stimulus port 0
        cp.ITM.ter[0].write(1);
    }

    itm::init_with_level(itm_logger::Level::Info).ok();
    itm::update_tpiu_baudrate(sysclk.raw(), 2_000_000)
        .expect("Failed to reset TPIU baudrate");

    loop {
        itm::debug!("ITM debug");
        itm::info!("ITM info");
        itm::warn!("ITM warn");
        delay.delay_ms(1_000u16);
    }
}
