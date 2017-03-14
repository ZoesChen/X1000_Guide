FINAL_OBJ := libpthread_demo.so
CXX := mips-linux-gnu-g++
LD := mips-linux-gnu-ld
CFLAG ?= -fpermissive -I./include
LDFLAG := -lpthread
STRIP := mips-linux-gnu-strip
RUNLIB := mips-linux-gnu-ranlib
AR := mips-linux-gnu-ar -cr

SRC_DIR := ./src
SRC := $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(patsubst %.cpp,%.o,${SRC})

$(FINAL_OBJ) : $(OBJS)
	#$(CC) $^ -o $@ $(LDFLAG)
	$(CXX) -shared -fPIC -o $@ $^
	$(STRIP) $@
	#$(AR) $@  $^
	#$(RUNLIB) $@
%.o : %.cpp
	@$(CXX) -fPIC $(CFLAG) $^ -c -o $@
clean:
	rm -rf ./src/*.o $(FINAL_OBJ) ./src/*.swp ./*.swp ./include/*.swp ./*.a ./*.out ./thread_c++_demo ./thread_c_demo
