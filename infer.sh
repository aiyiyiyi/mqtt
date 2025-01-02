#!/bin/bash
source log.sh
#检查infer命令是否存在，将输出重定向到 /dev/null 以抑制任何输出，并将错误也重定向到标准输出
command -v infer >/dev/null 2>&1
#检查上一个命令的退出状态。如果退出状态为 0（表示成功，即 infer 已安装），则执行 then 部分的代码。
if [ $? -eq 0 ]; then
#如果安装，则记录一条日志    
    log_info "infer is installed on this system."
else
#如果未安装，则使用log_err记录一条错误消息
    log_err "infer is not installed on this system."
#并使用 log_debug 记录两条调试消息，提供安装 infer 的命令。
    log_debug "- Please execute the following command to add "infer":"
    log_debug "-- curl -sSL "https://github.com/facebook/infer/releases/download/v1.1.0/infer-linux64-v1.1.0.tar.xz" | tar -C /opt/ -xJ"
    log_debug "-- ln -s /opt/infer-linux64-v1.1.0/bin/infer /usr/local/bin/infer"
#并退出，返回状态码1表示错误
    exit 1
fi
#检查 cmake 是否安装。同上，如果 cmake 未安装，脚本同样使用 exit 1 退出。
command -v cmake >/dev/null 2>&1
if [ $? -eq 0 ]; then
    log_info "cmake is installed on this system."
else
    log_err "cmake is not installed on this system."
    exit 1
fi
#获取当前脚本的绝对目录
CURREN_SCRIPT_ABS_DIR=$(
# 获取脚本所在的目录，并切换到该目录  
    cd $(dirname $0)
#打印当前工作目录的绝对路径
    pwd
)
#记录“开始使用infer构建和分析C++项目”
log_info "Starting to build and analyze the C++ project using infer."
#设置变量 BUILD_TYPE 为 "Release"，指定构建类型
BUILD_TYPE="Release"
#使用 cmake 命令创建一个新的构建目录（${CURREN_SCRIPT_ABS_DIR}/build_infer），并设置构建类型为 Release，同时导出编译命令数据库。
cmake -B ${CURREN_SCRIPT_ABS_DIR}/build_infer -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_EXPORT_COMPILE_COMMANDS=1
#使用 infer run 命令和 --compilation-database 选项指定编译命令数据库的路径，以分析 C++ 项目的构建
infer run --compilation-database ${CURREN_SCRIPT_ABS_DIR}/build_infer/compile_commands.json
#脚本使用 exit 0 正常退出，返回状态码 0 表示成功。
exit 0
#这个脚本用于检查 infer 和 cmake 是否已安装，如果已安装，则使用 cmake 构建一个 C++ 项目，并使用 infer 分析该项目的构建。
#如果任何检查失败（即 infer 或 cmake 未安装），脚本将记录错误消息并退出。脚本还包含了一些用于记录信息和调试消息的日志记录功能。