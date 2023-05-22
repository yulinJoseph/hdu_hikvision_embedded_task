CROSS_COMPILE = arm-fullhanv3-linux-uclibcgnueabi-
# CFLAGS = -Wall -Werror
CFLAGS += -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
CFLAGS += -lstdc++ -ldl -ffunction-sections -fdata-sections -ftree-vectorize -fPIC
CFLAGS += -lpthread -lm -lrt -ldl -rdynamic
CXX_FLAGS = -std=c++17

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++

ROOT_DIR = .
LIBS_DIR = $(ROOT_DIR)/lib/static

INC_DIR = -I$(ROOT_DIR)/inc/
INC_DIR += -I$(ROOT_DIR)/inc/wrapper/
INC_DIR += -I$(ROOT_DIR)/inc/include/
INC_DIR += -I$(ROOT_DIR)/inc/include/dsp/
INC_DIR += -I$(ROOT_DIR)/inc/include/dsp_ext/
INC_DIR += -I$(ROOT_DIR)/inc/include/isp/
INC_DIR += -I$(ROOT_DIR)/inc/include/isp_ext/
INC_DIR += -I$(ROOT_DIR)/inc/include/mpp/
INC_DIR += -I$(ROOT_DIR)/inc/include/types/
INC_DIR += -I$(ROOT_DIR)/inc/include/vicap/

CFLAGS += $(INC_DIR)

TARGET = main

CC_SRCS = $(wildcard ./src/*.c)
CC_OBJS = $(patsubst %.c,%.o,$(CC_SRCS))
CXX_SRCS = $(wildcard ./src/*.cpp ./src/wrapper/*.cpp)
CXX_OBJS = $(patsubst %.cpp,%.o,$(CXX_SRCS))

FH_MPP_LIBS = \
	$(LIBS_DIR)/libdsp.a \
	$(LIBS_DIR)/libdbi.a \
	$(LIBS_DIR)/libvb_mpi.a \
	$(LIBS_DIR)/libvmm.a \
	$(LIBS_DIR)/libmipi.a \
	$(LIBS_DIR)/libimx415_mipi.a \
	$(LIBS_DIR)/libisp.a \
	$(LIBS_DIR)/libispcore.a \
	$(LIBS_DIR)/libadvapi_osd.a \
	$(LIBS_DIR)/libadvapi.a


all: $(TARGET)
$(CC_OBJS): %.o:%.c
	@$(CC) $(CFLAGS) -c $< -o $@
$(CXX_OBJS): %.o:%.cpp
	@$(CXX) $(CFLAGS) $(CXX_FLAGS) -c $< -o $@
$(TARGET): $(CC_OBJS) $(CXX_OBJS)
	@$(CXX) $(CFLAGS) $(CXX_FLAGS) -o $@ $(CC_OBJS) $(CXX_OBJS) $(FH_MPP_LIBS)
	@rm -rf $(ROOT_DIR)/src/*.o
	@rm -rf $(ROOT_DIR)/src/wrapper/*.o

.PHONY: clean
clean:
	rm -rf $(ROOT_DIR)/src/*.o 
	rm -rf $(ROOT_DIR)/src/wrapper/*.o
	rm -rf $(TARGET)
