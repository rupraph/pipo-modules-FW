<script lang="ts">
  import { onDestroy, onMount } from "svelte";
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
    parse(entries, true);
  });
  function fetch() {
    pipoio.get("/logs").then(({ data }) => {
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
      <span>{log.timestamp}</span>
      <span class="log {log.type}">{log.message}</span>
    {/each}
  </div>
</Collapse>

<style>
  .logs {
    margin: 1rem;
    font-size: 1.1em;
    display: grid;
    grid-template-columns: auto auto;
    grid-template-columns: 7em auto;
    grid-auto-flow: column;
    justify-items: start;
    width: 100%;
    grid-auto-flow: row;
    width: 100%;
    column-gap: 1em;
    max-height: 300px;
    min-height: 50px;
    overflow-y: auto;
  }
  .log > span:nth-child(2n + 1) {
    font-weight: bold;
  }
  .log.info {
    color: #55ff55;
  }
  .log.warning {
    color: #f5f500;
  }
  .log.error {
    background-color: #f55555;
  }
</style>
