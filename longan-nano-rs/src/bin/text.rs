#![no_std]
#![no_main]

use panic_halt as _;

use embedded_graphics::mono_font::MonoTextStyle;
use embedded_graphics::mono_font::{ascii::FONT_10X20, ascii::FONT_6X10};
use embedded_graphics::pixelcolor::Rgb565;
use embedded_graphics::prelude::*;
use embedded_graphics::primitives::{PrimitiveStyle, Rectangle};
use embedded_graphics::text::Text;
use longan_nano::hal::delay::McycleDelay;
use longan_nano::hal::{pac, prelude::*};
use longan_nano::{lcd, lcd_pins};
use riscv_rt::entry;

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();

    // Configure clocks
    let mut rcu = dp
        .RCU
        .configure()
        .ext_hf_clock(8.mhz())
        .sysclk(108.mhz())
        .freeze();
    let mut afio = dp.AFIO.constrain(&mut rcu);
    let mut delay = McycleDelay::new(&rcu.clocks);

    let gpioa = dp.GPIOA.split(&mut rcu);
    let gpiob = dp.GPIOB.split(&mut rcu);

    let lcd_pins = lcd_pins!(gpioa, gpiob);
    let mut display = lcd::configure(dp.SPI0, lcd_pins, &mut afio, &mut rcu);
    let (width, height) = (display.size().width as i32, display.size().height as i32);

    let t1 = MonoTextStyle::new(&FONT_6X10, Rgb565::GREEN);
    let t2 = MonoTextStyle::new(&FONT_10X20, Rgb565::BLUE);
    let t3 = MonoTextStyle::new(&FONT_10X20, Rgb565::YELLOW);
    let fc = PrimitiveStyle::with_fill(Rgb565::BLACK);

    // clear screen
    Rectangle::new(Point::new(0, 0), Size::new(width as u32, height as u32))
        .into_styled(fc)
        .draw(&mut display)
        .unwrap();

    // type text
    Text::new("MENU", Point::new(10, 10), t1)
        .draw(&mut display)
        .unwrap();
    Text::new("Frequency", Point::new(10, 30), t2)
        .draw(&mut display)
        .unwrap();
    Text::new("Signal", Point::new(10, 60), t3)
        .draw(&mut display)
        .unwrap();

    loop {
        delay.delay_ms(100);
    }
}
