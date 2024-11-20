#!/usr/bin/sh
for file in *.pcap; do
    tcpdump -r $file;
done