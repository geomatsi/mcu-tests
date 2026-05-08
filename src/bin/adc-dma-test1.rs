#![no_main]
#![no_std]

use cortex_m::singleton;
use cortex_m_rt::entry;
use cortex_m_semihosting::hprintln;
use panic_semihosting as _;
use stm32f1xx_hal::{
    adc::{self, Continuous},
    gpio, pac,
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

    let adc_dma = adc1.with_dma(adc_ch0, dma_ch1);
    let buf = singleton!(: [u16; 8] = [0; 8]).unwrap();

    read_loop(buf, adc_dma);
}

#[allow(unconditional_recursion)]
fn read_loop(buf: &'static mut [u16; 8], adc_dma: AdcDmaSingle) -> ! {
    let (buf, adc_dma) = adc_dma.read(buf).wait();
    let _ = hprintln!("{:?}", buf);
    read_loop(buf, adc_dma);
}
