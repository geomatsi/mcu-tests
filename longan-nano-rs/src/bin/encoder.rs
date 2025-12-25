#![no_std]
#![no_main]

use core::fmt::Write;
use embedded_graphics::mono_font::ascii::FONT_6X10;
use embedded_graphics::mono_font::MonoTextStyle;
use embedded_graphics::pixelcolor::Rgb565;
use embedded_graphics::prelude::*;
use embedded_graphics::primitives::{PrimitiveStyle, Rectangle};
use embedded_graphics::text::Text;
use embedded_hal_compat::eh0_2::digital::v2::InputPin;
use embedded_hal_compat::ForwardCompat;
use heapless::String;
use longan_nano::hal::delay::McycleDelay;
use longan_nano::hal::{pac, prelude::*};
use longan_nano::{lcd, lcd_pins};
use panic_halt as _;
use riscv_rt::entry;
use rotary_encoder_embedded::{Direction, RotaryEncoder};

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

    // encoder
    let dt = gpiob.pb13.into_pull_up_input();
    let clk = gpiob.pb14.into_pull_up_input();
    let button = gpiob.pb12.into_pull_up_input();
    let mut enc = RotaryEncoder::new(dt.forward(), clk.forward()).into_standard_mode();

    // lcd
    let lcd_pins = lcd_pins!(gpioa, gpiob);
    let mut display = lcd::configure(dp.SPI0, lcd_pins, &mut afio, &mut rcu);
    let (width, height) = (display.size().width as i32, display.size().height as i32);

    // grapics
    let t1 = MonoTextStyle::new(&FONT_6X10, Rgb565::GREEN);
    let t2 = MonoTextStyle::new(&FONT_6X10, Rgb565::YELLOW);
    let t3 = MonoTextStyle::new(&FONT_6X10, Rgb565::BLACK);
    let fc = PrimitiveStyle::with_fill(Rgb565::BLACK);

    // clear screen
    Rectangle::new(Point::new(0, 0), Size::new(width as u32, height as u32))
        .into_styled(fc)
        .draw(&mut display)
        .unwrap();

    // type text
    Text::new("Encoder:", Point::new(10, 10), t1)
        .draw(&mut display)
        .unwrap();

    // data
    let mut data = String::<64>::new();
    let mut ecount: isize = 0;
    let mut bstate: isize = 0;

    loop {
        match enc.update() {
            Direction::Clockwise => {
                ecount += 1;
                Text::new(data.as_str(), Point::new(10, 30), t3)
                    .draw(&mut display)
                    .unwrap();
                data.clear();
                let _ = write!(data, "Clockwise: {}", ecount);
                Text::new(data.as_str(), Point::new(10, 30), t2)
                    .draw(&mut display)
                    .unwrap();
            }
            Direction::Anticlockwise => {
                ecount -= 1;
                Text::new(data.as_str(), Point::new(10, 30), t3)
                    .draw(&mut display)
                    .unwrap();
                data.clear();
                let _ = write!(data, "Anticlockwise: {}", ecount);
                Text::new(data.as_str(), Point::new(10, 30), t2)
                    .draw(&mut display)
                    .unwrap();
            }
            Direction::None => {}
        }

        match button.is_low() {
            Ok(true) => {
                if bstate == 0 {
                    Text::new("Button: pressed", Point::new(10, 50), t2)
                        .draw(&mut display)
                        .unwrap();
                    bstate = 1;
                }
            }
            Ok(false) => {
                if bstate == 1 {
                    Text::new("Button: pressed", Point::new(10, 50), t3)
                        .draw(&mut display)
                        .unwrap();
                    bstate = 0;
                }
            }
            _ => {}
        }

        delay.delay_ms(1);
    }
}
