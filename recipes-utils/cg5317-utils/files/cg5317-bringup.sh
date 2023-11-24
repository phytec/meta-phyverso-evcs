#!/bin/sh

ip addr add 192.168.5.11/24 dev seth0
ip addr add 192.168.6.11/24 dev seth1
ip link set seth0 up
ip link set seth1 up
