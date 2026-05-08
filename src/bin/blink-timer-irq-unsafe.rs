#![no_main]
#![no_std]

use cortex_m as cm;
use cortex_m_rt as rt;
use cortex_m_semihosting::hprintln;
use panic_semihosting as _;
use rt::entry;
use stm32f1xx_hal::{
    gpio::{Output, PushPull, gpioc::PC13},
    pac,
    pac::{Interrupt, TIM3, interrupt},
    prelude::*,
    timer::{CounterMs, Event},
};

type LedT = PC13<Output<PushPull>>;
type TimT = CounterMs<TIM3>;

static mut G_LED: Option<LedT> = None;
static mut G_TMR: Option<TimT> = None;

#[entry]
fn main() -> ! {
    let mut cp = cm::peripheral::Peripherals::take().unwrap();
    let dp = pac::Peripherals::take().unwrap();
    let mut rcc = dp.RCC.constrain();

    // configure NVIC interrupts
    setup_interrupts(&mut cp);

    // configure PC13 pin to blink LED
    let mut gpioc = dp.GPIOC.split(&mut rcc);
    let led = gpioc.pc13.into_push_pull_output(&mut gpioc.crh);

    // configure and start TIM3 periodic timer
    let mut tmr = dp.TIM3.counter_ms(&mut rcc);
    tmr.start(1.secs()).unwrap();
    tmr.listen(Event::Update);

    unsafe {
        G_LED = Some(led);
        G_TMR = Some(tmr);
    }

    loop {
        hprintln!("MAIN LOOP");
        cm::asm::wfi();
    }
}

fn setup_interrupts(cp: &mut cm::peripheral::Peripherals) {
    let nvic = &mut cp.NVIC;

    // Enable TIM3 IRQ, set prio 1 and clear any pending IRQs
    unsafe {
        nvic.set_priority(Interrupt::TIM3, 1);
        cm::peripheral::NVIC::unmask(Interrupt::TIM3);
    }

    cm::peripheral::NVIC::unpend(Interrupt::TIM3);
}

#[interrupt]
fn TIM3() {
    hprintln!("BLINK");
    unsafe {
        let led_ptr = core::ptr::addr_of_mut!(G_LED);
        let tim_ptr = core::ptr::addr_of_mut!(G_TMR);

        if let Some(led) = (*led_ptr).as_mut() {
            if let Some(tim) = (*tim_ptr).as_mut() {
                tim.wait().ok();
                let _ = led.toggle();
            }
        }
    }
}
