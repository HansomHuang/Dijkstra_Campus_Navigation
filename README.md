# SMBU Campus Navigation System

[中文说明](#中文说明) | [English](#english)

## 中文说明

### 项目简介

这是一个基于校园地图的导航系统，使用 **C 语言**实现后端路径计算服务，使用 **HTML / CSS / JavaScript** 实现前端交互界面。项目核心采用 **Dijkstra 最短路径算法**，用于计算校园内两个地点之间的最短路线，并在地图上可视化展示结果。

该项目是：
- 数据结构与算法课程项目

### 功能特点

- 基于校园节点图的最短路径计算
- 在地图上点击起点和终点进行路线选择
- 在前端画布上绘制导航路径
- 展示路径经过的地点名称与总距离
- 提供校园路径总览图
- 提供部分地点的导览与搜索功能

### 技术栈

- 后端：C
- 算法：Dijkstra
- 通信：本地 HTTP Socket 服务
- 前端：HTML, CSS, JavaScript

### 项目结构

```text
Campus_Navigation/
├─ backend/
│  └─ server.c
├─ frontend/
│  ├─ index.html
│  ├─ script_v2.js
│  ├─ style.css
│  ├─ map.jpg
│  └─ route_overview.jpg
├─ File.md
└─ README.md
```

### 工作原理

1. 后端在 `localhost:8080` 启动一个轻量级 HTTP 服务。
2. 前端请求 `/nodes` 获取校园节点及其地图坐标。
3. 用户在地图上选择起点和终点后，前端请求 `/route?start=<id>&end=<id>`。
4. 后端运行 Dijkstra 算法并返回最短路径、距离和节点信息。
5. 前端根据返回结果绘制路线并显示文字说明。

### 接口说明

#### `GET /nodes`

返回所有地点节点信息，包括：

- `id`
- `label`
- `x`
- `y`

#### `GET /route?start=<id>&end=<id>`

返回：

- `distance`：总距离
- `path`：最短路径对应的节点 ID 列表
- `nodes`：节点元数据

### 运行方式

#### 1. 编译后端

Windows（MinGW-w64）：

```bash
cd backend
gcc server.c -lws2_32 -o server.exe
```

Linux / macOS：

```bash
cd backend
gcc server.c -o server
```

#### 2. 启动后端服务

Windows：

```bash
cd backend
server.exe
```

Linux / macOS：

```bash
cd backend
./server
```

#### 3. 打开前端页面

使用浏览器直接打开 `frontend/index.html`，或使用本地静态服务器打开该页面。

为避免浏览器本地文件策略影响，推荐使用本地静态服务器，例如 VS Code Live Server。

### 后续可改进方向

- 增加更多地点介绍内容
- 支持多语言界面
- 支持路径权重动态调整
- 增加后台数据配置文件，而不是将图结构写死在代码中
- 提供部署版 API 或桌面端封装

---

## English

### Overview

This project is a campus navigation system built on top of a campus map. It uses **C** for the backend route computation service and **HTML / CSS / JavaScript** for the frontend interface. The core algorithm is **Dijkstra's shortest path algorithm**, which computes the shortest route between two campus locations and visualizes it on the map.

This project is:
- a data structures and algorithms course project

### Features

- Shortest path calculation on a campus graph
- Interactive start/end point selection on the map
- Route visualization on the frontend canvas
- Distance and route label display
- Campus route overview image
- Landmark guide and search panel

### Tech Stack

- Backend: C
- Algorithm: Dijkstra
- Communication: local HTTP socket server
- Frontend: HTML, CSS, JavaScript

### Project Structure

```text
Campus_Navigation/
├─ backend/
│  └─ server.c
├─ frontend/
│  ├─ index.html
│  ├─ script_v2.js
│  ├─ style.css
│  ├─ map.jpg
│  └─ route_overview.jpg
├─ File.md
└─ README.md
```

### How It Works

1. The backend starts a lightweight HTTP service on `localhost:8080`.
2. The frontend requests `/nodes` to get campus node data and map coordinates.
3. After the user selects a start point and an end point, the frontend calls `/route?start=<id>&end=<id>`.
4. The backend runs Dijkstra's algorithm and returns the shortest path, total distance, and node metadata.
5. The frontend draws the path on the map and displays textual route information.

### API Endpoints

#### `GET /nodes`

Returns all campus nodes, including:

- `id`
- `label`
- `x`
- `y`

#### `GET /route?start=<id>&end=<id>`

Returns:

- `distance`: total route distance
- `path`: list of node IDs in the shortest path
- `nodes`: node metadata

### How to Run

#### 1. Compile the backend

Windows (MinGW-w64):

```bash
cd backend
gcc server.c -lws2_32 -o server.exe
```

Linux / macOS:

```bash
cd backend
gcc server.c -o server
```

#### 2. Start the backend server

Windows:

```bash
cd backend
server.exe
```

Linux / macOS:

```bash
cd backend
./server
```

#### 3. Open the frontend

Open `frontend/index.html` in a browser, or serve the `frontend` directory with a local static server.

To avoid browser restrictions related to local files, using a local static server such as VS Code Live Server is recommended.

### Possible Improvements

- Add richer landmark descriptions
- Support multilingual UI
- Support dynamic edge weights
- Move graph data out of hardcoded source code into configurable files
- Provide a deployable API or desktop packaging
