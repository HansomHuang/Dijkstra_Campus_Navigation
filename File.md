##### SMBU 校园导航系统
---
- 算法：Dijkstra
- 2025年11月20日
---
- C + HTML
>核心算法Server.c（C语言）中使用Dijkstra算法来计算任意两点的最短路径。
>用户界面使用Index.html构建，script_v2.js和style.css来创建交互和外观效果。
---
###### 代码目录：
- Server.c
```c
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

// Node 结构
typedef struct {
    int id;
    char label[64];
    // 按钮位置于图中，以百分比计量 (0..100)
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

    // 定义点的编号/标签/于图中的位置（x/y）
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

    // Dijkstra路径权重和边，权值存储于二维数组中
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
// 从 start 节点到 end 节点，计算最短路径
// 输入：
//   start: 起点节点编号（0~21）
//   end:   终点节点编号
// 输出：
//   path[]: 存储路径上的节点编号/路径经由的点
//   *path_len: 路径包含多少个节点
//   *distance: 最短的总距离
void dijkstra(int start, int end, int *path, int *path_len, int *distance) {
    int n = node_count;
int dist[MAXN], visited[MAXN], prev[MAXN];
//dist[i]：从起点到节点 i 的当前已知最短距离
//visited[i]：节点 i 是否已被确定最短路径（1=是，0=否）
//prev[i]：在最短路径中，节点 i 的前一个节点是谁
    for(int i=0;i<n;i++){ dist[i] = INF; visited[i]=0; prev[i] = -1; }
    dist[start] = 0;
//初始时，所有距离设为无穷大（表示不可达），所有节点都未访问，没有前驱结点。
    for(int t=0;t<n;t++){   //在未访问的节点中，找 dist 最小的
        int u = -1, best = INF;
        for(int i=0;i<n;i++){
            if(!visited[i] && dist[i] < best){ best = dist[i]; u = i; } //u是当前处理节点

        }
        if(u == -1) break; //如果找不到可处理节点，说明剩余节点都不可达，结束
        visited[u] = 1;  //标记 u 为已访问，最短距离确定
        for(int v=0; v<n; v++){   //用 u 去更新它所有邻居 v 的距离
            if(adj[u][v] > 0){  //如果 u 和 v 之间有边
                int alt = dist[u] + adj[u][v];  //u 到 v 的新距离
                if(alt < dist[v]){
                    dist[v] = alt;  //更新最短距离
                    prev[v] = u;
                }
            }
        }
    }

    if(dist[end] == INF){
        *path_len = 0;
        *distance = -1;  //距离为 -1 表示不可达
        return;
    }

    int tmp[MAXN]; //从终点回溯到起点，沿着prev数组中存储的信息
    int idx = 0;  
    for(int v=end; v!=-1; v = prev[v]){  
        tmp[idx++] = v;
    }  //从 end 开始，沿着 prev[] 一路回到 start，tmp数组存储为路径的反向
    //将反向路径反转，存入输出参数 path数组
    *path_len = idx;
    for(int i=0;i<idx;i++){
        path[i] = tmp[idx-1-i];
    }
    *distance = dist[end];  //输出最短路径
}

//以下构建本地HTTP SOCKET服务部分，使用ChatGPT生成。
//用于将结果部署于localhost接口以使html调用c函数，以html呈现。

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
```
- Index.html (借助Qwen构建)
```html
<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>SMBU 校园导航系统</title>
  <link rel="stylesheet" href="style.css">
</head>
<body>
  <!-- 顶部 TabBar -->
  <header class="tabbar">
    <h1>SMBU 校园导航系统</h1>
    <nav class="tabs">
      <button class="tab-btn active" data-tab="navigation">导航</button>
      <button class="tab-btn" data-tab="route-image">路径</button>
      <button class="tab-btn" data-tab="guide">导览</button>
    </nav>
  </header>

  <!-- 主内容区 -->
  <main id="app">
    <!-- 导航 Tab -->
    <section id="navigation" class="tab-content active">
      <div id="map-wrapper">
        <img id="map-img" src="map.jpg" alt="Campus Map">
        <div id="overlay"></div>
        <canvas id="path-canvas"></canvas>
      </div>

      <div class="controls-section">
        <div class="info-row">
          <div class="info-item"><strong>起点:</strong> <span id="start-label">请选择</span></div>
          <div class="info-item"><strong>终点:</strong> <span id="end-label">请选择</span></div>
        </div>
        <div class="button-row">
          <button id="clear-btn" class="btn">清除</button>
          <button id="route-btn" class="btn primary">计算路径</button>
        </div>
      </div>
      <pre id="output" class="output-box"></pre>
      <p class="hint-text">点击地图上的按钮以选择起点/终点。</p>
      <p class="hint-text">黄博林 2025/11/20</p>
    </section>

<!--路径-->
    <section id="route-image" class="tab-content">
      <img src="route_overview.jpg" alt="校园路径总览图" class="full-image">
      <p class="image-caption">校园主要路径示意图</p>
    </section>
<!--导览-->
    <section id="guide" class="tab-content">
      <div class="guide-container">
        <aside class="guide-sidebar">
          <input type="text" id="search-input" placeholder="检索位置" class="search-box">
          <ul id="landmark-list" class="landmark-list">
            <li data-id="0">1-B栋</li>
            <li data-id="1">1-A栋</li>
            <li data-id="2">1-食堂</li>
            <li data-id="3">体育馆</li>
            <li data-id="4">学生活动中心</li>
            <li data-id="5">体育场看台</li>
            <li data-id="6">2-D栋</li>
            <li data-id="7">2-C栋</li>
            <li data-id="8">2-B栋</li>
            <li data-id="9">2-A栋</li>
            <li data-id="10">2-食堂</li>
            <li data-id="11">3-B栋</li>
            <li data-id="12">3-A栋</li>
            <li data-id="13">3-食堂</li>
            <li data-id="14">图书馆</li>
            <li data-id="15">1-教学楼</li>
            <li data-id="16">2-教学楼</li>
            <li data-id="17">3-教学楼</li>
            <li data-id="18">2-实验楼</li>
            <li data-id="19">主楼</li>
            <li data-id="20">1-实验楼</li>
            <li data-id="21">会堂</li>
          </ul>
        </aside>
        <article class="guide-detail" id="landmark-detail">
          <h2>SMBU 导览</h2>
          <p>选择地点以查看详细信息。</p>
        </article>
      </div>
    </section>
  </main>
  <template id="detail-0">
  <h2>1-B栋</h2>
  <p>1号学生宿舍B栋，四人间，配备独立卫浴，靠近1号食堂。</p>
  <p><strong>位置：</strong>校园西北角</p>
</template>
<template id="detail-1">
  <h2>1-A栋</h2>
  <p>1号学生宿舍A栋，与B栋结构相同，主要供女生入住。</p>
</template>
<template id="detail-2">
  <h2>1-食堂</h2>
  <p>提供面食，现炒/预制盖饭，麻辣烫，俄式/美式餐食。</p>
</template>
<template id="detail-3">
  <h2>体育馆</h2>
  <p>下层为标准室内泳池，上层为室内篮球/羽毛球场。</p>
</template>
<template id="detail-4">
  <h2>学生活动中心</h2>
  <p>社团根据地。提供美术/摄影/舞蹈/社交场所。</p>
</template>
<template id="detail-5">
  <h2>体育场看台</h2>
  <p>没有空调。</p>
</template>
<template id="detail-21">
  <h2>会堂</h2>
  <p>可容纳近千人，用于开学典礼、文艺汇演等大型活动。</p>
</template>
  <script src="script_v2.js"></script>
</body>
</html>
```
- Script_v2.js（借助Qwen构建）
```js
// script_v2.js
// 前端逻辑：读取 /nodes，渲染按钮；点击按钮选择 start/end；点击计算向 /route 请求并绘制路径。

const API_BASE = "http://localhost:8080";
let nodes = []; // array of {id,label,x,y}
let startId = null;
let endId = null;

const overlay = document.getElementById("overlay");
const pathCanvas = document.getElementById("path-canvas");
const mapImg = document.getElementById("map-img");
const startLabel = document.getElementById("start-label");
const endLabel = document.getElementById("end-label");
const output = document.getElementById("output");
const routeBtn = document.getElementById("route-btn");
const clearBtn = document.getElementById("clear-btn");

// ensure canvas size matches image
function resizeCanvas() {
    const rect = mapImg.getBoundingClientRect();
    pathCanvas.width = rect.width;
    pathCanvas.height = rect.height;
    pathCanvas.style.width = rect.width + "px";
    pathCanvas.style.height = rect.height + "px";
    pathCanvas.style.left = mapImg.offsetLeft + "px";
    pathCanvas.style.top = mapImg.offsetTop + "px";
}
window.addEventListener('resize', resizeCanvas);
mapImg.addEventListener('load', () => {
    renderNodeButtons();
    resizeCanvas();
    drawPath([]); // clear
});

// fetch nodes from backend
async function loadNodes() {
    try {
        const res = await fetch(API_BASE + "/nodes");
        const j = await res.json();
        nodes = j.nodes || [];
    } catch(err) {
        console.error("无法从后端获取节点：", err);
        output.textContent = "无法连接到后端，请先启动 C 服务（http://localhost:8080）";
    }
}

// render node buttons according to image size and percentage coordinates
function renderNodeButtons() {
    // remove existing buttons
    const old = document.querySelectorAll('.node-btn');
    old.forEach(e => e.remove());

    nodes.forEach(n => {
        const btn = document.createElement('button');
        btn.className = 'node-btn';
        btn.textContent = n.label;          // ← 显示 label 而不是 id
        btn.dataset.id = n.id;              // ← 用 data-id 存储 id
        btn.title = `${n.label} (ID: ${n.id})`; // 可选：鼠标悬停显示 ID

        // 使用百分比定位（相对于 #map-wrapper）
        btn.style.left = n.x + '%';
        btn.style.top = n.y + '%';

        btn.addEventListener('click', (ev) => {
            ev.stopPropagation();
            const clickedId = parseInt(btn.dataset.id, 10);
            // choose start if not set, else choose end; toggle selection if clicking same
            if(startId === null || (startId !== null && endId !== null)) {
                // set/start fresh
                startId = clickedId;
                endId = null;
            } else if(startId !== null && endId === null) {
                if(clickedId === startId) {
                    // unselect start
                    startId = null;
                } else {
                    endId = clickedId;
                }
            }
            updateSelectionStyles();
            updateLabels();
        });

        document.getElementById('map-wrapper').appendChild(btn);
    });
}

// updates styles of buttons based on startId/endId
function updateSelectionStyles() {
    const btns = document.querySelectorAll('.node-btn');
    btns.forEach(b => {
        b.classList.remove('selected-start');
        b.classList.remove('selected-end');
        const id = parseInt(b.dataset.id, 10); // ← 从 data-id 获取 id
        if(startId !== null && id === startId) b.classList.add('selected-start');
        if(endId !== null && id === endId) b.classList.add('selected-end');
    });
}

function updateLabels() {
    const startNode = nodes.find(n => n.id === startId);
    const endNode = nodes.find(n => n.id === endId);
    startLabel.textContent = startNode ? startNode.label : '—';
    endLabel.textContent = endNode ? endNode.label : '—';
}

clearBtn.addEventListener('click', () => {
    startId = null; endId = null;
    updateSelectionStyles();
    updateLabels();
    drawPath([]);
    output.textContent = '';
});

routeBtn.addEventListener('click', async () => {
    if(startId === null || endId === null) {
        alert("请先点击按钮以选择起点/终点。");
        return;
    }
    try {
        const res = await fetch(`${API_BASE}/route?start=${startId}&end=${endId}`);
        if(!res.ok) {
            const txt = await res.text();
            output.textContent = "后端返回错误: " + txt;
            return;
        }
        const j = await res.json();
        if(Array.isArray(j.path) && j.path.length > 0) {
            // map node ids to coordinates and draw polyline
            const coords = j.path.map(id => {
                const node = nodes.find(n => n.id === id);
                if (!node) {
                    console.warn("节点未找到:", id);
                    return { x: 0, y: 0 };
                }
                const rect = mapImg.getBoundingClientRect();
                return {
                    x: (node.x / 100) * rect.width,
                    y: (node.y / 100) * rect.height
                };
            });
            drawPath(coords);

            // 用 label 显示路径
            const pathLabels = j.path.map(id => {
                const node = nodes.find(n => n.id === id);
                return node ? node.label : `未知(${id})`;
            });
            output.textContent = `距离: ${j.distance} 米\n路径: ${pathLabels.join(' → ')}`;
        } else {
            output.textContent = "未找到路径";
            drawPath([]);
        }
    } catch(err) {
        console.error(err);
        output.textContent = "请求错误：" + err.message;
    }
});

// draw polyline on canvas (coords in pixel coordinates relative to image top-left)
function drawPath(coords) {
    const canvas = pathCanvas;
    const ctx = canvas.getContext('2d');
    // size it to image
    const rect = mapImg.getBoundingClientRect();
    canvas.width = rect.width;
    canvas.height = rect.height;

    ctx.clearRect(0,0,canvas.width, canvas.height);
    if(!coords || coords.length === 0) return;
    ctx.beginPath();
    ctx.lineWidth = 4;
    ctx.strokeStyle = "rgba(255,0,0,0.9)";
    // draw small circles at nodes and connecting lines
    for(let i=0;i<coords.length;i++){
        const c = coords[i];
        if(i===0) ctx.moveTo(c.x, c.y);
        else ctx.lineTo(c.x, c.y);
    }
    ctx.stroke();

    // draw nodes
    for(let i=0;i<coords.length;i++){
        const c = coords[i];
        ctx.beginPath();
        ctx.fillStyle = "rgba(255,255,255,0.95)";
        ctx.strokeStyle = "rgba(255,0,0,0.9)";
        ctx.lineWidth = 2;
        ctx.arc(c.x, c.y, 8, 0, Math.PI*2);
        ctx.fill();
        ctx.stroke();
    }
}

// initial load
(async function(){
    await loadNodes();
    // make sure image loaded (if cached)
    if(mapImg.complete) {
        renderNodeButtons();
        resizeCanvas();
    }
})();

// ========== Tab 切换 ==========
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
        btn.classList.add('active');
        document.getElementById(btn.dataset.tab).classList.add('active');
    });
});

// script_v2.js（底部追加）
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
        btn.classList.add('active');
        const tabId = btn.dataset.tab;
        document.getElementById(tabId).classList.add('active');
    });
});

// 导览：点击景点显示详情
document.getElementById('landmark-list')?.addEventListener('click', (e) => {
  if (e.target.tagName === 'LI') {
    const id = e.target.dataset.id;

    // 高亮选中项
    document.querySelectorAll('#landmark-list li').forEach(li => {
      li.classList.remove('active');
    });
    e.target.classList.add('active');

    // 获取详情内容
    const template = document.getElementById(`detail-${id}`);
    const detailContainer = document.getElementById('landmark-detail');

    if (template && detailContainer) {
      detailContainer.innerHTML = template.innerHTML;
    } else {
      detailContainer.innerHTML = '<h2>暂无介绍</h2><p>该位置暂无详细信息。</p>';
    }
  }
});

// 搜索功能（保持不变）
const searchInput = document.getElementById('search-input');
if (searchInput) {
  searchInput.addEventListener('input', () => {
    const query = searchInput.value.trim().toLowerCase();
    document.querySelectorAll('#landmark-list li').forEach(li => {
      const text = li.textContent.toLowerCase();
      li.style.display = text.includes(query) ? 'block' : 'none';
    });
  });
}
```
- Style.css
```css
/* style.css */
* {
  box-sizing: border-box;
}

body {
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "PingFang SC", sans-serif;
  background-color: #f9fafb;
  color: #333;
  line-height: 1.6;
}

/* ===== TabBar ===== */
.tabbar {
  background: #1e5799;
  color: white;
  padding: 12px 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12px;
  position: sticky;
  top: 0;
  z-index: 100;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  width: 768px;           /* 限定宽度 */
  margin: 0 auto;         /* 居中对齐 */
  border-radius: 8px;    /* 四角圆角 */
  overflow: hidden;       /* 防止内容溢出 */
}
.tabbar h1 {
  margin: 0;
  font-size: 1.6rem;
  text-align: center;
}
.tabs {
  display: flex;
  gap: 16px;
  justify-content: center;
  padding: 0 16px; /* 内边距，避免按钮贴边 */
}
.tab-btn {
  padding: 8px 16px;
  background: rgba(255,255,255,0.2);
  border: none;
  border-radius: 6px;
  color: white;
  cursor: pointer;
  font-weight: bold;
  transition: background 0.2s;
}
.tab-btn:hover {
  background: rgba(255,255,255,0.3);
}
.tab-btn.active {
  background: white;
  color: #1e5799;
}

/* ===== 主内容区 ===== */
#app {
  max-width: 800px;
  margin: 0 auto;
  padding: 16px;
}

/* 隐藏非激活 Tab */
.tab-content {
  display: none;
}
.tab-content.active {
  display: block;
}

/* ===== 导航 Tab ===== */
#map-wrapper {
  position: relative;
  width: 100%;
  margin-bottom: 20px;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
}
#map-img {
  width: 100%;
  display: block;
}
#path-canvas {
  position: absolute;
  top: 0;
  left: 0;
  pointer-events: none;
}

.node-btn {
  position: absolute;
  transform: translate(-50%, -50%);
  z-index: 10; /* 必须高于 canvas 和 overlay */
  pointer-events: auto;
  background: rgba(255,255,255,0.95);
  border: 1px solid #1976d2;
  border-radius: 0;
  width: 100px;
  height: 25px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: normal;
  cursor: pointer;
  box-shadow: 0 2px 6px rgba(0,0,0,0.2);
  font-size: 0.8rem;
  user-select: none;
}
.node-btn.selected-start {
  background: #1976d2;
  color: white;
}

.node-btn.selected-end {
  background: #d32f2f;
  color: white;
}
.controls-section {
  background: white;
  padding: 16px;
  border-radius: 8px;
  margin-bottom: 16px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.1);
}
.info-row {
  display: flex;
  justify-content: space-between;
  margin-bottom: 12px;
}
.button-row {
  display: flex;
  gap: 12px;
  justify-content: center;
}
.btn {
  padding: 8px 20px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
}
.primary { background: #007bff; color: white; }
.btn:hover { opacity: 0.9; }

.output-box {
  background: #f1f1f1;
  padding: 12px;
  border-radius: 6px;
  white-space: pre-wrap;
  margin-bottom: 12px;
}

.hint-text {
  text-align: center;
  color: #666;
  font-size: 14px;
}

/* ===== 路径 Tab ===== */
.full-image {
  width: 100%;
  height: auto;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
}
.image-caption {
  text-align: center;
  color: #666;
  margin-top: 8px;
}

/* ===== 导览 Tab ===== */
.guide-container {
  display: flex;
  gap: 24px;
  height: 600px;
  background: white;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
}
.guide-sidebar {
  width: 250px;
  padding: 16px;
  background: #f8f9fa;
  border-right: 1px solid #ddd;
  display: flex;
  flex-direction: column;
  max-height: 100%;
  overflow-y: auto; /* 关键：允许垂直滚动 */
}
.search-box {
  width: 100%;
  padding: 8px;
  margin-bottom: 12px;
  border: 1px solid #ddd;
  border-radius: 4px;
}
.landmark-list {
  list-style: none;
  padding: 0;
  margin: 0;
  flex: 1; /* 占据剩余空间 */
  overflow-y: auto; /* 备用：确保列表可滚 */
}
.landmark-list li {
  padding: 10px;
  cursor: pointer;
  border-radius: 4px;
  transition: background 0.2s;
}
.landmark-list li:hover {
  background: #e3f2fd;
}
.landmark-list li.active {
  background: #bbdefb;
  font-weight: bold;
}

.guide-detail {
  flex: 1;
  padding: 24px;
  overflow-y: auto;
}
.guide-detail h2 {
  margin-top: 0;
  color: #1e5799;
}
```