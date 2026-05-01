/*
 * Mini Terminal demo backend
 *
 * Express.js HTTP server. Run with: node server.js
 *
 * Endpoints:
 *   GET  /api/status  -> small JSON with device-style fields
 *   GET  /api/test    -> raw weather API response
 *   GET  /api/echo    -> echoes query params back as JSON
 *   POST /api/log     -> accepts a JSON body, returns {ok:true}
 *
 * Payload constraints (mirrors firmware net_data.h):
 *   - body  <= 512 bytes
 *   - <= 8 top-level fields
 *   - key   <= 23 chars
 *   - value <= 63 chars
 */

const express = require("express");
const net = require("net");
const tls = require("tls");

const PORT = process.env.PORT ? Number(process.env.PORT) : 8080;
const BOOT = Date.now();
const app = express();

const weather_api_key = "SDBSFsQz4uNCF8U81";
const weather_location = "yunfu";
const weather_unit = "c";
const weather_language = "zh-Hans";
const weather_api = `https://api.seniverse.com/v3/weather/now.json?key=${weather_api_key}&location=${weather_location}&language=${weather_language}&unit=${weather_unit}`;
const crypto_api =
  "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum&vs_currencies=usd";
const crypto_proxy =
  process.env.CRYPTO_PROXY ||
  process.env.HTTPS_PROXY ||
  process.env.HTTP_PROXY ||
  "http://127.0.0.1:7897";

function requestTextViaHttpProxy(targetUrl, proxyUrl, timeoutMs = 15000) {
  return new Promise((resolve, reject) => {
    const target = new URL(targetUrl);
    const proxy = new URL(proxyUrl);
    const targetPort = target.port || "443";
    const connectHost = `${target.hostname}:${targetPort}`;
    const socket = net.connect(Number(proxy.port || 8080), proxy.hostname);
    let settled = false;
    let connectBuffer = Buffer.alloc(0);

    function fail(err) {
      if (settled) return;
      settled = true;
      socket.destroy();
      reject(err);
    }

    socket.setTimeout(timeoutMs, () => fail(new Error("proxy connect timeout")));
    socket.on("error", fail);

    socket.on("connect", () => {
      socket.write(
        `CONNECT ${connectHost} HTTP/1.1\r\n` +
          `Host: ${connectHost}\r\n` +
          "Connection: close\r\n" +
          "\r\n",
      );
    });

    socket.on("data", function onConnectData(chunk) {
      const headerEnd = connectBuffer.indexOf("\r\n\r\n");
      connectBuffer = Buffer.concat([connectBuffer, chunk]);
      const end = connectBuffer.indexOf("\r\n\r\n");

      if (headerEnd < 0 && end < 0) return;

      socket.off("data", onConnectData);
      const header = connectBuffer.slice(0, end).toString("latin1");
      if (!header.startsWith("HTTP/1.1 200") && !header.startsWith("HTTP/1.0 200")) {
        fail(new Error(`proxy connect failed: ${header.split("\r\n")[0]}`));
        return;
      }

      const secure = tls.connect({
        socket,
        servername: target.hostname,
      });
      const chunks = [];

      secure.setTimeout(timeoutMs, () => {
        secure.destroy();
        fail(new Error("tls request timeout"));
      });
      secure.on("error", fail);
      secure.on("secureConnect", () => {
        secure.write(
          `GET ${target.pathname}${target.search} HTTP/1.1\r\n` +
            `Host: ${target.hostname}\r\n` +
            "Accept: application/json\r\n" +
            "Accept-Encoding: identity\r\n" +
            "User-Agent: mini-terminal-demo/1.0\r\n" +
            "Connection: close\r\n" +
            "\r\n",
        );
      });
      secure.on("data", (data) => chunks.push(data));
      secure.on("end", () => {
        if (settled) return;
        settled = true;
        const raw = Buffer.concat(chunks).toString("utf8");
        const split = raw.indexOf("\r\n\r\n");
        if (split < 0) {
          reject(new Error("invalid proxy response"));
          return;
        }

        const head = raw.slice(0, split);
        let body = raw.slice(split + 4);
        const status = Number((head.match(/^HTTP\/\d\.\d\s+(\d+)/) || [])[1]);
        if (status < 200 || status >= 300) {
          reject(new Error(`crypto api http ${status}: ${body.slice(0, 120)}`));
          return;
        }

        if (/transfer-encoding:\s*chunked/i.test(head)) {
          body = decodeChunkedBody(body);
        }

        resolve(body);
      });
    });
  });
}

function decodeChunkedBody(body) {
  let pos = 0;
  let out = "";

  for (;;) {
    const lineEnd = body.indexOf("\r\n", pos);
    if (lineEnd < 0) break;

    const sizeText = body.slice(pos, lineEnd).split(";", 1)[0].trim();
    const size = Number.parseInt(sizeText, 16);
    if (!Number.isFinite(size) || size < 0) break;

    pos = lineEnd + 2;
    if (size === 0) break;

    out += body.slice(pos, pos + size);
    pos += size + 2;
  }

  return out;
}

