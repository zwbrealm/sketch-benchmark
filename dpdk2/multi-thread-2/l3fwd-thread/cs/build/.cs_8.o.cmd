cmd_cs_8.o = gcc -Wp,-MD,./.cs_8.o.d.tmp  -m64 -pthread -I/home/zju/Documents/dpdk-20.08/lib/librte_eal/linux/include  -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_MACHINE_CPUFLAG_RDSEED -DRTE_MACHINE_CPUFLAG_FSGSBASE -DRTE_MACHINE_CPUFLAG_F16C -DRTE_MACHINE_CPUFLAG_AVX2  -I/home/zju/Desktop/zwb/libcap_dpdk/sketch/dpdk2/multi-thread-2/l3fwd-thread/cs/build/include -DRTE_USE_FUNCTION_VERSIONING -I/home/zju/Documents/dpdk-20.08/x86_64-native-linuxapp-gcc/include -include /home/zju/Documents/dpdk-20.08/x86_64-native-linuxapp-gcc/include/rte_config.h -D_GNU_SOURCE -O3 -g -mcmodel=medium  -I/home/zju/Documents/dpdk-20.08/examples/performance-thread/common -I/home/zju/Documents/dpdk-20.08/examples/performance-thread/common/arch/x86 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wold-style-definition -Wpointer-arith -Wcast-align -Wnested-externs -Wcast-qual -Wformat-nonliteral -Wformat-security -Wundef -Wwrite-strings -Wdeprecated -Wno-missing-field-initializers -Wimplicit-fallthrough=2 -Wno-format-truncation -Wno-address-of-packed-member -DALLOW_EXPERIMENTAL_API    -o cs_8.o -c /home/zju/Desktop/zwb/libcap_dpdk/sketch/dpdk2/multi-thread-2/l3fwd-thread/cs/cs_8.c 
