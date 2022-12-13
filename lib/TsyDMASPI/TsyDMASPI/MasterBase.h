#ifndef ARDUINO_TEENSY_DMA_SPI_MASTER_BASE_H
#define ARDUINO_TEENSY_DMA_SPI_MASTER_BASE_H

#include <Arduino.h>
#include <util/atomic.h>
#include <SPI.h>
#include <DMAChannel.h>
#include <deque>
#include "Constants.h"

ARDUINO_TEENSY_DMA_SPI_NAMESPACE_BEGIN

class MasterBase
{
protected:

    struct spi_transaction_t
    {
        uint16_t size;
        const uint8_t* tx_buffer;
        volatile uint8_t* rx_buffer;
        uint8_t cs_pin;
    };

    class SimpleQueue
    {
        spi_transaction_t data[16];
        int frontIdx = 0;
        int backIdx = 0;
        int length = 0;
    public:
        inline void emplace_back(spi_transaction_t& item)
        {
            if (length >= 16)
            {
                Serial.println("BUFFER QUEUE OVERFLOW");
                return;
            }

            data[backIdx] = item;
            backIdx = (backIdx+1) & 0x0F; // mod 16
            length++;
        }

        inline spi_transaction_t& front()
        {
            return data[frontIdx];
        }

        inline spi_transaction_t pop_front()
        {
            if (length == 0)
                return spi_transaction_t();

            spi_transaction_t output = data[frontIdx];
            frontIdx = (frontIdx + 1) & 0x0F; // mod 16
            length--;
            return output;
        }

        inline int size() { return length; }
        inline bool empty() { return length == 0; }

    };

    SPIClass* spi;
    SPISettings spi_setting;
    bool b_active_low {true};
    uint8_t current_cs_pin;

    //std::deque<spi_transaction_t> transactions;
    SimpleQueue transactions;
    bool b_in_transaction {false};
    volatile uint8_t dummy {0};

public:

    // Set your possible chip select pins here, up to 4 are supported
    // Any pins defined here will be set to inactive at the start of a transaction
    uint8_t csPins[4] = {255, 255, 255, 255};

    virtual ~MasterBase() {}

    bool begin(SPIClass& spic, const SPISettings& setting, const bool active_low)
    {
        spi = &spic;
        b_active_low = active_low;
        spi_setting = setting;
        spi->begin();
        if (!initDmaTx()) return false;
        if (!initDmaRx()) return false;
        return true;
    }

    void end()
    {
        spi->end();
        destroy();
    }

    // each derived class should have its own dma buffer
    virtual DMAChannel* dmarx() = 0;
    virtual DMAChannel* dmatx() = 0;

    bool transfer(const uint8_t* tx_buf, volatile uint8_t* rx_buf, const size_t size, const uint8_t cs_pin)
    {
        if (queue(tx_buf, rx_buf, size, cs_pin))
            yield();
        else
            return false;

        return true;
    }

    bool queue(const uint8_t* tx_buf, volatile uint8_t* rx_buf, const size_t size, const uint8_t cs_pin)
    {
        if ((size == 0) || (size > 0x7FFF))
        {
            Serial.printf("[ERROR] transaction size is invalid : %d\n", size);
            return false;
        }

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            spi_transaction_t t;
            t.tx_buffer = tx_buf;
            t.rx_buffer = rx_buf;
            t.size = size;
            t.cs_pin = cs_pin;
            transactions.emplace_back(t);
            beginTransaction();
        }

        return true;
    }

    void yield()
    {
        while(1)
        {
            bool b = false;
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                b = transactions.empty();
            }
            if (b) break;
        }
    }

    size_t remained()
    {
        size_t s;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            s = transactions.size();
        }
        return s;
    }

    void next()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            endTransaction();
            beginTransaction();
        }
    }

    // get SPIClass pointer to configure SPI directly
    SPIClass* spiclass() const
    {
        return spi;
    }

protected:

    void beginTransaction()
    {
        if (transactions.empty() || b_in_transaction) return;

        b_in_transaction = true;
        spi_transaction_t& trans = transactions.front();

        if (trans.rx_buffer)
            dmarx()->destinationBuffer(trans.rx_buffer, trans.size);
        else
        {
            dmarx()->destination(dummy);
            dmarx()->transferCount(trans.size);
        }

        if (trans.tx_buffer)
            dmatx()->sourceBuffer(trans.tx_buffer, trans.size);
        else
        {
            dmatx()->source(dummy);
            dmatx()->transferCount(trans.size);
        }

        initTransaction();

        spi->beginTransaction(spi_setting);
        current_cs_pin = trans.cs_pin;

        // Set all CS pins to not active
        if (csPins[0] != 255) digitalWriteFast(csPins[0], b_active_low);
        if (csPins[1] != 255) digitalWriteFast(csPins[1], b_active_low);
        if (csPins[2] != 255) digitalWriteFast(csPins[2], b_active_low);
        if (csPins[3] != 255) digitalWriteFast(csPins[3], b_active_low);
        // set the current cs pin to active
        digitalWriteFast(current_cs_pin, !b_active_low);
        
        dmarx()->enable();
        dmatx()->enable();
    }

    void endTransaction()
    {
        digitalWriteFast(current_cs_pin, b_active_low);

        current_cs_pin = 255;
        spi->endTransaction();
        transactions.pop_front();
        b_in_transaction = false;
        clearTransaction();
    }

    virtual bool initDmaTx() = 0;
    virtual bool initDmaRx() = 0;
    virtual void destroy() = 0;
    virtual void initTransaction() = 0;
    virtual void clearTransaction() = 0;
};


ARDUINO_TEENSY_DMA_SPI_NAMESPACE_END

namespace TsyDMASPI = arduino::teensy::spi::dma;


#endif // ARDUINO_TEENSY_DMA_SPI_MASTER_BASE_H
