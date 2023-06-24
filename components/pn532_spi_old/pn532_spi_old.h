#pragma once

#include "esphome/core/component.h"
#include "esphome/components/pn532_old/pn532_old.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace pn532_spi_old {

class PN532Spi : public pn532_old::PN532,
                 public spi::SPIDevice<spi::BIT_ORDER_LSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING,
                                       spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;

  void dump_config() override;

 protected:
  bool write_data(const std::vector<uint8_t> &data) override;
  bool read_data(std::vector<uint8_t> &data, uint8_t len) override;
  bool read_response(uint8_t command, std::vector<uint8_t> &data) override;
};

}  // namespace pn532_spi_old
}  // namespace esphome
