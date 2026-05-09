#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt::entry;
use hal::prelude::*;
use panic_semihosting as _;
use stm32f4xx_hal as hal;
use stm32f4xx_hal::rcc::Config;

use hd44780_driver::Cursor;
use hd44780_driver::CursorBlink;
use hd44780_driver::Display;
use hd44780_driver::DisplayMode;
use hd44780_driver::HD44780;

#[entry]
fn main() -> ! {
    let dp = hal::pac::Peripherals::take().unwrap();
    let cp = cm::Peripherals::take().unwrap();
    let mut rcc = dp
        .RCC
        .freeze(Config::hse(25.MHz()).sysclk(84.MHz()).pclk1(42.MHz()));

    let gpioa = dp.GPIOA.split(&mut rcc);
    let gpiob = dp.GPIOB.split(&mut rcc);
    let gpioc = dp.GPIOC.split(&mut rcc);
    let mut delay = cp.SYST.delay(&rcc.clocks);

    let mut led = gpioc.pc13.into_push_pull_output();

    // init lcd

    let rs = gpiob.pb9.into_push_pull_output();
    let mut rw = gpiob.pb8.into_push_pull_output();
    let en = gpiob.pb7.into_push_pull_output();

    // this driver uses a write-only interface, so RW must stay low.
    rw.set_low();

    let mut b0 = gpiob.pb3.into_push_pull_output();
    let mut b1 = gpiob.pb4.into_push_pull_output();
    let mut b2 = gpiob.pb5.into_push_pull_output();
    let mut b3 = gpiob.pb6.into_push_pull_output();

    // not needed for 4-bit mode
    b0.set_low();
    b1.set_low();
    b2.set_low();
    b3.set_low();

    let b4 = gpioa.pa9.into_push_pull_output();
    let b5 = gpioa.pa10.into_push_pull_output();
    let b6 = gpioa.pa11.into_push_pull_output();
    let b7 = gpioa.pa12.into_push_pull_output();

    let mut lcd = HD44780::new_4bit(rs, en, b4, b5, b6, b7, &mut delay).unwrap();

    lcd.clear(&mut delay).unwrap();
    lcd.set_display_mode(
        DisplayMode {
            display: Display::On,
            cursor_visibility: Cursor::Invisible,
            cursor_blink: CursorBlink::Off,
        },
        &mut delay,
    )
    .unwrap();

    loop {
        lcd.clear(&mut delay).unwrap();
        lcd.set_cursor_pos(0x00, &mut delay).unwrap();
        lcd.write_str("HELLO", &mut delay).unwrap();

        for c in 0..=5 {
            lcd.set_cursor_pos(0x40, &mut delay).unwrap();
            lcd.write_str("ID: ", &mut delay).unwrap();
            lcd.write_byte(b'0' + (c / 10) as u8, &mut delay).unwrap();
            lcd.write_byte(b'0' + (c % 10) as u8, &mut delay).unwrap();

            led.set_high();
            delay.delay_ms(500u32);
            led.set_low();
            delay.delay_ms(500u32);
        }

        lcd.clear(&mut delay).unwrap();
        lcd.set_cursor_pos(0x00, &mut delay).unwrap();
        lcd.write_str("WORLD", &mut delay).unwrap();

        for c in (0..=5).rev() {
            lcd.set_cursor_pos(0x40, &mut delay).unwrap();
            lcd.write_str("ID: ", &mut delay).unwrap();
            lcd.write_byte(b'0' + (c / 10) as u8, &mut delay).unwrap();
            lcd.write_byte(b'0' + (c % 10) as u8, &mut delay).unwrap();

            led.set_high();
            delay.delay_ms(500u32);
            led.set_low();
            delay.delay_ms(500u32);
        }
    }
}
