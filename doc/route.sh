#!/bin/sh

# 解决自动获取IP无法连外网问题
# 先将网卡设置成自动获取IP

# 172.31.10.254 为网卡获取的网关
# 172.16.255.254为pppoe获取的网关
# 外网连接走pppoe网关，内网连接走网卡网关

# 删掉默认外网连接网关
sudo route delete default 172.31.10.254
# 指定默认外网连接网关
sudo route add default 172.16.255.254
#if [ "$?" -ne "0" ]; then
sudo route add default 172.16.255.253
#fi
# 指定开发环境网关
sudo route -n add -net 10.1.0.0/16 172.31.10.254
sudo route -n add -net 172.31.0.0/16 172.31.10.254
#sudo route -n add -net 10.1.0.0/16 192.168.31.1

#指定线上环网关
#sudo route -n add -net 192.168.1.0/24 192.168.31.1
#sudo route -n add -net 192.168.8.0/24 172.31.10.254
sudo route -n add -net 192.168.1.0/24 192.168.31.1
sudo route -n add -net 192.168.3.0/24 192.168.31.1
sudo route -n add -net 192.168.8.0/24 192.168.31.1
