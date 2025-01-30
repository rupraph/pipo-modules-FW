import { defineConfig } from "vitest/config";

export default defineConfig({
  test: {
    include: ["test/web-sockets/**/*.test.ts"],
  },
});
