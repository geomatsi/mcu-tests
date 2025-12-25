#![no_std]
#![no_main]

use core::cell::RefCell;
use core::ops::DerefMut;
use critical_section::Mutex;
use gd32vf103xx_hal::pac::Interrupt;
use gd32vf103xx_hal::timer;
use gd32vf103xx_hal::timer::Timer;
use longan_nano::hal::{eclic::*, pac, pac::*, prelude::*};
use longan_nano::led::{rgb, Led, BLUE, GREEN};
use panic_halt as _;
use riscv_rt::entry;

static G_TMR: Mutex<RefCell<Option<Timer<TIMER1>>>> = Mutex::new(RefCell::new(None));
static G_LED: Mutex<RefCell<Option<GREEN>>> = Mutex::new(RefCell::new(None));
static B_LED: Mutex<RefCell<Option<BLUE>>> = Mutex::new(RefCell::new(None));

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let mut rcu = dp
        .RCU
        .configure()
        .ext_hf_clock(8.mhz())
        .sysclk(108.mhz())
        .freeze();

    let gpioa = dp.GPIOA.split(&mut rcu);
    let gpioc = dp.GPIOC.split(&mut rcu);

    let (mut red, mut green, mut blue) = rgb(gpioc.pc13, gpioa.pa1, gpioa.pa2);
    red.off();
    green.on();
    blue.off();

    let mut timer = Timer::timer1(dp.TIMER1, 1.hz(), &mut rcu);
    timer.listen(timer::Event::Update);

    critical_section::with(|cs| {
        G_TMR.borrow(cs).replace(Some(timer));
        G_LED.borrow(cs).replace(Some(green));
        B_LED.borrow(cs).replace(Some(blue));
    });

    ECLIC::reset();
    ECLIC::set_threshold_level(Level::L0);
    ECLIC::set_level_priority_bits(LevelPriorityBits::L3P1);

    ECLIC::setup(
        Interrupt::TIMER1,
        TriggerType::Level,
        Level::L1,
        Priority::P1,
    );

    unsafe {
        ECLIC::unmask(Interrupt::TIMER1);
        riscv::interrupt::enable();
    };

    loop {
        unsafe {
            riscv::asm::wfi();
        }
    }
}

#[allow(non_snake_case)]
#[no_mangle]
fn TIMER1() {
    critical_section::with(|cs| {
        if let Some(ref mut tim) = G_TMR.borrow(cs).borrow_mut().deref_mut() {
            tim.clear_update_interrupt_flag();
        }
    });

    critical_section::with(|cs| {
        if let (Some(ref mut g), Some(ref mut b)) = (
            G_LED.borrow(cs).borrow_mut().deref_mut(),
            B_LED.borrow(cs).borrow_mut().deref_mut(),
        ) {
            if g.is_on() {
                g.off();
            } else {
                g.on();
            }

            if b.is_on() {
                b.off();
            } else {
                b.on();
            }
        }
    });
}
