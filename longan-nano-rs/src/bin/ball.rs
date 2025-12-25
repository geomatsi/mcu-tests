#![no_std]
#![no_main]

use panic_halt as _;

use core::ops::Neg;
use embedded_graphics::pixelcolor::Rgb565;
use embedded_graphics::prelude::*;
use embedded_graphics::primitives::{Circle, PrimitiveStyle, Rectangle};
use longan_nano::hal::delay::McycleDelay;
use longan_nano::hal::{pac, prelude::*};
use longan_nano::{lcd, lcd_pins};
use riscv_rt::entry;

#[derive(Debug, Clone, Copy, Default)]
pub struct Ball {
    px: i32,
    py: i32,
    vx: i32,
    vy: i32,
    r: u32,
    dt: u32,
}

impl Ball {
    pub fn new(px: i32, py: i32, vx: i32, vy: i32, r: u32, dt: u32) -> Ball {
        Ball {
            px,
            py,
            vx,
            vy,
            r,
            dt,
        }
    }

    pub fn get_x(&self) -> i32 {
        self.px
    }

    pub fn get_y(&self) -> i32 {
        self.py
    }

    pub fn get_r(&self) -> u32 {
        self.r
    }

    pub fn step(&mut self) {
        self.px += self.vx * (self.dt as i32);
        self.py += self.vy * (self.dt as i32);
    }

    // bounce from the walls
    pub fn bounce(&mut self, wmin: i32, wmax: i32, hmin: i32, hmax: i32) -> bool {
        if self.px + (self.r as i32) >= wmax {
            self.vx = self.vx.neg();
            self.px = wmax - (self.r as i32);
            return true;
        }

        if self.px - (self.r as i32) <= wmin {
            self.vx = self.vx.neg();
            self.px = wmin + (self.r as i32);
            return true;
        }

        if self.py + (self.r as i32) >= hmax {
            self.vy = self.vy.neg();
            self.py = hmax - (self.r as i32);
            return true;
        }

        if self.py - (self.r as i32) <= hmin {
            self.vy = self.vy.neg();
            self.py = hmin + (self.r as i32);
            return true;
        }

        false
    }
}

fn area(p: &Ball) -> (Point, Point) {
    (
        Point::new(p.get_x() - p.get_r() as i32, p.get_y() - p.get_r() as i32),
        Point::new(p.get_x() + p.get_r() as i32, p.get_y() + p.get_r() as i32),
    )
}

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

    let fc = PrimitiveStyle::with_fill(Rgb565::BLACK);
    let bc = PrimitiveStyle::with_fill(Rgb565::GREEN);
    let mut ball = Ball::new(3, 3, 2, 7, 10, 1);

    // clear screen
    Rectangle::new(Point::new(0, 0), Size::new(width as u32, height as u32))
        .into_styled(fc)
        .draw(&mut display)
        .unwrap();

    loop {
        Rectangle::with_corners(area(&ball).0, area(&ball).1)
            .into_styled(fc)
            .draw(&mut display)
            .unwrap();

        ball.bounce(0, width, 0, height);
        ball.step();

        Circle::new(Point::new(ball.get_x(), ball.get_y()), ball.get_r())
            .into_styled(bc)
            .draw(&mut display)
            .unwrap();

        delay.delay_ms(100);
    }
}
