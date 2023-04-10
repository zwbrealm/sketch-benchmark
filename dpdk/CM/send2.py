from scapy.all import *

# 构造自定义载荷，这里使用字符串作为载荷
payload = "Hello, Scapy!"

# 构造以太网头并设置目标 MAC 地址
ether_pkt = Ether(dst="d0:9f:d9:70:50:68") / payload

while True:
    sendp(ether_pkt, iface='ens4f0')


