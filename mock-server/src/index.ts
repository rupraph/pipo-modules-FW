import express from "express";
import { createServer } from "http";
import { setupWebSocket } from "./ws";
import { setupRoutes } from "./routes";

const port = 3000;
const app = express();
const server = createServer(app);

// Middleware for parsing JSON body
app.use(express.json());
// add CORS
app.use((req, res, next) => {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Methods", "GET, POST");
  res.header(
    "Access-Control-Allow-Headers",
    "Origin, X-Requested-With, Content-Type, Accept"
  );
  next();
});

// Setup API routes
setupRoutes(app);

// Setup WebSocket
setupWebSocket(server);

// Start the server
server.listen(port, () => {
  console.log(`Server started on http://localhost:${port}`);
});
