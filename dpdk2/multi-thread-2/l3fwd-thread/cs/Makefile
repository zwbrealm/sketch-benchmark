# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2010-2015 Intel Corporation

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, detect a build directory, by looking for a path with a .config
RTE_TARGET ?= $(notdir $(abspath $(dir $(firstword $(wildcard $(RTE_SDK)/*/.config)))))

include $(RTE_SDK)/mk/rte.vars.mk

# binary namecd 
APP = cs_8

# all source are stored in SRCS-y
SRCS-y := cs_8.c
# SRCS-y += uthash.c
include $(RTE_SDK)/examples/performance-thread/common/common.mk

CFLAGS += -O3 -g -mcmodel=medium $(USER_FLAGS) $(INCLUDES) $(WERROR_FLAGS)
CFLAGS += -DALLOW_EXPERIMENTAL_API

include $(RTE_SDK)/mk/rte.extapp.mk
