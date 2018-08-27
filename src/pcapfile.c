/* simple pcap file writer (C) 2018 rofl0r */
#include <unistd.h>
#include <pcap/pcap.h>
#include "utils/endianness.h"

#ifdef SWITCH_ENDIAN
/* if defined allows to use the opposite endian format for testing */
#  define SWAP_IF_SWITCHED(X) end_bswap32(X)
#  define NOT_IF_SWITCHED !
#else
#  define SWAP_IF_SWITCHED(X) X
#  define NOT_IF_SWITCHED
#endif

static void pcapfile_write_header_be(int outfd) {
	write(outfd, "\xA1\xB2\xC3\xD4" "\x00\x02\x00\x04"
	             "\x00\x00\x00\x00" "\x00\x00\x00\x00"
	             "\x00\x04\x00\x00" "\x00\x00\x00\x7F", 24);
}
static void pcapfile_write_header_le(int outfd) {
	write(outfd, "\xD4\xC3\xB2\xA1" "\x02\x00\x04\x00"
	             "\x00\x00\x00\x00" "\x00\x00\x00\x00"
	             "\x00\x00\x04\x00" "\x7F\x00\x00\x00", 24);
}
void pcapfile_write_header(int outfd) {
	if (NOT_IF_SWITCHED ENDIANNESS_BE) pcapfile_write_header_be(outfd);
	else pcapfile_write_header_le(outfd);
}

void pcapfile_write_packet(int outfd, struct pcap_pkthdr *h_out, const unsigned char* data) {
	struct pcap_file_pkthdr {
		unsigned sec_epoch;
		unsigned ms_sec;
		unsigned caplen;
		unsigned len;
	} hdr_out = {
		.sec_epoch = SWAP_IF_SWITCHED(h_out->ts.tv_sec),
		.ms_sec = SWAP_IF_SWITCHED(h_out->ts.tv_usec),
		.caplen = SWAP_IF_SWITCHED(h_out->caplen),
		.len = SWAP_IF_SWITCHED(h_out->len),
	};
	write(outfd, &hdr_out, sizeof hdr_out);
	write(outfd, data, h_out->len);
}

