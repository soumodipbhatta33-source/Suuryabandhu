// mock-server.js
// A zero-dependency stand-in for your IoT controller/backend.
// Run:  node mock-server.js
// Serves the exact contract the Suryabandhu dashboard expects when
// USE_LIVE_DATA = true, so you can test the integration before real
// meters/ESP32 nodes are wired up.
//
// GET /api/sites/:id/live  ->
// {
//   "siteId": "lakeview",
//   "timestamp": "2026-09-27T10:42:03.123Z",
//   "solarNowKw": 2.143,
//   "flats": [
//     { "flat": "3A", "loadW": 812.4, "fault": false },
//     { "flat": "3B", "loadW": 265.0, "fault": false },
//     ...
//   ]
// }
//
// Swap this file's random-number generation for real sensor reads
// (e.g. Modbus/MQTT ingestion into these same fields) and the
// dashboard needs zero changes on the frontend side.

const http = require('http');

const SITES = {
  lakeview:   { flats: ['3A','3B','4C','5A','6D','7B','8A','2C'], base: 280, solarPeakKw: 2.6 },
  bardhaman:  { flats: ['1A','1B','2A','2B','3C','3D','4A','4B'], base: 260, solarPeakKw: 1.8 },
  sundarbans: { flats: ['A1','A2','A3','A4','B1','B2','B3','B4'], base: 300, solarPeakKw: 3.2 },
};

// Fault state persisted per flat across requests so faults last a few polls,
// mirroring how a real fault condition would.
const faultState = {};

function readingsFor(siteId) {
  const site = SITES[siteId];
  if (!site) return null;

  const t = Date.now();
  const solarNowKw = Math.max(0, site.solarPeakKw * (0.55 + 0.35 * Math.sin(t/45000)) + (Math.random()*0.15 - 0.075));

  const flats = site.flats.map(flat => {
    const key = `${siteId}:${flat}`;
    if (faultState[key] === undefined) faultState[key] = { fault: false, ticks: 0 };
    const fs = faultState[key];

    let loadW;
    if (fs.fault) {
      fs.ticks++;
      loadW = site.base * 0.35 + (Math.random()*20 - 10);
      if (fs.ticks > 8) { fs.fault = false; fs.ticks = 0; }
    } else {
      loadW = site.base + Math.sin(t/4000 + site.base) * 40 + (Math.random()*24 - 12);
      if (Math.random() < 0.01) { fs.fault = true; fs.ticks = 0; }
    }

    return { flat, loadW: Math.max(0, Math.round(loadW * 10) / 10), fault: fs.fault };
  });

  return { siteId, timestamp: new Date(t).toISOString(), solarNowKw: Math.round(solarNowKw * 1000) / 1000, flats };
}

const server = http.createServer((req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');

  const match = req.url.match(/^\/api\/sites\/([\w-]+)\/live$/);
  if (req.method === 'GET' && match) {
    const data = readingsFor(match[1]);
    if (!data) {
      res.writeHead(404, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ error: `Unknown siteId '${match[1]}'` }));
      return;
    }
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify(data));
    return;
  }

  res.writeHead(404, { 'Content-Type': 'application/json' });
  res.end(JSON.stringify({ error: 'Not found', hint: 'Try GET /api/sites/lakeview/live' }));
});

const PORT = process.env.PORT || 4000;
server.listen(PORT, () => {
  console.log(`Mock Suryabandhu controller running at http://localhost:${PORT}`);
  console.log(`Try: http://localhost:${PORT}/api/sites/lakeview/live`);
});
