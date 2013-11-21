#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "mcp23s17.h"


static const uint8_t spi_mode = 0;
static const uint8_t spi_bpw = 8; // bits per word
static const uint32_t spi_speed = 10000000; // 10MHz
static const uint16_t spi_delay = 0;
static const char * spidev[2][2] = {
    {"/dev/spidev0.0", "/dev/spidev0.1"},
    {"/dev/spidev1.0", "/dev/spidev1.1"},
};


// prototypes
static uint8_t get_spi_control_byte(uint8_t rw_cmd, uint8_t hw_addr);


int mcp23s17_open(int bus, int chip_select)
{
    int fd;
    // open
    if ((fd = open(spidev[bus][chip_select], O_RDWR)) < 0) {
        fprintf(stderr,
                "mcp23s17_open: ERROR Could not open SPI device (%s).",
                spidev[bus][chip_select]);
        return -1;
    }

    // initialise
    if (ioctl(fd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
        fprintf(stderr, "mcp23s17_open: ERROR Could not set SPI mode.");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw) < 0) {
        fprintf(stderr,
                "mcp23s17_open: ERROR Could not set SPI bits per word.");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
        fprintf(stderr, "mcp23s17_open: ERROR Could not set SPI speed.");
        return -1;
    }

    return fd;
}

uint8_t mcp23s17_read_reg(uint8_t reg, uint8_t hw_addr, int fd)
{
    uint8_t control_byte = get_spi_control_byte(READ_CMD, hw_addr);
    uint8_t tx_buf[3] = {control_byte, reg, 0};
    uint8_t rx_buf[sizeof tx_buf];

    struct spi_ioc_transfer spi;
    spi.tx_buf = (unsigned long) tx_buf;
    spi.rx_buf = (unsigned long) rx_buf;
    spi.len = sizeof tx_buf;
    spi.delay_usecs = spi_delay;
    spi.speed_hz = spi_speed;
    spi.bits_per_word = spi_bpw;

    // do the SPI transaction
    if ((ioctl(fd, SPI_IOC_MESSAGE(1), &spi) < 0)) {
        fprintf(stderr,
                "mcp23s17_read_reg: There was a error during the SPI "
                "transaction.");
        return -1;
    }

    // return the data
    return rx_buf[2];
}

void mcp23s17_write_reg(uint8_t data, uint8_t reg, uint8_t hw_addr, int fd)
{
    uint8_t control_byte = get_spi_control_byte(WRITE_CMD, hw_addr);
    uint8_t tx_buf[3] = {control_byte, reg, data};
    uint8_t rx_buf[sizeof tx_buf];

    struct spi_ioc_transfer spi;
    spi.tx_buf = (unsigned long) tx_buf;
    spi.rx_buf = (unsigned long) rx_buf;
    spi.len = sizeof tx_buf;
    spi.delay_usecs = spi_delay;
    spi.speed_hz = spi_speed;
    spi.bits_per_word = spi_bpw;

    // do the SPI transaction
    if ((ioctl(fd, SPI_IOC_MESSAGE(1), &spi) < 0)) {
        fprintf(stderr,
                "mcp23s17_write_reg: There was a error during the SPI "
                "transaction.");
    }
}

uint8_t mcp23s17_read_bit(uint8_t bit_num,
                          uint8_t reg,
                          uint8_t hw_addr,
                          int fd)
{
    return (mcp23s17_read_reg(reg, hw_addr, fd) >> bit_num) & 1;
}

void mcp23s17_write_bit(uint8_t data,
                        uint8_t bit_num,
                        uint8_t reg,
                        uint8_t hw_addr,
                        int fd)
{
    uint8_t reg_data = mcp23s17_read_reg(reg, hw_addr, fd);
    if (data) {
        reg_data |= 1 << bit_num; // set
    } else {
        reg_data &= 0xff ^ (1 << bit_num); // clear
    }
    return mcp23s17_write_reg(reg_data, reg, hw_addr, fd);
}


/**
 * Returns an SPI control byte.
 *
 * The MCP23S17 is a slave SPI device. The slave address contains four
 * fixed bits (0b0100) and three user-defined hardware address bits
 * (if enabled via IOCON.HAEN; pins A2, A1 and A0) with the
 * read/write command bit filling out the rest of the control byte::
 *
 *     +--------------------+
 *     |0|1|0|0|A2|A1|A0|R/W|
 *     +--------------------+
 *     |fixed  |hw_addr |R/W|
 *     +--------------------+
 *     |7|6|5|4|3 |2 |1 | 0 |
 *     +--------------------+
 *
 */
static uint8_t get_spi_control_byte(uint8_t rw_cmd, uint8_t hw_addr)
{
    hw_addr = (hw_addr << 1) & 0xE;
    rw_cmd &= 1; // just 1 bit long
    return 0x40 | hw_addr | rw_cmd;
}
