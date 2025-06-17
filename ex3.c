#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

// Định nghĩa cấu trúc bản tin MIB
typedef struct {
    uint8_t message_id; // 1 byte, giá trị 0x01
    uint16_t sfn_value; // 2 byte, giá trị gNodeB_sfn
} MIB_t;


// Hàm gửi bản tin MIB từ gNodeB
void send_mib(int sockfd, struct sockaddr_in *ue_addr, uint16_t gnb_sfn) {
    MIB_t mib;
    mib.message_id = 0x01;
    mib.sfn_value = htons(gnb_sfn); // Chuyển sang network byte order, tránh bị nhận sai giá trị
    sendto(sockfd, &mib, sizeof(mib), 0, (struct sockaddr *)ue_addr, sizeof(*ue_addr));
    printf("gNodeB: Sent MIB, SFN=%d\n", gnb_sfn);
}

// Tiến trình gNodeB
void gnodeb_process() {
    
    int sockfd;
    struct sockaddr_in  ue_addr;
    uint16_t gnb_sfn = 0;
    
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 10000000; // 10ms
    
    // Tạo UDP socket để giao tiếp giữa 2 tiến trình
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // IPv4 + Diagram 
    
    
    // Cấu hình địa chỉ UE
    memset(&ue_addr, 0, sizeof(ue_addr));
    ue_addr.sin_family = AF_INET;
    ue_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ue_addr.sin_port = htons(8081);
    
    int mib_counter = 0;
    
    while ((gnb_sfn < 1024)) {
        nanosleep(&ts, NULL);
        
        gnb_sfn += 1;
        //gnb_sfn = gnb_sfn % 1024;
        
        mib_counter += 1;
        
        if (mib_counter == 8) {
            send_mib(sockfd, &ue_addr, gnb_sfn);
            mib_counter = 0;
        }
    } 
    
    close(sockfd);
}


void ue_process() {
    int sockfd;
    struct sockaddr_in  ue_addr;
    uint16_t ue_sfn = 0;
    
    int is_sync = 0;
    
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 10000000; // 10ms
    
    // Tạo UDP socket để giao tiếp giữa 2 tiến trình
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // IPv4 + Diagram 
    
    
    // Cấu hình địa chỉ UE
    memset(&ue_addr, 0, sizeof(ue_addr));
    ue_addr.sin_family = AF_INET;
    ue_addr.sin_addr.s_addr = INADDR_ANY;
    ue_addr.sin_port = htons(8081);
    
    
    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&ue_addr, sizeof(ue_addr)) < 0) {
        perror("UE: Bind failed");
        exit(1);
    }
    
    int sync_counter = 0;
    
    MIB_t mib;
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    
    while ((ue_sfn < 1024)) {
        nanosleep(&ts, NULL); 
        
        ue_sfn += 1;
       // ue_sfn = ue_sfn % 1024;
        
        printf("UE: SFN=%d, Sync=%s\n", ue_sfn, is_sync ? "Yes" : "No");
        
        
        
        // Nhận MIB từ gNodeB
        int check = recvfrom(sockfd, &mib, sizeof(mib),MSG_DONTWAIT, (struct sockaddr *)&from_addr, &addr_len);
        if(check > 0 && mib.message_id == 0x01) {
                uint16_t gnb_sfn = ntohs(mib.sfn_value);
                 
                printf("UE: Received MIB, SFN=%d\n", gnb_sfn);
                
                if (!is_sync) {
                    ue_sfn = gnb_sfn;
                    is_sync = 1;
                    int sync_counter = 0;
                    printf("Update not Sync, SFN=%d\n", gnb_sfn);
                } else {
                    sync_counter += 1;
                    if (sync_counter >= 10) { 
                        ue_sfn = gnb_sfn;
                        sync_counter = 0;
                        printf("Update Sync, SFN=%d\n", gnb_sfn);
                    }
                }
 
            }
    
    }
    close(sockfd);
}




int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process: UE
        printf("Starting UE process...\n");
        ue_process();
    } else {
        // Parent process: gNodeB
        printf("Starting gNodeB process...\n");
        gnodeb_process();
    }

    return 0;
}