async function fetchWeather() {
  const resp = await fetch(weather_api);

  if (!resp.ok) {
    throw new Error(`weather api http ${resp.status}`);
  }

  const data = await resp.json();

  const result = data.results && data.results[0];
  const location = result && result.location;
  const now = result && result.now;

  if (!location || !now) {
    throw new Error("invalid weather api response");
  }

  return {
    city: weather_location,
    temp: Number(now.temperature),
    weather: String(now.text || ""),
    weather_code: String(now.code || ""),
    updated: String(result.last_update || ""),
  };
}

async function fetchCrypto() {
  let data;

  if (crypto_proxy) {
    data = JSON.parse(await requestTextViaHttpProxy(crypto_api, crypto_proxy));
  } else {
    const resp = await fetch(crypto_api, {
      headers: {
        Accept: "application/json",
        "User-Agent": "mini-terminal-demo/1.0",
      },
    });

    if (!resp.ok) {
      const text = await resp.text();
      throw new Error(`crypto api http ${resp.status}: ${text.slice(0, 120)}`);
    }

    data = await resp.json();
  }

  const bitcoin = data.bitcoin;
  const ethereum = data.ethereum;

  if (!bitcoin || !ethereum) {
    throw new Error("invalid crypto api response");
  }

  return {
    btc_usd: Math.round(Number(bitcoin.usd)),
    eth_usd: Math.round(Number(ethereum.usd)),
  };
}

function buildStatus(weather, crypto) {
  return {
    device: "mini_term",
    city: weather ? weather.city : "unknown",
    temp: weather ? weather.temp : -99,
    weather: weather ? weather.weather : "unknown",
    btc_usd: crypto ? crypto.btc_usd : 0,
    eth_usd: crypto ? crypto.eth_usd : 0,
    online: Boolean(weather),
  };
}

app.disable("x-powered-by");

app.use((req, res, next) => {
  console.log(
    `[${new Date().toISOString()}] ${req.method} ${req.originalUrl} from ${req.ip}`,
  );
  res.set("Connection", "close");
  next();
});

app.use(
  express.text({
    type: "*/*",
    limit: "4kb",
  }),
);

app.get("/api/status", async (req, res) => {
  const [weatherResult, cryptoResult] = await Promise.allSettled([
    fetchWeather(),
    fetchCrypto(),
  ]);

  if (weatherResult.status === "rejected") {
    console.error("weather api failed:", weatherResult.reason.message);
  }

  if (cryptoResult.status === "rejected") {
    console.error("crypto api failed:", cryptoResult.reason.message);
  }

  res.json(
    buildStatus(
      weatherResult.status === "fulfilled" ? weatherResult.value : null,
      cryptoResult.status === "fulfilled" ? cryptoResult.value : null,
    ),
  );
});

app.get("/api/test", async (req, res) => {
  try {
    const weatherRes = await fetch(weather_api);
    const text = await weatherRes.text();

    if (!weatherRes.ok) {
      res.status(weatherRes.status).json({
        ok: false,
        err: "weather_api_http_error",
        status: weatherRes.status,
        body: text,
      });
      return;
    }

    res.type("application/json").send(text);
  } catch (err) {
    console.error("weather api test failed:", err.message);
    res.status(502).json({
      ok: false,
      err: "weather_api_failed",
      message: err.message,
    });
  }
});

app.get("/api/crypto", async (req, res) => {
  try {
    res.json(await fetchCrypto());
  } catch (err) {
    console.error("crypto api test failed:", err);
    res.status(502).json({
      ok: false,
      err: "crypto_api_failed",
      message: err.message,
      cause: err.cause ? String(err.cause) : "",
    });
  }
});

app.get("/api/echo", (req, res) => {
  const out = {};
  let n = 0;

  for (const k of Object.keys(req.query)) {
    if (n++ >= 7) break;
    out[k.slice(0, 23)] = String(req.query[k]).slice(0, 63);
  }

  out.ok = true;
  res.json(out);
});

app.post("/api/log", (req, res) => {
  const body = typeof req.body === "string" ? req.body : "";
  console.log("  body:", body);
  res.json({ ok: true, len: body.length });
});

app.get("/", (req, res) => {
  res.json({ ok: true, hint: "try /api/status" });
});

app.use((req, res) => {
  res.status(404).json({ ok: false, err: "not_found" });
});

app.listen(PORT, "0.0.0.0", () => {
  console.log(`mini_terminal demo server listening on http://0.0.0.0:${PORT}`);
  console.log("endpoints:");
  console.log("  GET  /api/status");
  console.log("  GET  /api/test");
  console.log("  GET  /api/crypto");
  console.log("  GET  /api/echo?foo=bar");
  console.log("  POST /api/log");
});
