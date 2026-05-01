# ESP01S HTTP API Demo 调试记录

日期：2026-05-01

范围：

- `server/server.js`
- `Drivers/Peripherals/ESP01S.c`
- `App/Tasks/task_wifi.c`
- `App/Logic/net_data.c`
- `App/Gui/gui_api_demo.c`
- `App/Tasks/task_init.c`

## 1. 问题：API Demo 请求本地 Node 后端返回 `Failed (-1)`

现象：

- 单片机 API Demo 页面显示 `Failed (-1)`。
- Node 服务端能看到日志：`GET /api/status from 192.168.31.251`。
- 说明请求已经到达服务端，ESP-01S 到 PC 的 TCP/HTTP 发送方向是通的。

分析可能的问题：

- 最初怀疑是 HTTP 响应解析失败，`parse_http_response()` 没找到 `\r\n\r\n`。
- `task_wifi.c` 当 `resp.body == NULL` 时把结果写成 `-1`，导致 GUI 看不到真实状态。
- 也怀疑 ESP-AT 的 `+IPD` 分片、`CIPDINFO` 格式变化、主动/被动接收模式等导致响应没有被上层拿到。

对应的解决方式：

- `task_wifi.c` 改成 `ESP_HTTP_GET()` 返回 `ESP_OK` 时就透传 `resp.status_code/body/body_len`，不再因为 `body == NULL` 直接覆盖成 `-1`。
- `ESP01S.c` 初始化时增加：
  - `AT+CIPMODE=0`
  - `AT+CIPRECVMODE=0`
  - `AT+CIPDINFO=0`
- `parse_http_response()` 找不到 header/body 分隔符时，先透出 raw buffer 作为诊断。
- GUI 失败分支显示 `recv/stage/ret` 等诊断信息。

实际结果：

- 仍然出现 `Failed (-1) (no body)`，说明问题不只是 HTTP header/body 分隔符。

## 2. 问题：服务端有新日志，但单片机仍然 `Failed (-1) (no body)`

现象：

- 每次按键，服务端确实新增 `GET /api/status` 日志。
- 单片机仍显示 `-1` 或无 body。

分析：

- 进一步怀疑 `ESP_TCP_Send()` 等待 `SEND OK` 时，快速局域网响应已经提前到达。
- 如果 `+IPD,...` 在 `SEND OK` 前进入串口缓冲，原来的逐行等待逻辑可能把 `+IPD` 当无关行丢掉。
- 后续 `recv_ipd_data()` 再读时，HTTP 数据已经没有对应的 `+IPD` 前缀，导致收包长度为 0。

解决：

- 先尝试缓存提前出现的 `+IPD` 行，再交给 `recv_ipd_data()`。
- 后来发现不同 ESP-AT 固件输出顺序差异很大，继续维护逐行状态机风险较高。
- 最终把 `recv_ipd_data()` 改成 raw-byte slurp：
  - 发送 HTTP 请求后，不再严格逐行解析 `+IPD`。
  - 从 UART 持续读取原始字节到缓冲区。
  - 看到 `CLOSED\r\n` 或 HTTP 响应后出现空闲间隔就停止。
  - `parse_http_response()` 再从 raw buffer 中查找 `HTTP/`。

实际结果：

- 单片机开始能收到响应，但后续出现长时间 `Requesting...`。

## 3. 问题：页面一直显示 `Requesting...`

现象：

- 服务端有新请求日志。
- API Demo 页面长时间停在 `Requesting...`。

分析：

- `recv_ipd_data()` 绝对超时较长，单次请求可能等待太久。
- LVGL 的 encoder 交互可能同时触发 `LV_EVENT_KEY` 和 `LV_EVENT_CLICKED`，导致一次操作入队多个 HTTP 命令。
- `send_request()` 没有 busy 守卫，用户重复按键或双事件会让 WiFi task 串行处理多次请求，看起来像卡住。

解决：

- `gui_api_demo.c` 增加请求守卫：
  - 请求进行中不再重复入队。
  - 使用 `s_poll_active` 和 `nd.http_busy` 判断是否已有请求。
- `ESP_HTTP_GET()` 的接收绝对超时从 15s 降到 5s。
- `recv_ipd_data()` idle 等待缩短到 150ms，并且在看到 `HTTP/` 前不因为早期 AT 噪声的空闲间隔提前退出。
- `ESP_TCP_Send()` 改成拿到 `>` prompt 并发送数据后立即返回，不再消耗后续 `SEND OK/+IPD/HTTP` 输出。

实际结果：

- 不再长期卡在 `Requesting...`。
- 页面显示 `HTTP 200 recv:288 /api/status`，说明请求和响应链路已跑通。

