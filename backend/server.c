/*
 * server.c
 * backend中的c文件为dijkstra算法。
 * 本地http socket部署，由ChatGPT生成。
 *
 * 支持两个接口：
 *   GET /nodes
 *   GET /route?start=<id>&end=<id>
 *
 * 用 gcc 编译。
 *
 * 编译:
 *   Linux/macOS:
 *     gcc server.c -o server
 *   Windows (MinGW-w64):
 *     gcc server.c -lws2_32 -o server.exe
 *
 * 运行:
 *   ./server    （或 server.exe）
 *
 * 运行程序效果，切到frontend中的index.html然后Run Live Server于端口
 * 每次更新server.c中语句，运行前都需要用MinGW编译。打开Powershell，找到backend的文件目录，然后运行gcc server.c -lws2_32 -o server.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
  typedef int socklen_t;
#else
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
#endif

#define PORT 8080
#define BACKLOG 10
#define RECVBUF 4096

#define MAXN 128
#define INF 1000000000

// Node structure
typedef struct {
    int id;
    char label[64];
    // position of marker on image in percentage (0..100)
    double x_pct;
    double y_pct;
} Node;

static Node nodes[MAXN];
static int node_count = 0;

// adjacency matrix weights (0 = no edge)
static int adj[MAXN][MAXN];

// 初始化无向图
void init_graph() {
    int i, j;
    node_count = 22; // 22个节点

    // 邻接矩阵，循环存数组
    for (i = 0; i < MAXN; i++) {
        for (j = 0; j < MAXN; j++) {
            adj[i][j] = 0;
        }
    }

    // DEFINE LOCATIONS / NUMBER / BUTTON LOCAT / LABEL NAME
    nodes[0].id = 0; strcpy(nodes[0].label, "1-B栋"); nodes[0].x_pct = 22; nodes[0].y_pct = 96;
    nodes[1].id = 1; strcpy(nodes[1].label, "1-A栋"); nodes[1].x_pct = 37; nodes[1].y_pct = 87;
    nodes[2].id = 2; strcpy(nodes[2].label, "1-食堂"); nodes[2].x_pct = 35; nodes[2].y_pct = 66;
    nodes[3].id = 3; strcpy(nodes[3].label, "体育馆"); nodes[3].x_pct = 12; nodes[3].y_pct = 60;
    nodes[4].id = 4; strcpy(nodes[4].label, "学生活动中心"); nodes[4].x_pct = 33; nodes[4].y_pct = 47;
    nodes[5].id = 5; strcpy(nodes[5].label, "体育场看台"); nodes[5].x_pct = 13; nodes[5].y_pct = 45;
    nodes[6].id = 6; strcpy(nodes[6].label, "2-D栋"); nodes[6].x_pct = 12; nodes[6].y_pct = 33;
    nodes[7].id = 7; strcpy(nodes[7].label, "2-C栋"); nodes[7].x_pct = 8; nodes[7].y_pct = 25;
    nodes[8].id = 8; strcpy(nodes[8].label, "2-B栋"); nodes[8].x_pct = 25; nodes[8].y_pct = 11;
    nodes[9].id = 9; strcpy(nodes[9].label, "2-A栋"); nodes[9].x_pct = 15; nodes[9].y_pct = 19;
    nodes[10].id = 10; strcpy(nodes[10].label, "2-食堂"); nodes[10].x_pct = 35; nodes[10].y_pct = 38;
    nodes[11].id = 11; strcpy(nodes[11].label, "3-B栋"); nodes[11].x_pct = 29; nodes[11].y_pct = 6;
    nodes[12].id = 12; strcpy(nodes[12].label, "3-A栋"); nodes[12].x_pct = 44; nodes[12].y_pct = 5;
    nodes[13].id = 13; strcpy(nodes[13].label, "3-食堂"); nodes[13].x_pct = 44; nodes[13].y_pct = 10;
    nodes[14].id = 14; strcpy(nodes[14].label, "图书馆"); nodes[14].x_pct = 45; nodes[14].y_pct = 22;
    nodes[15].id = 15; strcpy(nodes[15].label, "1-教学楼"); nodes[15].x_pct = 40; nodes[15].y_pct = 54;
    nodes[16].id = 16; strcpy(nodes[16].label, "2-教学楼"); nodes[16].x_pct = 71; nodes[16].y_pct = 18;
    nodes[17].id = 17; strcpy(nodes[17].label, "3-教学楼"); nodes[17].x_pct = 53; nodes[17].y_pct = 16;
    nodes[18].id = 18; strcpy(nodes[18].label, "2-实验楼"); nodes[18].x_pct = 70; nodes[18].y_pct = 24;
    nodes[19].id = 19; strcpy(nodes[19].label, "主楼"); nodes[19].x_pct = 78; nodes[19].y_pct = 33;
    nodes[20].id = 20; strcpy(nodes[20].label, "1-实验楼"); nodes[20].x_pct = 90; nodes[20].y_pct = 44;
    nodes[21].id = 21; strcpy(nodes[21].label, "会堂"); nodes[21].x_pct = 62; nodes[21].y_pct = 68;

    // PATHS IN DIJKSTRA
    adj[0][1] = adj[1][0] = 75;     
    adj[1][2] = adj[2][1] = 100;     
    adj[2][3] = adj[3][2] = 75;     
    adj[2][4] = adj[4][2] = 50;     
    adj[3][5] = adj[5][3] = 100;
    adj[5][6] = adj[6][5] = 100;
    adj[6][7] = adj[7][6] = 75;
    adj[4][7] = adj[7][4] = 100;
    adj[4][10] = adj[10][4] = 200;
    adj[7][8] = adj[8][7] = 50;
    adj[8][9] = adj[9][8] = 100;
    adj[9][10] = adj[10][9] = 25;
    adj[8][10] = adj[10][8] = 25;
    adj[10][14] = adj[14][10] = 200;
    adj[10][15] = adj[15][10] = 100;
    adj[14][15] = adj[15][14] = 50;
    adj[8][11] = adj[11][8] = 75;
    adj[11][12] = adj[12][11] = 100;
    adj[12][13] = adj[13][12] = 150;
    adj[13][14] = adj[14][13] = 150;
    adj[14][17] = adj[17][14] = 200;
    adj[17][16] = adj[16][17] = 50;
    adj[16][18] = adj[18][16] = 150;
    adj[14][18] = adj[18][14] = 300;
    adj[18][19] = adj[19][18] = 150;
    adj[14][19] = adj[19][14] = 500;
    adj[19][20] = adj[20][19] = 250;
    adj[19][21] = adj[21][19] = 150;
    adj[21][1] = adj[1][21] = 500;
    adj[15][4] = adj[4][15] = 150;
}
//DIJKSTRA ALGORITHM 一共的距离存在 distance指针
void dijkstra(int start, int end, int *path, int *path_len, int *distance) {
    int n = node_count;
    int dist[MAXN], visited[MAXN], prev[MAXN];
    for(int i=0;i<n;i++){ dist[i] = INF; visited[i]=0; prev[i] = -1; }
    dist[start] = 0;

    for(int t=0;t<n;t++){
        int u = -1, best = INF;
        for(int i=0;i<n;i++){
            if(!visited[i] && dist[i] < best){ best = dist[i]; u = i; }
        }
        if(u == -1) break;
        visited[u] = 1;
        for(int v=0; v<n; v++){
            if(adj[u][v] > 0){
                int alt = dist[u] + adj[u][v];
                if(alt < dist[v]){
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
    }

    if(dist[end] == INF){
        *path_len = 0;
        *distance = -1;
        return;
    }

    int tmp[MAXN];
    int idx = 0;
    for(int v=end; v!=-1; v = prev[v]){
        tmp[idx++] = v;
    }
    // reverse into path
    *path_len = idx;
    for(int i=0;i<idx;i++){
        path[i] = tmp[idx-1-i];
    }
    *distance = dist[end];
}

//以下构建本地HTTP SOCKET的服务，使用ChatGPT生成。

// SEND STRING ON SOCKET
int send_all(int sock, const char *buf, int len) {
    int total = 0;
    while(total < len) {
        int sent = (int)send(sock, buf + total, len - total, 0);
        if(sent <= 0) return -1;
        total += sent;
    }
    return total;
}

// BUILD JSON FOR NODES
void build_nodes_json(char *out, int outcap) {
    int used = 0;
    used += snprintf(out + used, outcap - used, "{ \"nodes\": [");
    for(int i=0;i<node_count;i++){
        used += snprintf(out + used, outcap - used,
            "{\"id\":%d, \"label\":\"%s\", \"x\":%.3f, \"y\":%.3f}%s",
            nodes[i].id, nodes[i].label, nodes[i].x_pct, nodes[i].y_pct,
            (i==node_count-1) ? "" : ",");
        if(used >= outcap-100) break;
    }
    used += snprintf(out + used, outcap - used, "] }");
}

// parse query parameter like start= or end= from buffer
int parse_query(const char *buf, const char *key, int *outval) {
    const char *p = strstr(buf, "?");
    if(!p) p = strstr(buf, " ");
    if(!p) return 0;
    const char *k = strstr(p, key);
    if(!k) return 0;
    // k points to "start=" or "end="
    int val = 0;
    if(sscanf(k + strlen(key), "%d", &val) == 1) {
        *outval = val;
        return 1;
    }
    return 0;
}

// parse path: GET /route?start=1&end=4 HTTP/1.1
int parse_route_params(const char *buf, int *start, int *end) {
    const char *p = strstr(buf, "GET ");
    if(!p) return 0;
    const char *q = strstr(p, " ");
    if(!q) return 0;
    // find start=
    const char *s = strstr(p, "start=");
    const char *e = strstr(p, "end=");
    if(!s || !e) return 0;
    if(sscanf(s + 6, "%d", start) != 1) return 0;
    if(sscanf(e + 4, "%d", end) != 1) return 0;
    return 1;
}

void handle_client(int client_sock) {
    char buffer[RECVBUF+1];
    memset(buffer,0,sizeof(buffer));
    int n = (int)recv(client_sock, buffer, RECVBUF, 0);
    if(n <= 0) return;

    // --- simple routing based on first line ---
    if(strncmp(buffer, "GET /nodes", 10) == 0) {
        char json[8192];
        build_nodes_json(json, sizeof(json));
        char resp[9000];
        int len = snprintf(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            strlen(json), json);
        send_all(client_sock, resp, len);
    }
    else if(strncmp(buffer, "GET /route", 10) == 0) {
        int start = -1, end = -1;
        if(!parse_route_params(buffer, &start, &end)) {
            const char *bad = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nBad route params\n";
            send_all(client_sock, bad, (int)strlen(bad));
        } else {
            if(start < 0 || start >= node_count || end < 0 || end >= node_count) {
                const char *bad = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid node id\n";
                send_all(client_sock, bad, (int)strlen(bad));
            } else {
                int path[MAXN], path_len = 0, dist = 0;
                dijkstra(start, end, path, &path_len, &dist);
                // build JSON: { "distance": X, "path": [id,id,...], "nodes": [{id,label,x,y},...] }
                char jbuf[16384];
                int used = 0;
                used += snprintf(jbuf + used, sizeof(jbuf)-used, "{ \"distance\": %d, \"path\": [", dist);
                for(int i=0;i<path_len;i++){
                    used += snprintf(jbuf + used, sizeof(jbuf)-used, "%d%s", path[i], (i==path_len-1)?"":",");
                }
                used += snprintf(jbuf + used, sizeof(jbuf)-used, "], \"nodes\": [");
                for(int i=0;i<node_count;i++){
                    used += snprintf(jbuf + used, sizeof(jbuf)-used,
                        "{\"id\":%d,\"label\":\"%s\",\"x\":%.3f,\"y\":%.3f}%s",
                        nodes[i].id, nodes[i].label, nodes[i].x_pct, nodes[i].y_pct,
                        (i==node_count-1) ? "" : ",");
                    if(used > (int)sizeof(jbuf)-200) break;
                }
                used += snprintf(jbuf + used, sizeof(jbuf)-used, "] }");

                char resp[16500];
                int len = snprintf(resp, sizeof(resp),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Length: %zu\r\n"
                    "\r\n"
                    "%s",
                    strlen(jbuf), jbuf);
                send_all(client_sock, resp, len);
            }
        }
    }
    else {
        const char *home =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
            "Simple C Dijkstra server\n";
        send_all(client_sock, home, (int)strlen(home));
    }
}

int main() {
    init_graph();

#ifdef _WIN32
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
#endif

    int server_fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
#ifdef _WIN32
    // no setsockopt for SO_REUSEADDR needed typically on Windows
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
#ifdef _WIN32
        closesocket(server_fd);
        WSACleanup();
#else
        close(server_fd);
#endif
        return 1;
    }

    if(listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening on http://localhost:%d\n", PORT);

    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = (int)accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if(client_sock < 0) {
            perror("accept");
            continue;
        }
        handle_client(client_sock);
#ifdef _WIN32
        closesocket(client_sock);
#else
        close(client_sock);
#endif
    }

#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif

    return 0;
}