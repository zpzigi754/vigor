#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>

#include <rte_byteorder.h>
#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_mbuf.h>
#include <rte_mbuf_ptype.h>
#include <rte_tcp.h>
#include <rte_udp.h>

#include "nf_util.h"

void* chunks_borrowed[MAX_N_CHUNKS];
size_t chunks_borrowed_num = 0;

uint32_t global_packet_type = 0;

bool
nf_has_tcpudp_header(struct ipv4_hdr* header)
{
  // NOTE: Use non-short-circuiting version of OR, so that symbex doesn't fork
  //       since here we only care of it's UDP or TCP, not if it's a specific one
  return header->next_proto_id == IPPROTO_TCP | header->next_proto_id == IPPROTO_UDP;
}

void
nf_set_ipv4_checksum_hw(struct rte_mbuf *mbuf, struct ipv4_hdr* ip_header, void *l4_header)
{
	/*
		https://doc.dpdk.org/guides/prog_guide/mbuf_lib.html#meta-information

		In order to use hardware offloading of header checksum we need to set
		a few parameters in the mbuf structure before sending the packet.

		We need to set
			* the IPv4 checksum to 0
			* the l2 length to the size of the Ethernet header
			* the l3 length to the size of the IPv4 header
			* the offloading flags to PKT_TX_IPV4 | PKT_TX_IP_CKSUM ORed with
				either PKT_TX_TCP_CKSUM or PKT_TX_UDP_CKSUM depending on the type
				of the packet (setting both all the time doesn't work, the NIC
				will not send the packet)
			* the TCP/UDP header to the IPv4 pseudoheader checksum

		If any of these are wrong the NIC will not send the packet

		The NIC itself and the tx queue need to be configured for this as well,
		see nf_init_device.
	*/

  ip_header->hdr_checksum = 0;

  mbuf->ol_flags |= PKT_TX_IPV4 | PKT_TX_IP_CKSUM;
  mbuf->l2_len = sizeof(struct ether_hdr);
  mbuf->l3_len = sizeof(struct ipv4_hdr);

	if (ip_header->next_proto_id == IPPROTO_TCP) {
		struct tcp_hdr* tcp_header = (struct tcp_hdr*) l4_header;
		tcp_header->cksum = rte_ipv4_phdr_cksum(ip_header, mbuf->ol_flags);
		mbuf->ol_flags |= PKT_TX_TCP_CKSUM;
	} else if (ip_header->next_proto_id == IPPROTO_UDP) {
		struct udp_hdr * udp_header = (struct udp_hdr*) l4_header;
		udp_header->dgram_cksum = rte_ipv4_phdr_cksum(ip_header, mbuf->ol_flags);
		mbuf->ol_flags |= PKT_TX_UDP_CKSUM;
	}
}

void
nf_set_ipv4_udptcp_checksum(struct ipv4_hdr* ip_header, struct tcpudp_hdr* l4_header, void* packet) {
  // Make sure the packet pointer points to the TCPUDP continuation
  void* payload = nf_borrow_next_chunk(packet, ip_header->total_length - sizeof(struct tcpudp_hdr));
  assert((char*)payload == ((char*)l4_header + sizeof(struct tcpudp_hdr)));

  ip_header->hdr_checksum = 0; // Assumed by cksum calculation
  if (ip_header->next_proto_id == IPPROTO_TCP) {
    struct tcp_hdr* tcp_header = (struct tcp_hdr*) l4_header;
    tcp_header->cksum = 0; // Assumed by cksum calculation
    tcp_header->cksum = rte_ipv4_udptcp_cksum(ip_header, tcp_header);
  } else if (ip_header->next_proto_id == IPPROTO_UDP) {
    struct udp_hdr * udp_header = (struct udp_hdr*) l4_header;
    udp_header->dgram_cksum = 0; // Assumed by cksum calculation
    udp_header->dgram_cksum = rte_ipv4_udptcp_cksum(ip_header, udp_header);
  }
  ip_header->hdr_checksum = rte_ipv4_cksum(ip_header);
}


uintmax_t
nf_util_parse_int(const char* str, const char* name,
                  int base, char next) {
  char* temp;
  intmax_t result = strtoimax(str, &temp, base);

  // There's also a weird failure case with overflows, but let's not care
  if(temp == str || *temp != next) {
    rte_exit(EXIT_FAILURE, "Error while parsing '%s': %s\n", name, str);
  }

  return result;
}

char*
nf_mac_to_str(struct ether_addr* addr)
{
	// format is xx:xx:xx:xx:xx:xx\0
	uint16_t buffer_size = 6 * 2 + 5 + 1; //FIXME: why dynamic alloc here?
	char* buffer = (char*) calloc(buffer_size, sizeof(char));
	if (buffer == NULL) {
		rte_exit(EXIT_FAILURE, "Out of memory in nf_mac_to_str!");
	}

	ether_format_addr(buffer, buffer_size, addr);
	return buffer;
}

char*
nf_ipv4_to_str(uint32_t addr)
{
	// format is xxx.xxx.xxx.xxx\0
	uint16_t buffer_size = 4 * 3 + 3 + 1;
	char* buffer = (char*) calloc(buffer_size, sizeof(char)); //FIXME: why dynamic alloc here?
	if (buffer == NULL) {
		rte_exit(EXIT_FAILURE, "Out of memory in nf_ipv4_to_str!");
	}

	snprintf(buffer, buffer_size, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8,
		 addr        & 0xFF,
		(addr >>  8) & 0xFF,
		(addr >> 16) & 0xFF,
		(addr >> 24) & 0xFF
	);
	return buffer;
}
