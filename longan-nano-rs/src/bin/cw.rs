#![no_std]
#![no_main]

use bitbang_hal::i2c::I2cBB;
use gd32vf103xx_hal as hal;
use hal::timer::Timer;
use longan_nano::hal::delay::McycleDelay;
use longan_nano::hal::{pac, prelude::*};
use longan_nano::led::{rgb, Led};
use panic_halt as _;
use riscv_rt::entry;
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
    let mut rcu = dp.RCU.configure().freeze();

    let gpioa = dp.GPIOA.split(&mut rcu);
    let gpiob = dp.GPIOB.split(&mut rcu);
    let gpioc = dp.GPIOC.split(&mut rcu);

    let (mut red, mut green, mut blue) = rgb(gpioc.pc13, gpioa.pa1, gpioa.pa2);
    let leds: [&mut dyn Led; 3] = [&mut red, &mut green, &mut blue];

    leds[0].off();
    leds[1].off();
    leds[2].off();

    // bitbang i2c
    let tmr = Timer::timer1(dp.TIMER1, 200.khz(), &mut rcu);
    let scl = gpiob.pb8.into_open_drain_output();
    let sda = gpiob.pb9.into_open_drain_output();
    let i2c = bitbang_hal::i2c::I2cBB::new(scl, sda, tmr);

    // si5351
    let mut clock = Si5351Device::new_adafruit_module(i2c);
    clock.init_adafruit_module().unwrap();
    clock
        .set_frequency(si5351::PLL::A, si5351::ClockOutput::Clk0, 14_300_000)
        .unwrap();

    // delay
    let mut delay = McycleDelay::new(&rcu.clocks);

    loop {
        leds[0].on();
        leds[1].off();
        leds[2].off();

        // send 'V'
        cw_sym(&mut clock, &mut delay, '.');
        cw_sym(&mut clock, &mut delay, '.');
        cw_sym(&mut clock, &mut delay, '.');
        cw_sym(&mut clock, &mut delay, '_');

        // inter-word interval
        delay.delay_ms(CW_DOT);

        // send 'V'
        cw_sym(&mut clock, &mut delay, '.');
        cw_sym(&mut clock, &mut delay, '.');
        cw_sym(&mut clock, &mut delay, '.');
        cw_sym(&mut clock, &mut delay, '_');

        leds[0].off();
        leds[1].on();
        leds[2].off();

        delay.delay_ms(1000);
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
