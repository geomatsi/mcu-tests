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
            gpioa::{PA0, PA1, PA2, PA3, PA4},
            Analog,
        },
        pac,
        prelude::*,
        rcc,
        timer::{CounterMs, Event},
    };

    type RdmaType1 = adc::AdcDma1<AdcPinsOne, Scan>;
    type RdmaType2 = adc::AdcDma1<AdcPinsTwo, Scan>;
    type RbufType1 = &'static mut [u16; 2];
    type RbufType2 = &'static mut [u16; 3];

    pub struct AdcPinsOne(PA0<Analog>, PA1<Analog>);
    pub struct AdcPinsTwo(PA2<Analog>, PA3<Analog>, PA4<Analog>);

    impl SetChannels<AdcPinsOne> for Adc<pac::ADC1> {
        fn set_samples(&mut self) {
            self.set_channel_sample_time(0, adc::SampleTime::T_28);
            self.set_channel_sample_time(1, adc::SampleTime::T_28);
        }

        fn set_sequence(&mut self) {
            self.set_regular_sequence(&[0, 1]);
        }
    }

    impl SetChannels<AdcPinsTwo> for Adc<pac::ADC1> {
        fn set_samples(&mut self) {
            self.set_channel_sample_time(2, adc::SampleTime::T_28);
            self.set_channel_sample_time(3, adc::SampleTime::T_28);
            self.set_channel_sample_time(4, adc::SampleTime::T_28);
        }

        fn set_sequence(&mut self) {
            self.set_regular_sequence(&[2, 3, 4]);
        }
    }

    pub enum State {
        One,
        Two,
    }

    #[shared]
    struct Shared {
        state: State,
        transfer1: Option<Transfer<W, RbufType1, RdmaType1>>,
        transfer2: Option<Transfer<W, RbufType2, RdmaType2>>,
        adc_pins1: Option<AdcPinsOne>,
        adc_pins2: Option<AdcPinsTwo>,
        adc_dma1: Option<RdmaType1>,
        adc_dma2: Option<RdmaType2>,
        buffer1: Option<RbufType1>,
        buffer2: Option<RbufType2>,
        tim2: CounterMs<pac::TIM2>,
    }

    #[local]
    struct Local {}

    #[init]
    fn init(cx: init::Context) -> (Shared, Local) {
        let mut flash = cx.device.FLASH.constrain();
        let mut rcc = cx.device.RCC.freeze(rcc::Config::hsi().adcclk(1.MHz()), &mut flash.acr);

        let mut dma_ch1 = cx.device.DMA1.split(&mut rcc).1;
        dma_ch1.listen(dma::Event::TransferComplete);

        let adc1 = adc::Adc::new(cx.device.ADC1, &mut rcc);
        let mut gpioa = cx.device.GPIOA.split(&mut rcc);
        let adc_pins1 = AdcPinsOne(
            gpioa.pa0.into_analog(&mut gpioa.crl),
            gpioa.pa1.into_analog(&mut gpioa.crl),
        );
        let adc_pins2 = AdcPinsTwo(
            gpioa.pa2.into_analog(&mut gpioa.crl),
            gpioa.pa3.into_analog(&mut gpioa.crl),
            gpioa.pa4.into_analog(&mut gpioa.crl),
        );

        let buffer1 = singleton!(: [u16; 2] = [0; 2]).unwrap();
        let buffer2 = singleton!(: [u16; 3] = [0; 3]).unwrap();
        let adc_dma1 = adc1.with_scan_dma(adc_pins1, dma_ch1);

        let mut tim2 = cx.device.TIM2.counter_ms(&mut rcc);
        tim2.start(1.secs()).unwrap();
        tim2.listen(Event::Update);

        (
            Shared {
                state: State::One,
                transfer1: None,
                transfer2: None,
                adc_pins1: None,
                adc_pins2: Some(adc_pins2),
                adc_dma1: Some(adc_dma1),
                adc_dma2: None,
                buffer1: Some(buffer1),
                buffer2: Some(buffer2),
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

    #[task(
        binds = TIM2,
        priority = 1,
        shared = [state, transfer1, adc_dma1, buffer1, transfer2, adc_dma2, buffer2, tim2]
    )]
    fn start_adc_dma(cx: start_adc_dma::Context) {
        let mut state = cx.shared.state;
        let mut transfer1 = cx.shared.transfer1;
        let mut adc_dma1 = cx.shared.adc_dma1;
        let mut buffer1 = cx.shared.buffer1;
        let mut transfer2 = cx.shared.transfer2;
        let mut adc_dma2 = cx.shared.adc_dma2;
        let mut buffer2 = cx.shared.buffer2;
        let mut tim2 = cx.shared.tim2;

        (
            &mut state,
            &mut transfer1,
            &mut adc_dma1,
            &mut buffer1,
            &mut transfer2,
            &mut adc_dma2,
            &mut buffer2,
            &mut tim2,
        )
            .lock(|state, transfer1, adc_dma1, buffer1, transfer2, adc_dma2, buffer2, tim2| {
                tim2.clear_interrupt(Event::Update);
                tim2.unlisten(Event::Update);

                match state {
                    State::One => {
                        if let (Some(adc_dma), Some(buffer)) = (adc_dma1.take(), buffer1.take()) {
                            hprintln!("TASK: start next xfer");
                            *transfer1 = Some(adc_dma.read(buffer));
                        } else {
                            hprintln!("TASK: ERR: no ADC/DMA type One");
                        }
                    }
                    State::Two => {
                        if let (Some(adc_dma), Some(buffer)) = (adc_dma2.take(), buffer2.take()) {
                            hprintln!("TASK: start next xfer");
                            *transfer2 = Some(adc_dma.read(buffer));
                        } else {
                            hprintln!("TASK: ERR: no ADC/DMA type Two");
                        }
                    }
                }
            });
    }

    #[task(
        binds = DMA1_CHANNEL1,
        priority = 2,
        shared = [
            state,
            transfer1,
            transfer2,
            adc_pins1,
            adc_pins2,
            adc_dma1,
            adc_dma2,
            buffer1,
            buffer2,
            tim2
        ]
    )]
    fn dma1_channel1(cx: dma1_channel1::Context) {
        let mut state = cx.shared.state;
        let mut transfer1 = cx.shared.transfer1;
        let mut transfer2 = cx.shared.transfer2;
        let mut adc_pins1 = cx.shared.adc_pins1;
        let mut adc_pins2 = cx.shared.adc_pins2;
        let mut adc_dma1 = cx.shared.adc_dma1;
        let mut adc_dma2 = cx.shared.adc_dma2;
        let mut buffer1 = cx.shared.buffer1;
        let mut buffer2 = cx.shared.buffer2;
        let mut tim2 = cx.shared.tim2;

        (
            &mut state,
            &mut transfer1,
            &mut transfer2,
            &mut adc_pins1,
            &mut adc_pins2,
            &mut adc_dma1,
            &mut adc_dma2,
            &mut buffer1,
            &mut buffer2,
            &mut tim2,
        )
            .lock(
                |state,
                 transfer1,
                 transfer2,
                 adc_pins1,
                 adc_pins2,
                 adc_dma1,
                 adc_dma2,
                 buffer1,
                 buffer2,
                 tim2| {
                    match state {
                        State::One => {
                            if let (Some(transfer), Some(pins2)) = (transfer1.take(), adc_pins2.take()) {
                                let (buf1, adc_dma) = transfer.wait();
                                let (adc, pins1, chan) = adc_dma.split();

                                hprintln!("DMA1_CH1 IRQ: ONE: {:?}", buf1);

                                *adc_dma2 = Some(adc.with_scan_dma(pins2, chan));
                                *adc_pins1 = Some(pins1);
                                *buffer1 = Some(buf1);
                                *state = State::Two;
                            } else {
                                hprintln!("DMA1_CH1 IRQ: ERR: no transfer of type One");
                            }
                        }
                        State::Two => {
                            if let (Some(transfer), Some(pins1)) = (transfer2.take(), adc_pins1.take()) {
                                let (buf2, adc_dma) = transfer.wait();
                                let (adc, pins2, chan) = adc_dma.split();

                                hprintln!("DMA1_CH1 IRQ: TWO: {:?}", buf2);

                                *adc_dma1 = Some(adc.with_scan_dma(pins1, chan));
                                *adc_pins2 = Some(pins2);
                                *buffer2 = Some(buf2);
                                *state = State::One;
                            } else {
                                hprintln!("DMA1_CH1 IRQ: ERR: no transfer of type Two");
                            }
                        }
                    }

                    tim2.start(1.secs()).unwrap();
                    tim2.listen(Event::Update);
                },
            );
    }
}