## 4. 问题：成功后页面再次卡死，快速退出则没事

现象：

- 页面等到响应回来后卡死。
- 如果进入 API Demo 后快速退出，则系统没事。

分析：

- 现象指向 LVGL 成功显示路径，而不是 ESP/TCP。
- `task_lvgl` 原栈大小为 768 words。
- 成功分支曾调用 `set_json_result_text()`，该函数在 LVGL timer 回调中通过 `net_data_get_json()` 复制整个 `NetJsonData_t` 到栈上。
- `NetJsonData_t` 包含最多 8 个字段，每个字段有 key/value/double/type，结构体接近 1KB；再叠加 `NetData_t`、`char text[192]`、LVGL 调用栈，容易导致 LVGL task 栈溢出。

解决：

- `gui_api_demo.c` 成功分支不再调用 `net_data_get_json()` 和 `set_json_result_text()`。
- 成功后直接显示 `nd.last_http_body` 原始 JSON，降低 LVGL task 栈开销。
- `task_init.c` 将 `task_lvgl` 栈从 768 words 增加到 1024 words。
- `task_wifi` 栈也保留为 2048 words，以适配 HTTP buffer、ESP-AT 收包和 cJSON 相关调用。

实际结果：

- 页面不再卡死。
- 显示内容变为 `HTTP 200 recv:289` 加原始接收内容。

## 5. 问题：页面显示 `SEND OK +IPD + HTTP headers + JSON`，不是纯 JSON

现象：

- 页面显示类似：
  - `HTTP 200 recv:289`
  - `SEND OK`
  - `+IPD,249:HTTP/1.1 200 OK`
  - HTTP 响应头：数据格式、字符编码、长度、连接状态、日期、时间
  - 最后才是 JSON body

分析：

- 这说明 TCP/HTTP 链路已经成功。
- 当前 raw-byte slurp 会把 ESP-AT 噪声、`SEND OK`、`+IPD`、HTTP header、body 一起收到缓冲区。
- `parse_http_response()` 已能识别 `HTTP 200`，但 body 定位不够稳。
- 有可能 raw buffer 中的换行不是标准 `\r\n\r\n`，或者显示/固件路径把换行规格改变，导致 body fallback 回到 raw buffer。

解决：

- 完善 `parse_http_response()`：
  - 从 `HTTP/` 位置开始查找，不受前面 `SEND OK/+IPD` 噪声影响。
  - 优先查找标准 `\r\n\r\n` header/body 分隔符。
  - 兼容 `\n\n` 分隔符。
  - 如果仍找不到分隔符，就从 HTTP 段里直接找 JSON 起始符 `{` 或 `[`。
  - 找到 `Content-Length:` 时按声明长度截断，避免尾部混入 `CLOSED` 或其他 AT 文本。

预期结果：

- API Demo 顶部显示：
  - `HTTP 200 recv:xxx`
- 下方只显示 JSON body：
  - `{"device":"mini_term","ver":"v0.5",...}`

## 6. 当前结论

已经确认成功的部分：

- STM32 能通过 ESP-01S 访问 PC 上的 Node 服务端。
- 服务端能收到来自单片机 IP 的 `GET /api/status`。
- ESP-01S 能把 HTTP 响应回传给 STM32。
- 单片机能解析出 HTTP 200。

本次实际有效的关键改动：

- 服务端使用 Node 内置 HTTP，明确设置 `Content-Length` 和 `Connection: close`。
- ESP 初始化固定为普通 TCP、主动接收、简化 `+IPD` 信息。
- HTTP GET 前防御性 `CIPCLOSE`，避免上次 socket 残留。
- `ESP_TCP_Send()` 只等 `>` prompt，不再抢读后续响应。
- `recv_ipd_data()` 改为 raw-byte 收包，再由 `parse_http_response()` 查找 HTTP 响应。
- GUI 请求加 busy/poll 守卫，避免一次点击入队多个请求。
- GUI 成功分支避免在 LVGL task 栈上复制大 JSON 结构体。
- LVGL task 栈从 768 words 增加到 1024 words。

后续建议：

- 如果需要字段化显示 JSON，不要在 GUI 回调里复制 `NetJsonData_t`。
- 更稳的方式是在 `net_data.c` 中维护一个静态显示文本缓冲，例如 `s_json_display_text[192]`，由 WiFi task 解析 HTTP body 时生成；GUI 只读取并显示该小字符串。
- 等纯 JSON body 显示稳定后，再考虑把诊断信息 `g_wifi_stage/g_esp_http_stage/g_esp_recv_len` 收敛到 debug 宏下。
