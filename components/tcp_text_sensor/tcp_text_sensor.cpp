#include "esphome/core/log.h"
#include "tcp_text_sensor.h"




#include <memory>
#include <string>
#include <vector>

namespace esphome {
namespace tcp_text_sensor {
using namespace esphome;

static const char *TAG = "tcp_text_sensor";

void TCPTextSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up stream server...");

    struct sockaddr_in bind_addr = {
        .sin_len = sizeof(struct sockaddr_in),
        .sin_family = AF_INET,
        .sin_port = htons(this->port_),
        .sin_addr = {
            .s_addr = ESPHOME_INADDR_ANY,
        }
    };

    this->socket_ = socket::socket(AF_INET, SOCK_STREAM, PF_INET);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 20000; // ESPHome recommends 20-30 ms max for timeouts

    this->socket_->setsockopt(SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    this->socket_->setsockopt(SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    this->socket_->bind(reinterpret_cast<struct sockaddr *>(&bind_addr), sizeof(struct sockaddr_in));
    this->socket_->listen(8);

}

void TCPTextSensor::loop() {
    this->accept();
    this->read();
    this->cleanup();
}
void TCPTextSensor::accept() {
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(struct sockaddr_in);
    std::unique_ptr<socket::Socket> socket = this->socket_->accept(reinterpret_cast<struct sockaddr *>(&client_addr), &client_addrlen);
    if (!socket)
        return;

    socket->setblocking(false);
    std::string identifier = socket->getpeername();
    this->clients_.emplace_back(std::move(socket), identifier);
    ESP_LOGD(TAG, "New client connected from %s", identifier.c_str());
}

void TCPTextSensor::cleanup() {
    auto discriminator = [](const Client &client) { return !client.disconnected; };
    auto last_client = std::partition(this->clients_.begin(), this->clients_.end(), discriminator);
    this->clients_.erase(last_client, this->clients_.end());
}

void TCPTextSensor::read() {
    uint8_t buf[128];
    ssize_t len;
    for (Client &client : this->clients_) {
        while ((len = client.socket->read(&buf, sizeof(buf))) > 0){
            ESP_LOGD(TAG, "Read %d bytes from client %s", len, client.identifier.c_str());
            ESP_LOGD(TAG, "Content: %s", buf);
            std::string data_str(reinterpret_cast<const char*>(buf), len);
            this->publish_state(data_str);
		}
        if (len == 0) {
            ESP_LOGD(TAG, "Client %s disconnected", client.identifier.c_str());
            client.disconnected = true;
            continue;
        }
    }
}
void TCPTextSensor::send(const std::string &data) {
//    sent the data to each client
    if (data.size() > 0) {
        ESP_LOGD(TAG, "Sending %s", data.c_str());
        for (Client &client : this->clients_) {
            client.socket->write(data.c_str(), data.size());
        }
    }
}

void TCPTextSensor::send(const uint8_t *data, size_t len) {
//    sent the data to each client
//    convert the data to a string
    if (len > 0) {
        std::string data_str(reinterpret_cast<const char*>(data), len);
        ESP_LOGD(TAG, "Sending %s", data_str.c_str());
        for (Client &client : this->clients_) {
            client.socket->write(data, len);
        }
    }
}
void TCPTextSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "Stream Server:");
    ESP_LOGCONFIG(TAG, "  Address: %s:%u",
                  esphome::network::get_ip_address().str().c_str(),
                  this->port_);
}

void TCPTextSensor::on_shutdown() {
    for (const Client &client : this->clients_)
        client.socket->shutdown(SHUT_RDWR);
}
TCPTextSensor::Client::Client(std::unique_ptr<esphome::socket::Socket> socket, std::string identifier)
    : socket(std::move(socket)), identifier{identifier}
{
}
}  // namespace empty_text_sensor
}  // namespace esphome