#![deny(unsafe_code)]
#![no_main]
#![no_std]

use panic_rtt_target as _;

#[rtic::app(device = stm32f1xx_hal::pac)]
mod app {
    use core::fmt::Write;
    use rtt_target::{UpChannel, rtt_init};
    use stm32f1xx_hal::{
        gpio::{Output, PushPull, gpioc::PC13},
        pac,
        prelude::*,
        timer::{CounterMs, Event},
    };

    #[shared]
    struct Shared {}

    #[local]
    struct Local {
        stream1: UpChannel,
        stream2: UpChannel,
        led: PC13<Output<PushPull>>,
        tim2: CounterMs<pac::TIM2>,
        tim3: CounterMs<pac::TIM3>,
    }

    #[init]
    fn init(cx: init::Context) -> (Shared, Local) {
        let channels = rtt_init! {
            up: {
                0: {
                    size: 512,
                    name: "stream1"
                }
                1: {
                    size: 512,
                    name: "stream2"
                }
            }
        };

        let stream1 = channels.up.0;
        let stream2 = channels.up.1;

        let mut flash = cx.device.FLASH.constrain();
        let mut rcc = cx.device.RCC.freeze(
            stm32f1xx_hal::rcc::Config::hse(8.MHz())
                .sysclk(8.MHz())
                .pclk1(8.MHz()),
            &mut flash.acr,
        );
        let mut gpioc = cx.device.GPIOC.split(&mut rcc);
        let led = gpioc.pc13.into_push_pull_output(&mut gpioc.crh);

        let mut tim2 = cx.device.TIM2.counter_ms(&mut rcc);
        tim2.start(1.secs()).unwrap();
        tim2.listen(Event::Update);

        let mut tim3 = cx.device.TIM3.counter_ms(&mut rcc);
        tim3.start(200.millis()).unwrap();
        tim3.listen(Event::Update);

        (
            Shared {},
            Local {
                stream1,
                stream2,
                led,
                tim2,
                tim3,
            },
        )
    }

    #[idle]
    fn idle(_cx: idle::Context) -> ! {
        loop {
            // Keep the core awake so host-side RTT attach does not time out.
            cortex_m::asm::nop();
        }
    }

    #[task(binds = TIM2, priority = 1, local = [beat: u8 = 0, tim2, stream1])]
    fn tim2(cx: tim2::Context) {
        writeln!(cx.local.stream1, "TIM2 beat = {}", *cx.local.beat).ok();
        *cx.local.beat = cx.local.beat.wrapping_add(1);
        cx.local.tim2.clear_interrupt(Event::Update);
    }

    #[task(binds = TIM3, priority = 1, local = [led, tim3, stream2])]
    fn tim3(cx: tim3::Context) {
        writeln!(cx.local.stream2, "TIM3 blink").ok();
        cx.local.led.toggle();
        cx.local.tim3.clear_interrupt(Event::Update);
    }
}
