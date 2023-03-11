#!/usr/bin/bash

# 开放端口
sudo firewall-cmd --zone=public --add-port=8888/tcp

# 运行程序
./chatServer 8888 5
