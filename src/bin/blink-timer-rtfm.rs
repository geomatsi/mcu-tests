#![deny(unsafe_code)]
#![no_main]
#![no_std]

use panic_rtt_target as _;

#[rtic::app(device = stm32f1xx_hal::pac)]
mod app {
    use rtt_target::{rprintln, rtt_init_print};
    use stm32f1xx_hal::{
        gpio::{gpioc::PC13, Output, PinState, PushPull},
        pac,
        prelude::*,
        timer::{CounterMs, Event},
    };

    #[shared]
    struct Shared {}

    #[local]
    struct Local {
        beat: u8,
        led: PC13<Output<PushPull>>,
        tim2: CounterMs<pac::TIM2>,
        tim3: CounterMs<pac::TIM3>,
    }

    #[init]
    fn init(cx: init::Context) -> (Shared, Local) {
        rtt_init_print!();

        let mut rcc = cx.device.RCC.constrain();
        let mut gpioc = cx.device.GPIOC.split(&mut rcc);
        let led = gpioc
            .pc13
            .into_push_pull_output_with_state(&mut gpioc.crh, PinState::High);

        let mut tim2 = cx.device.TIM2.counter_ms(&mut rcc);
        tim2.start(1.secs()).unwrap();
        tim2.listen(Event::Update);

        let mut tim3 = cx.device.TIM3.counter_ms(&mut rcc);
        tim3.start(200.millis()).unwrap();
        tim3.listen(Event::Update);

        (
            Shared {},
            Local {
                beat: 0,
                led,
                tim2,
                tim3,
            },
        )
    }

    #[idle]
    fn idle(_cx: idle::Context) -> ! {
        loop {
            cortex_m::asm::wfi();
        }
    }

    #[task(binds = TIM2, priority = 1, local = [beat, tim2])]
    fn tim2(cx: tim2::Context) {
        rprintln!("TIM2 beat = {}", *cx.local.beat);
        *cx.local.beat = cx.local.beat.wrapping_add(1);
        cx.local.tim2.clear_interrupt(Event::Update);
    }

    #[task(binds = TIM3, priority = 1, local = [led, tim3])]
    fn tim3(cx: tim3::Context) {
        rprintln!("TIM3 blink");
        if cx.local.led.is_set_low() {
            cx.local.led.set_high();
        } else {
            cx.local.led.set_low();
        }
        cx.local.tim3.clear_interrupt(Event::Update);
    }
}
