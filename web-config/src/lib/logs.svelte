<script lang="ts">
  import { onDestroy, onMount } from "svelte";
  import axios from "axios";
  import Collapse from "./collapse.svelte";
  import { pipoio } from "../pipoio";
  type Log = {
    type: "error" | "warning" | "info";
    message: string;
    timestamp: string;
  };
  let logs: Log[] = [];
  let interval = 0;
  function format(timestamp: string) {
    const date = new Date(parseInt(timestamp));
    date.setHours(date.getHours() - 1);
    return `${date.toTimeString().split(" ")[0]}:${date.getMilliseconds()}`;
  }

  function parse(entries: string[], append = false) {
    const news = entries
      .map((entry) => {
        const match = entry.match(/(\d+):\((\d)\)(.*)/);
        if (!match) return null;
        return {
          timestamp: format(match[1]),
          type: ["info", "warning", "error"][parseInt(match[2])],
          message: match[3],
        };
      })
      .filter(Boolean) as Log[];
    logs = append ? [...logs, ...news] : news;
  }
  pipoio.on("logs", ({ entries }) => {
    console.log("entries", entries);
    parse(entries, true);
  });
  function fetch() {
    axios.get("/logs").then(({ data }) => {
      console.log("ICI", logs);
      parse(data.split("--"));
    });
  }
  onMount(() => {
    fetch();
  });
  onDestroy(() => clearInterval(interval));
</script>

<Collapse title="Logs">
  <div class="logs {logs.length > 6 ? 'scroll' : ''}">
    {#each logs as log}
      <span class="log {log.type}"
        ><bold>{log.timestamp}</bold>
        <p>{log.message}</p></span
      >
    {/each}
  </div>
</Collapse>

<style>
  div {
    width: 100%;
  }
  .logs {
    padding: 1rem;
    border-bottom: 1px solid white;
    width: 100%;
    max-height: 300px;
    min-height: 50px;
    overflow-y: auto;
  }
  .log {
    display: grid;
    grid-template-columns: 9em auto;
    grid-auto-flow: column;
    align-items: center;
    justify-content: start;
    line-height: 1rem;
  }
  .log.info p {
    color: #55ff55;
  }
  .log.warning p {
    color: #f5f500;
  }
  .log.error p {
    background-color: #f55555;
  }
</style>
