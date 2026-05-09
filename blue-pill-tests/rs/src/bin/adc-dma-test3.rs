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
    adc::{self, Continuous},
    dma::{self, Transfer, W},
    gpio,
    pac::{self, interrupt},
    prelude::*,
    rcc,
};

type RdmaT = adc::AdcDma1<gpio::gpioa::PA0<gpio::Analog>, Continuous>;
type RbufT = &'static mut [u16; 4];

static G_XFR: Mutex<RefCell<Option<Transfer<W, RbufT, RdmaT>>>> = Mutex::new(RefCell::new(None));
static G_DMA: Mutex<RefCell<Option<RdmaT>>> = Mutex::new(RefCell::new(None));
static G_BUF: Mutex<RefCell<Option<RbufT>>> = Mutex::new(RefCell::new(None));

#[entry]
fn main() -> ! {
    let cp = cm::Peripherals::take().unwrap();
    let dp = pac::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let mut rcc = dp
        .RCC
        .freeze(rcc::Config::hsi().adcclk(2.MHz()), &mut flash.acr);
    let mut nvic = cp.NVIC;

    let mut delay = cp.SYST.delay(&rcc.clocks);

    let mut dma_ch1 = dp.DMA1.split(&mut rcc).1;
    dma_ch1.listen(dma::Event::TransferComplete);

    let adc1 = adc::Adc::new(dp.ADC1, &mut rcc);
    let mut gpioa = dp.GPIOA.split(&mut rcc);
    let adc_ch0 = gpioa.pa0.into_analog(&mut gpioa.crl);

    let adc_dma = adc1.with_dma(adc_ch0, dma_ch1);
    let buf = singleton!(: [u16; 4] = [0; 4]).unwrap();

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

        delay.delay_ms(5_000_u16);
    }
}

#[interrupt]
fn DMA1_CHANNEL1() {
    cm::interrupt::free(|cs| {
        if let Some(xfer) = G_XFR.borrow(cs).replace(None) {
            let (buf, adc_dma) = xfer.wait();
            hprintln!("DMA1_CH1 IRQ: results: {:?}", buf);
            G_DMA.borrow(cs).replace(Some(adc_dma));
            G_BUF.borrow(cs).replace(Some(buf));
        } else {
            hprintln!("DMA1_CH1 IRQ: ERR: no xfer");
        }
    });
}
