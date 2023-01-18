/* Copyright (C) 2020-2022 Oxan van Leeuwen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stream_server.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/util.h"

#include "esphome/components/network/util.h"
#include "esphome/components/socket/socket.h"

static const char *TAG = "streamserver";

using namespace esphome;

void StreamServerComponent::setup() {
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

void StreamServerComponent::loop() {
    this->accept();
    this->read();
    this->write();
    this->cleanup();
}

void StreamServerComponent::accept() {
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

void StreamServerComponent::cleanup() {
    auto discriminator = [](const Client &client) { return !client.disconnected; };
    auto last_client = std::partition(this->clients_.begin(), this->clients_.end(), discriminator);
    this->clients_.erase(last_client, this->clients_.end());
}

void StreamServerComponent::read() {
//    int len;
//    while ((len = this->stream_->available()) > 0) {
//        char buf[128];
//        len = std::min(len, 128);
//        this->stream_->read_array(reinterpret_cast<uint8_t*>(buf), len);
//        ESP_LOGD(TAG, "Read %d bytes", len);
//        for (int i = 0; i < len; i++) {
//            ESP_LOGD(TAG, "%02x", buf[i]);
//        }
//        for (const Client &client : this->clients_)
//            client.socket->write(buf, len);
//    }
}

void StreamServerComponent::send(const std::string &data) {
//    sent the data to each client
    if (data.size() > 0) {
        ESP_LOGD(TAG, "Sending %s", data.c_str());
        for (Client &client : this->clients_) {
            client.socket->write(data.c_str(), data.size());
        }
    }
}

void StreamServerComponent::send(const uint8_t *data, size_t len) {
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
// git config --local user.email "

void StreamServerComponent::write() {
    uint8_t buf[128];
    ssize_t len;
    for (Client &client : this->clients_) {
        while ((len = client.socket->read(&buf, sizeof(buf))) > 0){
            ESP_LOGD(TAG, "Read %d bytes from client %s", len, client.identifier.c_str());
            ESP_LOGD(TAG, "Content: %s", buf);
            std::string data_str(reinterpret_cast<const char*>(buf), len);
            this->state = data_str;
		}
        if (len == 0) {
            ESP_LOGD(TAG, "Client %s disconnected", client.identifier.c_str());
            client.disconnected = true;
            continue;
        }
    }
}

void StreamServerComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "Stream Server:");
    ESP_LOGCONFIG(TAG, "  Address: %s:%u",
                  esphome::network::get_ip_address().str().c_str(),
                  this->port_);
}

void StreamServerComponent::on_shutdown() {
    for (const Client &client : this->clients_)
        client.socket->shutdown(SHUT_RDWR);
}

StreamServerComponent::Client::Client(std::unique_ptr<esphome::socket::Socket> socket, std::string identifier)
    : socket(std::move(socket)), identifier{identifier}
{
}
