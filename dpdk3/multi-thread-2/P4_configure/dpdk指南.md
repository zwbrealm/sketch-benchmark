MLNX版本下载，直接wget

https://content.mellanox.com/ofed/MLNX_OFED-5.4-3.6.8.1/MLNX_OFED_LINUX-5.4-3.6.8.1-ubuntu20.04-x86_64.tgz

然后解压缩，安装



tar -xvzf

sudo ./mlnxofedinstall --dpdk

然后按照他输出的提示restart

/etc/init.d/openibd restart

# How to run the pktgen





To run the tests in this repo, you first need to configure the packet generator on the second server machine, directly connected to the DUT.
We use Pktgen-DPDK to generate random packets used for the syntetic trace experiments and dpdk-burst-replay to send the [IMC trace](http://pages.cs.wisc.edu/~tbenson/IMC10_Data.html).


#  hugepages
echo 8000 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
echo 8000 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages





echo Y | sudo tee /sys/module/vfio/parameters/enable_unsafe_noiommu_mode

## Install DPDK

The first thing to do is to install DPDK.
To do so, follow these instructions:

```shell
# Clone DPDK
wget https://fast.dpdk.org/rel/dpdk-20.11.3.tar.xz

tar -xvf dpdk-20.11.3.tar.xz
cd dpdk-20.11.3

meson build
ninja -C build
sudo ninja -C build install
```

After DPDK is installed, you need to mount the hugepages and bind the ports to the DPDK UIO driver.
Follow [these](https://doc.dpdk.org/guides/linux_gsg/sys_reqs.html#running-dpdk-applications) instructions to do it.

## Pktgen-DPDK
After DPDK is installed, now it is time to install pktgen-DPDK.

```shell
# Clone Pktgen-DPDK
git clone --depth 1 --branch pktgen-21.03.1 https://gitee.com/YouWeiNetwork/Pktgen-DPDK.git

cd Pktgen-DPDK
meson build
ninja -C build
sudo ninja -C build install
sudo ldconfig

分配大页-单节点
echo 8000 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
```




绑定网卡
enp101s0f0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether 0c:42:a1:7a:bc:60  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp101s0f1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.10.1  netmask 255.255.255.0  broadcast 192.168.10.255
        inet6 fe80::6943:250a:937c:59d0  prefixlen 64  scopeid 0x20<link>
        ether 0c:42:a1:7a:bc:61  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 474  bytes 30948 (30.9 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp179s0f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether 0c:42:a1:7a:bc:80  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 365  bytes 61527 (61.5 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp179s0f1: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether 0c:42:a1:7a:bc:81  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 79218  bytes 13295650 (13.2 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

0000:17:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' if=eno1 drv=tg3 unused=vfio-pci *Active*
0000:17:00.1 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' if=eno2 drv=tg3 unused=vfio-pci *Active*
0000:18:00.0 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' if=eno3 drv=tg3 unused=vfio-pci
0000:18:00.1 'NetXtreme BCM5720 Gigabit Ethernet PCIe 165f' if=eno4 drv=tg3 unused=vfio-pci
0000:65:00.0 'MT27800 Family [ConnectX-5] 1017' if=enp101s0f0 drv=mlx5_core unused=vfio-pci
0000:65:00.1 'MT27800 Family [ConnectX-5] 1017' if=enp101s0f1 drv=mlx5_core unused=vfio-pci *Active*

Other Network devices
=====================
0000:b3:00.0 'MT27800 Family [ConnectX-5] 1017' unused=mlx5_core,vfio-pci
0000:b3:00.1 'MT27800 Family [ConnectX-5] 1017' unused=mlx5_core,vfio-pci



At this point, Pktgen-DPDK is installed. To run the generator uses the following command.
Please note that the cores allocation should be customized to your environment.

```shell
sudo /usr/local/bin/pktgen -c fffff -n 4 --proc-type auto --file-prefix pg -- -T -P -m "[1/3/5:7/9].0, [11/13/15:17/19].1" -f pktgen-dpdk-cfg

sudo /usr/local/bin/pktgen -c fffff -n 4 --proc-type auto --file-prefix pg -- -T -P -m "[1:7].0, [11:17].1" -f pktgen-dpdk-cfg 
 
```

After the initialization, you can type `start 0` to start sending packets.

## DPDK Burst Replay
In our tests, we use the data center trace UNI1 from [this link](http://pages.cs.wisc.edu/~tbenson/IMC10_Data.html).
Before start sending the trace, we need to do some additional elaboration to (1) add the Ethernet layer and (2) add a payload in the packets of the trace to match the wire size, since the capture file was trimmed for privacy reasons.

In all the commands, the parameters under <> should be filled with your settings.

### Trace preparation
Download the trace, and run the following command to add the Ethernet layer.

```shell
wget http://pages.cs.wisc.edu/~tbenson/IMC_DATA/univ1_trace.tgz
tar -xzf univ1_trace.tgz
tcprewrite --dlt=enet --enet-dmac=<mac_src> --enet-smac=<mac_dst> --infile=univ1_pt1 --outfile=output.pcap
```

Then, clone the following repository and run this script.

```shell
git clone git@github.com:sebymiano/classbench-generators.git

cd classbench-generators
pip3 install -r requirements.txt

python3 pcap/convert-trace-with-right-size-single-core.py -i output.pcap -o output_complete.pcap -s <mac_src> -d <mac_dst> -p
```

The last script will take a while until the trace is created.
After this step, you can start sending the trace with DPDK Burst Replay.

### DPDK Burst Replay Installation
Clone the DPDK Burst Replay repository and apply our patch.

```shell
git clone https://github.com/FraudBuster/dpdk-burst-replay.git

cd dpdk-burst-replay
git apply dpdk-burst-replay.patch

autoreconf -i && ./configure && make && sudo make install
```

At this point, the dpdk-burst-replay tool is installed. To start sending the trace, run this command.

```shell
sudo dpdk-replay --nbruns 100000000000 --numacore <set_numa_id (e.g., 1)> output_complete.pcap <pci_dev_id (e.g., 0000:af:00.0)>

sudo dpdk-replay --nbruns 10000 --numacore 1 test1.pcap 0000:af:00.0
```




Packet length 60 is to short to contain a layer 52 byte IP header for DLT 0x0001











/home/zju/Desktop/zwb/libcap_dpdk/sketch/dpdk/performance-thread-2/l3fwd-thread/build

./es -l 0-3 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)"





0000:65:00.0 'MT27800 Family [ConnectX-5] 1017' if=ens1f0 drv=mlx5_core unused=vfio-pci
0000:65:00.1 'MT27800 Family [ConnectX-5] 1017' if=ens1f1 drv=mlx5_core unused=vfio-pci
0000:b3:00.0 'MT27800 Family [ConnectX-5] 1017' if=ens3f0 drv=mlx5_core unused=vfio-pci
0000:b3:00.1 'MT27800 Family [ConnectX-5] 1017' if=ens3f1 drv=mlx5_core unused=vfio-pci