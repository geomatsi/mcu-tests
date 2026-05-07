#![no_main]
#![no_std]

use core::cell::RefCell;

use cortex_m::{asm::wfi, interrupt::Mutex};
use cortex_m_rt::entry;
use panic_semihosting as _;
use stm32f1xx_hal::{
    gpio::{gpioc::PC13, Output, PushPull},
    pac::{interrupt, Interrupt, Peripherals, TIM2},
    prelude::*,
    timer::{CounterMs, Event},
};

type LedT = PC13<Output<PushPull>>;

static G_LED: Mutex<RefCell<Option<LedT>>> = Mutex::new(RefCell::new(None));
static G_TIM: Mutex<RefCell<Option<CounterMs<TIM2>>>> = Mutex::new(RefCell::new(None));

#[entry]
fn main() -> ! {
    let dp = Peripherals::take().unwrap();
    let mut rcc = dp.RCC.constrain();

    let mut gpioc = dp.GPIOC.split(&mut rcc);
    let led = gpioc.pc13.into_push_pull_output(&mut gpioc.crh);

    let mut tim = dp.TIM2.counter_ms(&mut rcc);
    tim.start(1.secs()).unwrap();
    tim.listen(Event::Update);

    cortex_m::interrupt::free(|cs| {
        *G_LED.borrow(cs).borrow_mut() = Some(led);
        *G_TIM.borrow(cs).borrow_mut() = Some(tim);
    });

    unsafe {
        cortex_m::peripheral::NVIC::unmask(Interrupt::TIM2);
    }

    loop {
        wfi();
    }
}

#[interrupt]
fn TIM2() {
    static mut LED: Option<LedT> = None;
    static mut TIM: Option<CounterMs<TIM2>> = None;

    let led = LED.get_or_insert_with(|| {
        cortex_m::interrupt::free(|cs| G_LED.borrow(cs).replace(None).unwrap())
    });
    let tim = TIM.get_or_insert_with(|| {
        cortex_m::interrupt::free(|cs| G_TIM.borrow(cs).replace(None).unwrap())
    });

    let _ = led.toggle();
    tim.wait().ok();
}
