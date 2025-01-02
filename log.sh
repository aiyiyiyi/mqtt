#!/bin/bash
#日志级别 debug-1, info-2, warn-3, error-4, always-5
#LOG_LEVEL 变量设置了日志的级别。在这个例子中，它被设置为 1，这意味着只有 debug 级别（1）及以上的日志会被打印和/或记录。
LOG_LEVEL=1   #包含debug日志及以后的打印
#日志文件
#LOG_FILE 变量指定了日志文件的路径。在这个例子中，它被设置为 /dev/null，这是一个特殊的文件，会丢弃所有写入其中的数据。
LOG_FILE=/dev/null
#调试日志
#这个函数创建一个包含当前日期、时间和日志消息的字符串。
function log_debug(){
  content="[DEBUG] [$(date '+%Y-%m-%d %H:%M:%S]') $@"
#检查当前的日志级别是否允许打印 debug 消息。如果允许，它将消息追加到 LOG_FILE 指定的文件，并在终端上以绿色打印消息（使用 ANSI 转义序列）
#\033[32m 设置绿色，\033[0m 重置颜色
  [ $LOG_LEVEL -le 1  ] && echo $content >> $LOG_FILE && echo -e "\033[32m"  ${content}  "\033[0m"
}
#信息日志
function log_info(){
  content="[INFO] [$(date '+%Y-%m-%d %H:%M:%S')]  $@"
  [ $LOG_LEVEL -le 2  ] && echo $content >> $LOG_FILE && echo -e "\033[36m"  ${content} "\033[0m"
}
#警告日志
function log_warn(){
  content="[WARN] [$(date '+%Y-%m-%d %H:%M:%S')] $@"
  [ $LOG_LEVEL -le 3  ] && echo $content >> $LOG_FILE && echo -e "\033[33m" ${content} "\033[0m"
}
#错误日志
function log_err(){
  content="[ERROR] [$(date '+%Y-%m-%d %H:%M:%S')] $@"
  [ $LOG_LEVEL -le 4  ] && echo $content >> $LOG_FILE && echo -e "\033[31m" ${content} "\033[0m"
}
#一直都会打印的日志
function log_always(){
   content="[ALWAYS] [$(date '+%Y-%m-%d %H:%M:%S')] $@"
   [ $LOG_LEVEL -le 5  ] && echo $content >> $LOG_FILE && echo -e  "\033[37m" ${content} "\033[0m"
}
#shopt -s expand_aliases 启用别名扩展。
shopt -s expand_aliases
#source ~/.bashrc 加载用户的 .bashrc 文件，这通常包含用户定义的别名、函数和其他配置。
source ~/.bashrc
#脚本为每个日志函数定义了一个别名，这些别名在调用时会将当前行号（$LINENO）作为参数传递给相应的日志函数。
alias log_debug='log_debug $LINENO:'
alias log_info='log_info $LINENO:'
alias log_warn='log_warn $LINENO:'
alias log_err='log_err $LINENO:'
alias log_always='log_always $LINENO:'