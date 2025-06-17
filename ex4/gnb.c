#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

// Định nghĩa cấu trúc bản tin Paging
typedef struct {
    uint32_t message_type; 
    uint32_t ue_id;        
    uint32_t tac;          
    uint32_t cn_domain;    
} Paging_t;

int main() {
    int udp_sockfd, tcp_sockfd, tcp_client_sockfd;

    struct sockaddr_in gnb_udp_addr, ue_addr, gnb_tcp_addr, amf_addr;

    Paging_t ngap_paging, rrc_paging;
    uint16_t sfn = 0;

    // Tạo UDP socket cho giao tiếp với UE
    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        perror("gNodeB: UDP Socket creation failed");
        exit(1);
    }

    // Cấu hình địa chỉ UDP gNodeB
    memset(&gnb_udp_addr, 0, sizeof(gnb_udp_addr));
    gnb_udp_addr.sin_family = AF_INET;
    gnb_udp_addr.sin_addr.s_addr = INADDR_ANY;
    gnb_udp_addr.sin_port = htons(5000);

    // Bind UDP socket
    if (bind(udp_sockfd, (struct sockaddr *)&gnb_udp_addr, sizeof(gnb_udp_addr)) < 0) {
        perror("gNodeB: UDP Bind failed");
        exit(1);
    }

    // Tạo TCP socket cho giao tiếp với AMF
    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
    // Cấu hình địa chỉ TCP gNodeB
    memset(&gnb_tcp_addr, 0, sizeof(gnb_tcp_addr));
    gnb_tcp_addr.sin_family = AF_INET;
    gnb_tcp_addr.sin_addr.s_addr = INADDR_ANY;
    gnb_tcp_addr.sin_port = htons(6000);

    // Bind TCP socket
    if (bind(tcp_sockfd, (struct sockaddr *)&gnb_tcp_addr, sizeof(gnb_tcp_addr)) < 0) {
        perror("gNodeB: TCP Bind failed");
        exit(1);
    }

    // Lắng nghe TCP
    listen(tcp_sockfd, 5);

    // Chấp nhận kết nối từ AMF
    socklen_t amf_addr_len = sizeof(amf_addr);
    tcp_client_sockfd = accept(tcp_sockfd, (struct sockaddr *)&amf_addr, &amf_addr_len);


    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 10000000; // 10ms

    // Cấu hình địa chỉ UE
    memset(&ue_addr, 0, sizeof(ue_addr));
    ue_addr.sin_family = AF_INET;
    ue_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ue_addr.sin_port = htons(5001);

    printf("gNodeB: Started\n");
    while (1) {
        nanosleep(&ts, NULL); // 

        sfn += 1;
        sfn = sfn % 1024; // 
        printf("gNodeB: SFN=%u\n", sfn);

        
        int bytes = recv(tcp_client_sockfd, &ngap_paging, sizeof(ngap_paging), MSG_DONTWAIT);
        if (bytes > 0 && ntohl(ngap_paging.message_type) == 100) {
            printf("gNodeB: Received NgAP Paging (UE_ID=%u, TAC=%u, CN_Domain=%u)\n",
                   ntohl(ngap_paging.ue_id), ntohl(ngap_paging.tac), ntohl(ngap_paging.cn_domain));

            
            rrc_paging = ngap_paging;
            printf("gNodeB: Sending RRC Paging at SFN=%u\n", sfn);
            sendto(udp_sockfd, &rrc_paging, sizeof(rrc_paging), 0, (struct sockaddr *)&ue_addr, sizeof(ue_addr));
            break; 
        }
    }

    close(udp_sockfd);
    close(tcp_client_sockfd);
    close(tcp_sockfd);
    return 0;
}