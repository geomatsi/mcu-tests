#![no_std]
#![no_main]

use bitbang_hal::i2c::I2cBB;
use core::fmt::Write;
use embedded_graphics::mono_font::ascii::FONT_6X10;
use embedded_graphics::mono_font::MonoTextStyle;
use embedded_graphics::pixelcolor::Rgb565;
use embedded_graphics::prelude::*;
use embedded_graphics::primitives::{PrimitiveStyle, Rectangle};
use embedded_graphics::text::Text;
use embedded_hal_compat::eh0_2::digital::v2::InputPin;
use embedded_hal_compat::ForwardCompat;
use gd32vf103xx_hal as hal;
use hal::timer::Timer;
use heapless::String;
use longan_nano::hal::delay::McycleDelay;
use longan_nano::hal::{pac, prelude::*};
use longan_nano::led::{rgb, Led};
use longan_nano::{lcd, lcd_pins};
use panic_halt as _;
use riscv_rt::entry;
use rotary_encoder_embedded::{Direction, RotaryEncoder};
use si5351::{Si5351, Si5351Device};

type SiI2C = I2cBB<
    hal::gpio::gpiob::PB8<hal::gpio::Output<hal::gpio::OpenDrain>>,
    hal::gpio::gpiob::PB9<hal::gpio::Output<hal::gpio::OpenDrain>>,
    Timer<hal::pac::TIMER1>,
>;

const CW_DOT: u32 = 120;

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let mut rcu = dp
        .RCU
        .configure()
        .ext_hf_clock(8.mhz())
        .sysclk(108.mhz())
        .freeze();
    let mut afio = dp.AFIO.constrain(&mut rcu);

    let gpioa = dp.GPIOA.split(&mut rcu);
    let gpiob = dp.GPIOB.split(&mut rcu);
    let gpioc = dp.GPIOC.split(&mut rcu);

    // leds
    let (mut red, mut green, mut blue) = rgb(gpioc.pc13, gpioa.pa1, gpioa.pa2);
    let leds: [&mut dyn Led; 3] = [&mut red, &mut green, &mut blue];

    leds[0].off();
    leds[1].off();
    leds[2].on();

    // encoder
    let dt = gpiob.pb13.into_pull_up_input();
    let clk = gpiob.pb14.into_pull_up_input();
    let button = gpiob.pb12.into_pull_up_input();
    let mut enc = RotaryEncoder::new(dt.forward(), clk.forward()).into_standard_mode();

    // lcd
    let lcd_pins = lcd_pins!(gpioa, gpiob);
    let mut display = lcd::configure(dp.SPI0, lcd_pins, &mut afio, &mut rcu);
    let (width, height) = (display.size().width as i32, display.size().height as i32);

    // bitbang i2c
    let tmr = Timer::timer1(dp.TIMER1, 200.khz(), &mut rcu);
    let scl = gpiob.pb8.into_open_drain_output();
    let sda = gpiob.pb9.into_open_drain_output();
    let i2c = bitbang_hal::i2c::I2cBB::new(scl, sda, tmr);

    // si5351
    let mut clock = Si5351Device::new_adafruit_module(i2c);
    clock.init_adafruit_module().unwrap();

    // delay
    let mut delay = McycleDelay::new(&rcu.clocks);

    // data
    let mut data = String::<64>::new();
    let mut freq: u32 = 14_300_000;
    let mut bstate: isize = 0;

    // grapics
    let print = MonoTextStyle::new(&FONT_6X10, Rgb565::YELLOW);
    let clear = MonoTextStyle::new(&FONT_6X10, Rgb565::BLACK);
    let fc = PrimitiveStyle::with_fill(Rgb565::BLACK);

    // clear screen
    Rectangle::new(Point::new(0, 0), Size::new(width as u32, height as u32))
        .into_styled(fc)
        .draw(&mut display)
        .unwrap();

    // type initial freq
    let _ = write!(data, "Freq: {}", freq);
    Text::new(data.as_str(), Point::new(10, 30), print)
        .draw(&mut display)
        .unwrap();

    loop {
        match enc.update() {
            Direction::Clockwise => {
                freq += 500;
                Text::new(data.as_str(), Point::new(10, 30), clear)
                    .draw(&mut display)
                    .unwrap();
                data.clear();
                let _ = write!(data, "Freq: {}", freq);
                Text::new(data.as_str(), Point::new(10, 30), print)
                    .draw(&mut display)
                    .unwrap();
            }
            Direction::Anticlockwise => {
                freq -= 500;
                Text::new(data.as_str(), Point::new(10, 30), clear)
                    .draw(&mut display)
                    .unwrap();
                data.clear();
                let _ = write!(data, "Freq: {}", freq);
                Text::new(data.as_str(), Point::new(10, 30), print)
                    .draw(&mut display)
                    .unwrap();
            }
            Direction::None => {}
        }

        match button.is_low() {
            Ok(true) => {
                if bstate == 0 {
                    Text::new("Ready to transmit...", Point::new(10, 50), print)
                        .draw(&mut display)
                        .unwrap();
                    bstate = 1;
                }
            }
            Ok(false) => {
                if bstate == 1 {
                    Text::new("Ready to transmit...", Point::new(10, 50), clear)
                        .draw(&mut display)
                        .unwrap();
                    bstate = 0;

                    Text::new("Transmitting...", Point::new(10, 50), print)
                        .draw(&mut display)
                        .unwrap();

                    // on button release set freq and transmit
                    clock
                        .set_frequency(si5351::PLL::A, si5351::ClockOutput::Clk0, freq)
                        .unwrap();

                    for _ in 1..5 {
                        leds[0].on();
                        leds[1].off();
                        leds[2].off();

                        // send 'V'
                        cw_sym(&mut clock, &mut delay, '.');
                        cw_sym(&mut clock, &mut delay, '.');
                        cw_sym(&mut clock, &mut delay, '.');
                        cw_sym(&mut clock, &mut delay, '_');

                        leds[0].off();
                        leds[1].on();
                        leds[2].off();

                        // inter-word interval
                        delay.delay_ms(CW_DOT);
                    }

                    Text::new("Transmitting...", Point::new(10, 50), clear)
                        .draw(&mut display)
                        .unwrap();

                    leds[0].off();
                    leds[1].off();
                    leds[2].on();
                }
            }
            _ => {}
        }

        delay.delay_ms(1);
    }
}

fn cw_sym(clock: &mut Si5351Device<SiI2C>, delay: &mut McycleDelay, ch: char) {
    clock.set_clock_enabled(si5351::ClockOutput::Clk0, true);
    clock.flush_output_enabled().unwrap();

    match ch {
        '.' => {
            delay.delay_ms(CW_DOT);
        }
        '_' => {
            delay.delay_ms(CW_DOT * 3);
        }
        _ => {}
    }

    clock.set_clock_enabled(si5351::ClockOutput::Clk0, false);
    clock.flush_output_enabled().unwrap();

    // inter-symbol interval
    delay.delay_ms(CW_DOT);
}
