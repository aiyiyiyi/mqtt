#!/bin/bash
#调用log.sh脚本执行     （引用外部脚本）
source log.sh
# Function to create the build directory and compile
build() {
    #判断是否存在目录build
    if [ ! -d "./build" ]; then
        #如果不存在，则创建一个build目录，并向log_info函数发送字符串参数，记录一条信息级别的日志。
        log_info "Creating build directory..."
        mkdir build
        #if条件语句的结束标志
    fi
    #继续发送
    log_info "Entering build directory..."
    #进入build目录，如果不存在，则退出
    cd build || exit
    #向 log_info 函数传递的字符串参数是 "Running cmake..."，意味着脚本正在记录一条关于即将运行 cmake 命令的日志消息。
    log_info "Running cmake..."
    #在 build 目录中运行 cmake .. 命令来配置项目。.. 表示 cmake 应该在当前目录的上一级目录中查找 CMakeLists.txt 文件。
    cmake -DCMAKE_INSTALL_PREFIX=../output ..
    #正在记录一条关于即将开始编译过程的日志消息。
    log_info "Compiling..."
    #编译    j代表可以运行编译多个项目
    make -j
    #脚本正在记录一条关于即将退出构建目录的日志消息。
    log_info "Exiting build directory..."
    #返回上级目录
    cd ..
}

install() {
    if [ -d "./output" ]; then
        log_info "Cleaning install directory..."
        rm -rf output/*
    else
        log_warn "install directory does not exist."
    fi

    log_info "Entering build directory..."
    cd build || exit
    log_info "Begin install..."
    make install

    log_info "Exiting build directory..."
    cd ..

}

# Function to clean the build directory
#如果 ./build 目录存在，则删除该目录中的所有文件和子目录（rm -rf build/*），并记录一条信息日志。如果 ./build 目录不存在，则记录一条信息日志表示构建目录不存在。
clean() {
    if [ -d "./build" ]; then
        log_info "Cleaning build directory..."
        rm -rf build/*
    else
        log_info "Build directory does not exist."
    fi
}
#======================================================================================
# Main script logic    主要脚本逻辑
#根据传递给脚本的第一个参数（$1），决定是执行 clean 函数还是 build 函数。
#如果参数是 clean，则调用 clean 函数。
#如果参数不是 clean（或者没有提供参数），则调用 build 函数。
if [ "$1" == "clean" ]; then
    clean
elif [ "$1" == "install" ]; then
    log_info "installing..."
    install
else
    build
fi
#检查当前目录下是否存在 build 目录，如果不存在，则记录一条日志并创建该目录。这是构建脚本中常见的步骤，用于确保构建过程有一个干净的起始点。
