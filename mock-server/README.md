# Pipo Mock Server

This is an express server that mocks the behavior of the Pipo API.
It is used to quickly develop the frontend without having to rely on a pipo board
## Installation

```bash
npm install
```

## Usage

```bash
npm run start
```

That's it! The server is now running on `http://localhost:3000`,
you will need to update your `web-config/.env.development` file to `VITE_STATIC_IP=http://localhost:3000`
