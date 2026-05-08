#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt::entry;
use panic_rtt_target as _;
use rtt_target::{rprintln, rtt_init_print};
use smart_leds::{RGB8, SmartLedsWrite};
use stm32f1xx_hal::{
    pac,
    prelude::*,
    rcc,
    spi::{Mode, Phase, Polarity},
};

const NUM_LEDS: usize = 8;
const MODE: Mode = Mode {
    polarity: Polarity::IdleLow,
    phase: Phase::CaptureOnFirstTransition,
};

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let cp = cm::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let mut rcc = dp.RCC.freeze(
        rcc::Config::hse(8.MHz()).sysclk(48.MHz()).pclk1(24.MHz()),
        &mut flash.acr,
    );

    rtt_init_print!();

    let mut delay = cp.SYST.delay(&rcc.clocks);

    let mut gpiob = dp.GPIOB.split(&mut rcc);
    let spi = dp.SPI2.spi(
        (
            Some(gpiob.pb13.into_alternate_push_pull(&mut gpiob.crh)),
            Some(gpiob.pb14.into_floating_input(&mut gpiob.crh)),
            Some(gpiob.pb15.into_alternate_push_pull(&mut gpiob.crh)),
        ),
        MODE,
        3.MHz(),
        &mut rcc,
    );

    rprintln!("ready to go...");

    let mut ws = ws2812_spi::Ws2812::new(spi);
    let mut pdata = [RGB8::default(); NUM_LEDS];
    let cdata: [RGB8; 3] = [
        RGB8 {
            r: 0x10,
            g: 0x0,
            b: 0x0,
        },
        RGB8 {
            r: 0x0,
            g: 0x10,
            b: 0x0,
        },
        RGB8 {
            r: 0x0,
            g: 0x0,
            b: 0x10,
        },
    ];
    let mut p: usize = 0;
    let mut c: usize = 0;

    loop {
        let pos = p.wrapping_rem(NUM_LEDS);
        let color = c.wrapping_rem(3);

        rprintln!("iteration: pos {} color {}...", pos, color);

        pdata[pos] = cdata[color];
        ws.write(pdata.iter().cloned()).unwrap();
        delay.delay_ms(100u16);

        if pos == 7 {
            c += 1;
        }

        p += 1;
    }
}
