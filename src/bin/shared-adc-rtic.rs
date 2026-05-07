#![deny(unsafe_code)]
#![no_main]
#![no_std]

use panic_rtt_target as _;

#[rtic::app(device = stm32f1xx_hal::pac)]
mod app {
    use rtt_target::{rprintln, rtt_init_print};
    use shared_bus::{AdcProxy, CortexMMutex};
    use stm32f1xx_hal::{
        adc::Adc,
        gpio::{
            gpioa::{PA0, PA1},
            Analog,
        },
        pac::{self, ADC1},
        prelude::*,
        rcc,
        timer::{CounterMs, Event},
    };

    #[shared]
    struct Shared {}

    #[local]
    struct Local {
        adc_proxy1: AdcProxy<'static, CortexMMutex<Adc<ADC1>>>,
        adc_proxy2: AdcProxy<'static, CortexMMutex<Adc<ADC1>>>,
        adc_ch1: PA0<Analog>,
        adc_ch2: PA1<Analog>,
        tim2: CounterMs<pac::TIM2>,
        tim3: CounterMs<pac::TIM3>,
    }

    #[init]
    fn init(cx: init::Context) -> (Shared, Local) {
        rtt_init_print!();

        let mut flash = cx.device.FLASH.constrain();
        let mut rcc = cx.device.RCC.freeze(
            rcc::Config::hse(8.MHz())
                .sysclk(32.MHz())
                .pclk1(16.MHz())
                .adcclk(8.MHz()),
            &mut flash.acr,
        );

        let adc = Adc::new(cx.device.ADC1, &mut rcc);
        let adc_bus: &'static _ = shared_bus::new_cortexm!(Adc<ADC1> = adc).unwrap();
        let adc_proxy1 = adc_bus.acquire_adc();
        let adc_proxy2 = adc_bus.acquire_adc();

        let mut gpioa = cx.device.GPIOA.split(&mut rcc);
        let adc_ch1 = gpioa.pa0.into_analog(&mut gpioa.crl);
        let adc_ch2 = gpioa.pa1.into_analog(&mut gpioa.crl);

        let mut tim2 = cx.device.TIM2.counter_ms(&mut rcc);
        tim2.start(1.secs()).unwrap();
        tim2.listen(Event::Update);

        let mut tim3 = cx.device.TIM3.counter_ms(&mut rcc);
        tim3.start(500.millis()).unwrap();
        tim3.listen(Event::Update);

        (
            Shared {},
            Local {
                adc_proxy1,
                adc_proxy2,
                adc_ch1,
                adc_ch2,
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

    #[task(binds = TIM2, priority = 1, local = [adc_proxy1, adc_ch1, tim2])]
    fn task1(cx: task1::Context) {
        let val: u16 = cx.local.adc_proxy1.read(cx.local.adc_ch1).unwrap();
        rprintln!("reading1: {}", val);
        cx.local.tim2.clear_interrupt(Event::Update);
    }

    #[task(binds = TIM3, priority = 1, local = [adc_proxy2, adc_ch2, tim3])]
    fn task2(cx: task2::Context) {
        let val: u16 = cx.local.adc_proxy2.read(cx.local.adc_ch2).unwrap();
        rprintln!("reading2: {}", val);
        cx.local.tim3.clear_interrupt(Event::Update);
    }
}
