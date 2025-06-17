#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


typedef struct {
    uint32_t message_type; 
    uint32_t ue_id;     
    uint32_t tac;         
    uint32_t cn_domain;    
} Paging_t;

int main() {
    int sockfd;
    struct sockaddr_in gnb_addr;

    // Tạo TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("AMF: Socket creation failed");
        exit(1);
    }

    // Cấu hình địa chỉ gNodeB
    memset(&gnb_addr, 0, sizeof(gnb_addr));
    gnb_addr.sin_family = AF_INET;
    gnb_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    gnb_addr.sin_port = htons(6000);

    // Kết nối đến gNodeB
    if (connect(sockfd, (struct sockaddr *)&gnb_addr, sizeof(gnb_addr)) < 0) {
        perror("AMF: Connection failed");
        exit(1);
    }

    // Tạo bản tin NgAP Paging
    Paging_t ngap_paging;
    ngap_paging.message_type = htonl(100);
    ngap_paging.ue_id = htonl(1234);
    ngap_paging.tac = htonl(100);   
    ngap_paging.cn_domain = htonl(100); 

   
    printf("AMF: Sending NgAP Paging (UE_ID=%u, TAC=%u, CN_Domain=%u)\n",
           ntohl(ngap_paging.ue_id), ntohl(ngap_paging.tac), ntohl(ngap_paging.cn_domain));
    send(sockfd, &ngap_paging, sizeof(ngap_paging), 0);

    close(sockfd);
    return 0;
}