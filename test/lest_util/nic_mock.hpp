// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2017 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <hw/nic.hpp>

#ifndef NICK_MOCK_HPP
#define NICK_MOCK_HPP

#define NIC_INFO(X,...) INFO("Mock NIC", X, ##__VA_ARGS__)

class Nic_mock : public hw::Nic {

public:
  Proto proto() const override
  { return Nic::Proto::ETH; }

  const char* driver_name() const override
  { return "Mock driver"; }

  std::string device_name() const override
  { return "Mock device name"; }

  /** The mac address. */
  const MAC::Addr& mac() const noexcept override
  { return mac_; }

  virtual uint16_t MTU() const noexcept override
  { return 1500; }


  downstream create_link_downstream() override
  { return transmit_to_link_; };

  void set_ip4_upstream(upstream handler) override
  { ip4_handler_ = handler; }

  void set_ip6_upstream(upstream handler) override
  { ip6_handler_ = handler; }

  void set_arp_upstream(upstream handler) override
  { arp_handler_ = handler; }

  /** Number of bytes in a frame needed by the device itself **/
  size_t frame_offset_device() override
  { return frame_offs_dev_; }

  /** Number of bytes in a frame needed by the link layer **/
  size_t frame_offset_link() override
  { return frame_offs_link_; }

  static constexpr uint16_t packet_len()
  { return 1514; }

  net::Packet_ptr create_packet(int) override
  {
    auto buffer = bufstore().get_buffer();
    auto* ptr = (net::Packet*) buffer.addr;

    new (ptr) net::Packet(frame_offs_dev_ + frame_offs_link_,
                          0,
                          frame_offs_dev_ + packet_len(),
                          buffer.bufstore);

    return net::Packet_ptr(ptr);
  }


  size_t transmit_queue_available() override
  { return 1024; }

  void deactivate() override
  { link_up_ = false; }

  /** Stats getters **/
  uint64_t get_packets_rx() override
  { return packets_rx_; }

  uint64_t get_packets_tx() override
  { return packets_tx_; }

  uint64_t get_packets_dropped() override
  { return packets_dropped_; }

  void move_to_this_cpu() override {}

  //
  // MOCK Extensions
  //

  ~Nic_mock() {}
  Nic_mock() : Nic(256, 2048) {}

  // Public data members (ahem)
  MAC::Addr mac_ = {0xc0,0x00,0x01,0x70,0x00,0x01};
  static constexpr size_t frame_offs_dev_ = 0;
  static constexpr size_t frame_offs_link_ = 14;

  std::vector<net::Packet_ptr> tx_queue_;

  void transmit(net::Packet_ptr, MAC::Addr, net::Ethertype)
  {
    NIC_INFO("transimtting packet");
    //tx_queue_.emplace_back(std::move(ptr));
  }

  void receive(net::Packet_ptr ptr){

    if(ip4_handler_) {
      NIC_INFO("pushing packet to IP4");
      ip4_handler_(std::move(ptr));
    } else {
      NIC_INFO("nowhere to push packet. Drop.");
    }
  }

  void flush() override {}
  void poll() override {}

private:

  upstream ip4_handler_ = nullptr;
  upstream ip6_handler_ = nullptr;
  upstream arp_handler_ = nullptr;
  downstream transmit_to_link_ = downstream{this, &Nic_mock::transmit};
  bool link_up_ = true;
  uint64_t packets_rx_ = 0;
  uint64_t packets_tx_ = 0;
  uint64_t packets_dropped_ = 0;


};


#endif
