#ifndef PCAPFILE_H
#define PCAPFILE_H

#include <pcap/pcap.h>

void pcapfile_write_header(int outfd);
void pcapfile_write_packet(int outfd, struct pcap_pkthdr *h_out, const unsigned char* data);

//RcB: DEP "pcapfile.c"

#endif

