#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/socket/socket.h"

#ifdef ARDUINO_ARCH_ESP8266
#include <ESPAsyncTCP.h>
#else
// AsyncTCP.h includes parts of freertos, which require FreeRTOS.h header to be included first
#include <freertos/FreeRTOS.h>
#include <AsyncTCP.h>
#endif



namespace esphome {
namespace tcp_text_sensor {

class TCPTextSensor : public text_sensor::TextSensor, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void on_shutdown() override;

  float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

  void set_port(uint16_t port) { this->port_ = port; }
  int get_client_count() { return this->clients_.size(); }
  void send(const std::string &data);
  void send(const uint8_t *data, size_t len);
  void send(const std::vector<uint8_t> &data) { this->send(data.data(), data.size()); }
  std::string state{};
 protected:
  void accept();
  void read();
  void cleanup();

  struct Client {
        Client(std::unique_ptr<esphome::socket::Socket> socket, std::string identifier);

        std::unique_ptr<esphome::socket::Socket> socket{nullptr};
        std::string identifier{};
        bool disconnected{false};
    };
  std::unique_ptr<esphome::socket::Socket> socket_{};
  uint16_t port_{6638};
  std::vector<Client> clients_{};
};

}  // namespace empty_text_sensor
}  // namespace esphome
