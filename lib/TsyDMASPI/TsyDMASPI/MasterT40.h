#ifndef ARDUINO_TEENSY_DMA_SPI_MASTER_T40_H
#define ARDUINO_TEENSY_DMA_SPI_MASTER_T40_H
#if defined(__IMXRT1062__)

#include "MasterBase.h"

inline void tsydmaspi_rxisr_0();

ARDUINO_TEENSY_DMA_SPI_NAMESPACE_BEGIN


struct Master0 : public MasterBase
{
    friend void tsydmaspi_rxisr_0();

    virtual ~Master0()
    {
        destroy();
    }

    bool begin(const bool active_low = true)
    {
        return this->MasterBase::begin(SPI, SPISettings(), active_low);
    }

    bool begin(const SPISettings& setting, const bool active_low = true)
    {
        return this->MasterBase::begin(SPI, setting, active_low);
    }

    virtual DMAChannel* dmarx() override final
    {
        static DMAChannel *rx = new DMAChannel();
        return rx;
    }

    virtual DMAChannel* dmatx() override final
    {
        static DMAChannel *tx = new DMAChannel();
        return tx;
    }

    virtual bool initDmaTx() override final
    {
        if (dmatx() == nullptr)
        {
            Serial.println("[ERROR] create dma tx buffer failed");
            return false;
        }
        dmatx()->disable();
        dmatx()->destination((volatile uint8_t &)IMXRT_LPSPI4_S.TDR);
        dmatx()->disableOnCompletion();
        dmatx()->triggerAtHardwareEvent(DMAMUX_SOURCE_LPSPI4_TX);
        if (dmatx()->error())
        {
            delete dmatx();
            return false;
        }
        return true;
    }

    virtual bool initDmaRx() override final
    {
        if (dmarx() == nullptr)
        {
            Serial.println("[ERROR] create dma tx buffer failed");
            return false;
        }
        dmarx()->disable();
        dmarx()->source((volatile uint8_t &)IMXRT_LPSPI4_S.RDR);
        dmarx()->disableOnCompletion();
        dmarx()->triggerAtHardwareEvent(DMAMUX_SOURCE_LPSPI4_RX);
        dmarx()->attachInterrupt(tsydmaspi_rxisr_0);
        dmarx()->interruptAtCompletion();
        if (dmarx()->error())
        {
            delete dmarx();
            return false;
        }
        return true;
    }

    virtual void destroy() override final
    {
        if (dmarx() != nullptr) delete dmarx();
        if (dmatx() != nullptr) delete dmatx();
    }

    virtual void initTransaction() override final
    {
        IMXRT_LPSPI4_S.TCR = (IMXRT_LPSPI4_S.TCR & ~(LPSPI_TCR_FRAMESZ(31))) | LPSPI_TCR_FRAMESZ(7);
        IMXRT_LPSPI4_S.FCR = 0;

        // Lets try to output the first byte to make sure that we are in 8 bit mode...
        IMXRT_LPSPI4_S.DER = LPSPI_DER_TDDE | LPSPI_DER_RDDE; //enable DMA on both TX and RX
        IMXRT_LPSPI4_S.SR = 0x3f00;                           // clear out all of the other status...
    }

    virtual void clearTransaction() override final
    {
        IMXRT_LPSPI4_S.FCR = LPSPI_FCR_TXWATER(15); // _spi_fcr_save; // restore the FSR status...
        IMXRT_LPSPI4_S.DER = 0;                     // DMA no longer doing TX (or RX)

        IMXRT_LPSPI4_S.CR = LPSPI_CR_MEN | LPSPI_CR_RRF | LPSPI_CR_RTF; // actually clear both...
        IMXRT_LPSPI4_S.SR = 0x3f00;                                     // clear out all of the other status...
    }
};

ARDUINO_TEENSY_DMA_SPI_NAMESPACE_END

static TsyDMASPI::Master0 TsyDMASPI0;

inline void tsydmaspi_rxisr_0()
{
    TsyDMASPI0.dmarx()->clearInterrupt();
    TsyDMASPI0.next();
}

#endif // defined(__IMXRT1062__)
#endif // ARDUINO_TEENSY_DMA_SPI_MASTER_T40_H
