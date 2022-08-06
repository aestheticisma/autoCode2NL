CXX = g++
CFLAGS = -std=c++14 -Wall -g

TARGET = server
OBJS = ../src/log/*.cc ../src/timer/*.cc \
	   ../src/http/*.cc ../src/server/*.cc \
	   ../src/buffer/*.cc ../src/main.cc \
	   ../src/grpc/proto/*.cc

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET) -pthread `pkg-config --libs --cflags protobuf grpc++`

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
