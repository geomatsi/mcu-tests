#![no_main]
#![no_std]

use cortex_m::singleton;
use cortex_m_rt::entry;
use cortex_m_semihosting::hprintln;
use panic_semihosting as _;
use stm32f1xx_hal::{
    adc::{self, Continuous},
    gpio,
    pac,
    prelude::*,
    rcc,
};

type AdcDmaSingle = adc::AdcDma1<gpio::gpioa::PA0<gpio::Analog>, Continuous>;

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let mut flash = dp.FLASH.constrain();
    let mut rcc = dp
        .RCC
        .freeze(rcc::Config::hsi().adcclk(2.MHz()), &mut flash.acr);

    let dma_ch1 = dp.DMA1.split(&mut rcc).1;
    let adc1 = adc::Adc::new(dp.ADC1, &mut rcc);

    let mut gpioa = dp.GPIOA.split(&mut rcc);
    let adc_ch0 = gpioa.pa0.into_analog(&mut gpioa.crl);

    let mut adc_dma: AdcDmaSingle = adc1.with_dma(adc_ch0, dma_ch1);
    let mut buf = singleton!(: [u16; 8] = [0; 8]).unwrap();

    loop {
        let (next_buf, next_adc_dma) = adc_dma.read(buf).wait();
        let _ = hprintln!("{:?}", next_buf);
        buf = next_buf;
        adc_dma = next_adc_dma;
    }
}
