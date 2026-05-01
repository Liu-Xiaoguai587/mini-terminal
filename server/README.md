# mini_terminal demo server

Express.js HTTP server that the firmware's `gui_api_demo` page fetches JSON
from.

## Run

```bash
cd server
npm install
node server.js           # listens on 0.0.0.0:8080
PORT=8000 node server.js # custom port
```

## Endpoints

| Method | Path          | Description                                    |
|--------|---------------|------------------------------------------------|
| GET    | `/api/status` | Device-style status JSON (temp, hum, uptime…)  |
| GET    | `/api/echo`   | Echoes query params back as JSON               |
| POST   | `/api/log`    | Accepts a JSON body, returns `{ok, len}`       |

`/api/status` example response (kept under firmware's 512-byte body buffer
and ≤ 8 top-level fields, matching `NET_JSON_MAX_FIELDS`):

```json
{
  "device": "mini_term",
  "ver":    "v0.5",
  "temp":   23.4,
  "hum":    52.7,
  "uptime": 137,
  "online": true,
  "msg":    "hello stm32"
}
```

## Wiring it into the firmware

Edit `App/Gui/gui_api_demo.c` and set `DEMO_HOST` to your PC's LAN IP
(the address the ESP-01S can reach — not `localhost`/`127.0.0.1`):

```c
#define DEMO_HOST  "192.168.1.42"
#define DEMO_PORT  8080
#define DEMO_PATH  "/api/status"
```

Make sure your firewall allows inbound TCP on the chosen port.
