#![deny(unsafe_code)]
#![no_main]
#![no_std]

use cortex_m::singleton;
use cortex_m_semihosting::hprintln;
use panic_semihosting as _;

#[rtic::app(device = stm32f1xx_hal::pac)]
mod app {
    use super::*;
    use stm32f1xx_hal::{
        adc::{self, Adc, Scan, SetChannels},
        dma::{self, Transfer, W},
        gpio::{
            Analog,
            gpioa::{PA0, PA1, PA2, PA3},
        },
        pac,
        prelude::*,
        rcc,
        timer::{CounterMs, Event},
    };

    type RdmaT = adc::AdcDma1<AdcPins, Scan>;
    type RbufT = &'static mut [u16; 4];

    pub struct AdcPins(PA0<Analog>, PA1<Analog>, PA2<Analog>, PA3<Analog>);

    impl SetChannels<AdcPins> for Adc<pac::ADC1> {
        fn set_samples(&mut self) {
            self.set_channel_sample_time(0, adc::SampleTime::T_28);
            self.set_channel_sample_time(1, adc::SampleTime::T_28);
            self.set_channel_sample_time(2, adc::SampleTime::T_28);
            self.set_channel_sample_time(3, adc::SampleTime::T_28);
        }

        fn set_sequence(&mut self) {
            self.set_regular_sequence(&[0, 1, 2, 3]);
        }
    }

    #[shared]
    struct Shared {
        xfr: Option<Transfer<W, RbufT, RdmaT>>,
        dma: Option<RdmaT>,
        buf: Option<RbufT>,
        tim2: CounterMs<pac::TIM2>,
    }

    #[local]
    struct Local {}

    #[init]
    fn init(cx: init::Context) -> (Shared, Local) {
        let mut flash = cx.device.FLASH.constrain();
        let mut rcc = cx
            .device
            .RCC
            .freeze(rcc::Config::hsi().adcclk(1.MHz()), &mut flash.acr);

        let mut dma_ch1 = cx.device.DMA1.split(&mut rcc).1;
        dma_ch1.listen(dma::Event::TransferComplete);

        let adc1 = adc::Adc::new(cx.device.ADC1, &mut rcc);
        let mut gpioa = cx.device.GPIOA.split(&mut rcc);
        let adc_pins = AdcPins(
            gpioa.pa0.into_analog(&mut gpioa.crl),
            gpioa.pa1.into_analog(&mut gpioa.crl),
            gpioa.pa2.into_analog(&mut gpioa.crl),
            gpioa.pa3.into_analog(&mut gpioa.crl),
        );

        let buffer = singleton!(: [u16; 4] = [0; 4]).unwrap();
        let adc_dma = adc1.with_scan_dma(adc_pins, dma_ch1);

        let mut tim2 = cx.device.TIM2.counter_ms(&mut rcc);
        tim2.start(1.secs()).unwrap();
        tim2.listen(Event::Update);

        (
            Shared {
                xfr: None,
                dma: Some(adc_dma),
                buf: Some(buffer),
                tim2,
            },
            Local {},
        )
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        loop {
            cortex_m::asm::wfi();
        }
    }

    #[task(binds = TIM2, priority = 1, shared = [xfr, dma, buf, tim2])]
    fn start_adc_dma(cx: start_adc_dma::Context) {
        let mut xfr = cx.shared.xfr;
        let mut dma = cx.shared.dma;
        let mut buf = cx.shared.buf;
        let mut tim2 = cx.shared.tim2;

        (&mut xfr, &mut dma, &mut buf, &mut tim2).lock(|xfr, dma, buf, tim2| {
            tim2.clear_interrupt(Event::Update);
            tim2.unlisten(Event::Update);

            if let (Some(adc_dma), Some(buffer)) = (dma.take(), buf.take()) {
                hprintln!("TASK: start next xfer");
                *xfr = Some(adc_dma.read(buffer));
            } else {
                hprintln!("TASK: ERR: no rdma");
            }
        });
    }

    #[task(binds = DMA1_CHANNEL1, priority = 2, shared = [xfr, dma, buf, tim2])]
    fn dma1_channel1(cx: dma1_channel1::Context) {
        let mut xfr = cx.shared.xfr;
        let mut dma = cx.shared.dma;
        let mut buf = cx.shared.buf;
        let mut tim2 = cx.shared.tim2;

        (&mut xfr, &mut dma, &mut buf, &mut tim2).lock(|xfr, dma, buf, tim2| {
            if let Some(transfer) = xfr.take() {
                let (buffer, adc_dma) = transfer.wait();
                hprintln!("DMA1_CH1 IRQ: {:?}", buffer);
                *dma = Some(adc_dma);
                *buf = Some(buffer);
            } else {
                hprintln!("DMA1_CH1 IRQ: ERR: no xfer");
            }

            tim2.start(1.secs()).unwrap();
            tim2.listen(Event::Update);
        });
    }
}
