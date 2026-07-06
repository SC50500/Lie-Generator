// server.js
// Minimal backend proxy: browser -> this server -> ollama.com/api
//
// Why this exists: ollama.com's cloud API doesn't send CORS headers for
// arbitrary browser origins, and it shouldn't hold your API key client-side
// anyway. This server holds the key (loaded from a local .env file) and is
// the only thing that ever talks to ollama.com directly.

import "dotenv/config"; // loads variables from a local .env file into process.env
import express from "express";
import cors from "cors";

const app = express();
app.use(cors());          // allow your frontend page to call this server
app.use(express.json());  // parse JSON request bodies

const OLLAMA_API_KEY = process.env.OLLAMA_API_KEY;
const OLLAMA_ENDPOINT = "https://ollama.com/api/chat";
const PORT = process.env.PORT || 3787;

if (!OLLAMA_API_KEY) {
  console.error("Missing OLLAMA_API_KEY. Create a .env file in this folder with:\n  OLLAMA_API_KEY=your_actual_key_here");
  process.exit(1);
}

// The only route your frontend needs to know about.
app.post("/api/chat", async (req, res) => {
  try {
    const upstream = await fetch(OLLAMA_ENDPOINT, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        "Authorization": `Bearer ${OLLAMA_API_KEY}`
      },
      body: JSON.stringify(req.body) // { model, messages, stream }
    });

    const text = await upstream.text();

    if (!upstream.ok) {
      console.error("Ollama error:", upstream.status, text);
      return res.status(upstream.status).send(text);
    }

    res.set("Content-Type", "application/json");
    res.send(text);
  } catch (err) {
    console.error("Proxy error:", err);
    res.status(502).json({ error: "proxy_failed", message: err.message });
  }
});

app.listen(PORT, () => {
  console.log(`Ollama proxy listening on http://localhost:${PORT}`);
  console.log(`Frontend should POST to http://localhost:${PORT}/api/chat`);
});
