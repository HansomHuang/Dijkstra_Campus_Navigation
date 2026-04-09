// script.js
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

            // ✅ 关键修复：用 label 显示路径
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