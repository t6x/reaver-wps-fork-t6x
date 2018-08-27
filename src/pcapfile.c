#include <unistd.h>
#include <pcap/pcap.h>

void pcapfile_write_header(int outfd) {
	write(outfd, "\xD4\xC3\xB2\xA1" "\x02\x00\x04\x00"
	             "\x00\x00\x00\x00" "\x00\x00\x00\x00"
	             "\x00\x00\x04\x00" "\x7F\x00\x00\x00", 24);
}

void pcapfile_write_packet(int outfd, struct pcap_pkthdr *h_out, const unsigned char* data) {
	struct pcap_file_pkthdr {
		unsigned sec_epoch;
		unsigned ms_sec;
		unsigned caplen;
		unsigned len;
	} hdr_out = {
		.sec_epoch = h_out->ts.tv_sec,
		.ms_sec = h_out->ts.tv_usec,
		.caplen = h_out->caplen,
		.len = h_out->len,
	};
	write(outfd, &hdr_out, sizeof hdr_out);
	write(outfd, data, h_out->len);
}

