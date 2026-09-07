# 编译器与编译选项
CXX      := g++
CXXFLAGS := -std=c++11 -O2 -Wall -Wextra -pthread
INCLUDES := -I./ -I../

# 目标可执行文件名称
TARGET   := benchmarks_main

# 自动匹配当前目录及 platform 目录下的所有 .cpp 文件
SRCS     := $(wildcard *.cpp) $(wildcard platform/*.cpp)
OBJS     := $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# 编译 .cpp 为 .o（仅依赖 .cpp 本身）
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 运行程序
run: $(TARGET)
	./$(TARGET)

# 清理编译生成的文件
clean:
	rm -f $(TARGET) $(OBJS)
