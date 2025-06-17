#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>


typedef struct {
    uint32_t message_type; 
    uint32_t ue_id;        
    uint32_t tac;          
    uint32_t cn_domain;    
} Paging_t;

int main() {
    int sockfd;
    struct sockaddr_in ue_addr;

    Paging_t rrc_paging;

    uint16_t sfn = 0;

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 10000000; // 10ms

    // Tạo UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Cấu hình địa chỉ UE
    memset(&ue_addr, 0, sizeof(ue_addr));
    ue_addr.sin_family = AF_INET;
    ue_addr.sin_addr.s_addr = INADDR_ANY;
    ue_addr.sin_port = htons(5001);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&ue_addr, sizeof(ue_addr)) < 0) {
        perror("UE: Bind failed");
        exit(1);
    }


    int cycle_counter = 0;
    printf("UE: Started\n");
    while (1) {

        nanosleep(&ts, NULL); 

        sfn +=1;
        sfn = sfn % 1024;

        cycle_counter++;

        if (cycle_counter >= 8) {
            printf("UE: SFN=%u\n", sfn);
            struct sockaddr_in from_addr;
            socklen_t addr_len = sizeof(from_addr);

            // Thiết lập timeout 80ms
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 80000;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            int check = recvfrom(sockfd, &rrc_paging, sizeof(rrc_paging), 0, (struct sockaddr *)&from_addr, &addr_len);
            
            if (check > 0 && ntohl(rrc_paging.message_type) == 100 && ntohl(rrc_paging.tac) == 100) {
                printf("UE: Received RRC Paging, UE_ID=%u, TAC=%u, CN_Domain=%u\n",
                       ntohl(rrc_paging.ue_id), ntohl(rrc_paging.tac), ntohl(rrc_paging.cn_domain));
                break; 
            }
            cycle_counter = 0;
        }
    }

    close(sockfd);
    return 0;
}