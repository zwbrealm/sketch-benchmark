from scapy.all import *
import struct
import time
import socket
# 目标主机的MAC地址
target_mac = "d0:94:66:6b:39:92"
# target_mac1 = "d0:9f:d9:70:57:51"

while True:
# 构造数据包
    src_mac = "d0:94:66:6a:ea:44"
    dst_mac = target_mac
    ethertype = 0x0800 # IPv4
    src_ip = "127.0.0.1"
    dst_ip = "192.168.123.199"
    src_port = 1234
    dst_port = 8888
    proto = 17 # UDP
    data = b"Hello, world!"
    payload = struct.pack("!4s4sHHH", socket.inet_aton(src_ip), socket.inet_aton(dst_ip), src_port, dst_port, proto) + data
    pkt = Ether(dst=dst_mac, src=src_mac, type=ethertype)/payload
#    pkt1 = Ether(dst=dst_mac1, src=src_mac, type=ethertype)/payload

    # 发送数据包
    sendp(pkt, iface="eno1")
  #  sendp(pkt1, iface="eno1")
    time.sleep(0.1)
