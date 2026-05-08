#![no_main]
#![no_std]

use core::cell::RefCell;
use cortex_m as cm;
use cortex_m::interrupt::Mutex;
use cortex_m::singleton;
use cortex_m_rt::entry;
use cortex_m_semihosting::hprintln;
use panic_semihosting as _;
use stm32f1xx_hal::{
    adc::{self, Adc, Scan, SetChannels},
    dma::{self, Transfer, W},
    gpio::{
        Analog,
        gpioa::{PA0, PA1, PA2, PA3},
    },
    pac::{self, interrupt},
    prelude::*,
    rcc,
};

type RdmaT = adc::AdcDma1<AdcPins, Scan>;
type RbufT = &'static mut [u16; 4];

static G_XFR: Mutex<RefCell<Option<Transfer<W, RbufT, RdmaT>>>> = Mutex::new(RefCell::new(None));
static G_DMA: Mutex<RefCell<Option<RdmaT>>> = Mutex::new(RefCell::new(None));
static G_BUF: Mutex<RefCell<Option<RbufT>>> = Mutex::new(RefCell::new(None));

struct AdcPins(PA0<Analog>, PA1<Analog>, PA2<Analog>, PA3<Analog>);

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

#[entry]
fn main() -> ! {
    let cp = cm::Peripherals::take().unwrap();
    let dp = pac::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let mut rcc = dp
        .RCC
        .freeze(rcc::Config::hsi().adcclk(1.MHz()), &mut flash.acr);
    let mut nvic = cp.NVIC;

    let mut delay = cp.SYST.delay(&rcc.clocks);

    let mut dma_ch1 = dp.DMA1.split(&mut rcc).1;
    dma_ch1.listen(dma::Event::TransferComplete);

    let adc1 = adc::Adc::new(dp.ADC1, &mut rcc);
    let mut gpioa = dp.GPIOA.split(&mut rcc);
    let adc_pins = AdcPins(
        gpioa.pa0.into_analog(&mut gpioa.crl),
        gpioa.pa1.into_analog(&mut gpioa.crl),
        gpioa.pa2.into_analog(&mut gpioa.crl),
        gpioa.pa3.into_analog(&mut gpioa.crl),
    );

    let buf = singleton!(: [u16; 4] = [0; 4]).unwrap();
    let adc_dma = adc1.with_scan_dma(adc_pins, dma_ch1);

    unsafe {
        nvic.set_priority(pac::Interrupt::DMA1_CHANNEL1, 1);
        cm::peripheral::NVIC::unmask(pac::Interrupt::DMA1_CHANNEL1);
    }

    cm::peripheral::NVIC::unpend(pac::Interrupt::DMA1_CHANNEL1);

    let xfer = adc_dma.read(buf);
    cm::interrupt::free(|cs| {
        G_XFR.borrow(cs).replace(Some(xfer));
    });

    loop {
        hprintln!("IDLE: wait 1 sec");
        delay.delay_ms(1_000_u16);

        cm::interrupt::free(|cs| {
            if let (Some(adc_dma), Some(buf)) = (
                G_DMA.borrow(cs).replace(None),
                G_BUF.borrow(cs).replace(None),
            ) {
                hprintln!("IDLE: start next xfer");
                let xfer = adc_dma.read(buf);
                G_XFR.borrow(cs).replace(Some(xfer));
            } else {
                hprintln!("IDLE: ERR: no rdma");
            }
        });
    }
}

#[interrupt]
fn DMA1_CHANNEL1() {
    cm::interrupt::free(|cs| {
        if let Some(xfer) = G_XFR.borrow(cs).replace(None) {
            let (buf, adc_dma) = xfer.wait();
            hprintln!("DMA1_CH1 IRQ: {:?}", buf);
            G_DMA.borrow(cs).replace(Some(adc_dma));
            G_BUF.borrow(cs).replace(Some(buf));
        } else {
            hprintln!("DMA1_CH1 IRQ: ERR: no xfer");
        }
    });
}
