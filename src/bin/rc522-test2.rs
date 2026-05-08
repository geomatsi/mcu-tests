#![deny(unsafe_code)]
#![no_main]
#![no_std]

use panic_rtt_target as _;

#[rtic::app(device = stm32f1xx_hal::pac)]
mod app {
    use core::fmt::Write;
    use mfrc522::{
        Initialized, Mfrc522,
        comm::eh02::spi::{DummyDelay, SpiInterface},
    };
    use rtt_target::{UpChannel, rtt_init};
    use stm32f1xx_hal::{
        gpio::{
            Alternate, Edge, ExtiPin, Floating, Input, Output, PinState, PushPull,
            gpioa::PA15,
            gpiob::{PB1, PB3, PB4, PB5},
            gpioc::PC13,
        },
        pac,
        prelude::*,
        spi::{Mode, Phase, Polarity, Spi},
        timer::{CounterMs, Event},
    };

    type SpiPins = (
        Option<PB3<Alternate<PushPull>>>,
        Option<PB4<Input<Floating>>>,
        Option<PB5<Alternate<PushPull>>>,
    );
    type SpiType = Spi<pac::SPI1, u8>;
    type NfcType = Mfrc522<SpiInterface<SpiType, PA15<Output<PushPull>>, DummyDelay>, Initialized>;

    const MODE: Mode = Mode {
        polarity: Polarity::IdleLow,
        phase: Phase::CaptureOnFirstTransition,
    };

    #[shared]
    struct Shared {}

    #[local]
    struct Local {
        stream1: UpChannel,
        stream2: UpChannel,
        tim3: CounterMs<pac::TIM3>,
        led: PC13<Output<PushPull>>,
        irq: PB1<Input<Floating>>,
        nfc: NfcType,
    }

    #[init]
    fn init(mut cx: init::Context) -> (Shared, Local) {
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

        let mut rcc = cx.device.RCC.constrain();
        let mut afio = cx.device.AFIO.constrain(&mut rcc);

        let mut gpioa = cx.device.GPIOA.split(&mut rcc);
        let mut gpiob = cx.device.GPIOB.split(&mut rcc);
        let mut gpioc = cx.device.GPIOC.split(&mut rcc);

        let led = gpioc
            .pc13
            .into_push_pull_output_with_state(&mut gpioc.crh, PinState::High);

        let mut tim3 = cx.device.TIM3.counter_ms(&mut rcc);
        tim3.start(200.millis()).unwrap();
        tim3.listen(Event::Update);

        let mut irq = gpiob.pb1.into_floating_input(&mut gpiob.crl);
        irq.make_interrupt_source(&mut afio);
        irq.enable_interrupt(&mut cx.device.EXTI);
        irq.trigger_on_edge(&mut cx.device.EXTI, Edge::RisingFalling);

        let (pa15, pb3, pb4) = afio.mapr.disable_jtag(gpioa.pa15, gpiob.pb3, gpiob.pb4);
        let pins: SpiPins = (
            Some(pb3.into_alternate_push_pull(&mut gpiob.crl)),
            Some(pb4),
            Some(gpiob.pb5.into_alternate_push_pull(&mut gpiob.crl)),
        );
        let spi = cx
            .device
            .SPI1
            .remap(&mut afio.mapr)
            .spi(pins, MODE, 1.MHz(), &mut rcc);

        let nss = pa15.into_push_pull_output(&mut gpioa.crh);
        let itf = SpiInterface::new(spi).with_nss(nss);
        let nfc = Mfrc522::new(itf).init().unwrap();

        (
            Shared {},
            Local {
                stream1,
                stream2,
                tim3,
                led,
                irq,
                nfc,
            },
        )
    }

    #[idle]
    fn idle(_cx: idle::Context) -> ! {
        loop {
            cortex_m::asm::nop();
        }
    }

    #[task(binds = EXTI1, priority = 2, local = [irq, nfc, stream1])]
    fn exti1(cx: exti1::Context) {
        if cx.local.irq.check_interrupt() {
            match cx.local.nfc.reqa() {
                Ok(atqa) => match cx.local.nfc.select(&atqa) {
                    Ok(uid) => writeln!(cx.local.stream1, "NFC: * {:?}", uid.as_bytes()).ok(),
                    Err(_) => writeln!(cx.local.stream1, "NFC: failed to read UID").ok(),
                },
                Err(_) => writeln!(cx.local.stream1, "NFC: empty IRQ").ok(),
            };

            cx.local.irq.clear_interrupt_pending_bit();
        } else {
            writeln!(cx.local.stream1, "NFC: unexpected IRQ").ok();
        }
    }

    #[task(binds = TIM3, priority = 1, local = [led, tim3, stream2])]
    fn tim3(cx: tim3::Context) {
        if cx.local.led.is_set_low() {
            cx.local.led.set_high();
        } else {
            cx.local.led.set_low();
        }
        writeln!(cx.local.stream2, "TIM3 blink").ok();
        cx.local.tim3.clear_interrupt(Event::Update);
    }
}
