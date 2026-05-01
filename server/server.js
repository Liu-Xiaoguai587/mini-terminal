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
  const resp = await fetch(crypto_api);

  if (!resp.ok) {
    throw new Error(`crypto api http ${resp.status}`);
  }

  const data = await resp.json();
  const bitcoin = data.bitcoin;
  const ethereum = data.ethereum;

  if (!bitcoin || !ethereum) {
    throw new Error("invalid crypto api response");
  }

  return {
    btc_usd: Number(bitcoin.usd),
    eth_usd: Number(ethereum.usd),
  };
}

function buildStatus(weather, crypto) {
  return {
    device: "mini_term",
    city: weather.city,
    temp: weather.temp,
    btc_usd: crypto.btc_usd,
    eth_usd: crypto.eth_usd,
    online: true,
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
  try {
    const [weather, crypto] = await Promise.all([
      fetchWeather(),
      fetchCrypto(),
    ]);
    res.json(buildStatus(weather, crypto));
  } catch (err) {
    console.error("status api failed:", err.message);
    res.status(502).json({
      ok: false,
      err: "status_failed",
      message: err.message,
    });
  }
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
  console.log("  GET  /api/echo?foo=bar");
  console.log("  POST /api/log");
});
