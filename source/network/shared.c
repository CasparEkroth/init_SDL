#include "shared.h"

#if _WIN32
    //for geting ptint statmeins while on PC
    void open_console(){
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif

char* get_local_ip() {
    static char ip[INET_ADDRSTRLEN] = {0};

#if defined(_WIN32)

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return NULL;

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        WSACleanup();
        return NULL;
    }

    struct hostent* host_entry = gethostbyname(hostname);
    if (!host_entry) {
        WSACleanup();
        return NULL;
    }

    for (int i = 0; host_entry->h_addr_list[i] != NULL; i++) {
        struct in_addr addr;
        memcpy(&addr, host_entry->h_addr_list[i], sizeof(struct in_addr));
        if (strcmp(inet_ntoa(addr), "127.0.0.1") != 0) {
            strncpy(ip, inet_ntoa(addr), sizeof(ip));
            break;
        }
    }

    WSACleanup();

#elif defined(__APPLE__) || defined(__linux__)
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;

        struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
        if (inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip))) {
            break;
        }
    }

    freeifaddrs(ifaddr);
#endif

    return ip[0] ? ip : NULL;
}


char* get_local_broadcast(){
    char* ip = get_local_ip(); 
    static char broadcast_ip[INET_ADDRSTRLEN] = {0};

    if (!ip) return NULL;
    int len = strlen(ip);
    int last_dot = -1;
    for (int i = 0; i < len; i++) {
        if (ip[i] == '.') {
            last_dot = i;
        }
    }
    if (last_dot == -1 || last_dot + 4 >= INET_ADDRSTRLEN) {
        return NULL;  // invalid IP format
    }
    strncpy(broadcast_ip, ip, last_dot + 1);
    strcpy(broadcast_ip + last_dot + 1, "255");// append "255"
    return broadcast_ip;
}

