<script lang="ts">
  import axios from "axios";
  import Config from "./config.svelte";
  import { configSave } from "../../services/config";
  import Tabs from "./tabs.svelte";
  import { formatNumbers, onError } from "../../utils";
  import type { PipoConfig } from "../../types";
  import { pipoio } from "../../pipoio";

  let fetchError: string;
  let error: string;

  async function fetch() {
    try {
      const names = (await axios.get<string>("/configs")).data.split(",");
      const active = (await axios.get<string>("/config-active")).data;
      let config = (
        await axios.get<PipoConfig<"unknown">>("/configs", {
          params: { name: active },
        })
      ).data;
      // Format numbers in the config object
      config = formatNumbers(config, 4);
      return {
        names,
        active,
        config,
      };
    } catch (e) {
      fetchError = onError(e);
      return {
        names: [],
        active: "",
        config: null,
      };
    }
  }

  let state = fetch();
  pipoio.on("connect", () => {
    state = fetch();
  });

  function onClick(name: string) {
    return axios({
      method: "post",
      url: "/active-config",
      params: { name },
    })
      .catch((e) => {
        console.log(e);
        error = e;
      })
      .then(() => fetch())
      .then((resp) => {
        state = Promise.resolve(resp);
      });
  }
  async function onCreate() {
    const { names } = await state;
    const index = names.length + 1;
    return axios({
      method: "post",
      url: "/config-new",
      params: { name: `config-${index}` },
    })
      .catch((e) => {
        console.log(e);
        error = e;
      })
      .then(() => fetch())
      .then((resp) => {
        state = Promise.resolve(resp);
      });
  }

  function onRename(oldname: string, newname: string) {
    return axios({
      method: "post",
      url: "/config-rename",
      params: { oldname, newname },
    })
      .catch((e) => {
        console.log(e);
        error = e;
      })
      .then(() => fetch())
      .then((resp) => {
        state = Promise.resolve(resp);
      });
  }
  function onDelete(name: string) {
    return axios({
      method: "post",
      url: "/config-delete",
      params: { name },
    })
      .catch((e) => {
        console.log(e);
        error = e;
      })
      .then(() => fetch())
      .then((resp) => {
        state = Promise.resolve(resp);
      });
  }
</script>

<div class="configs">
  {#if error}
    <p class="error">{error}</p>
  {/if}

  {#if !error && state}
    {#await state}
      <p>Waiting for Pipo to send configs...</p>
    {:then resp}
      <Tabs
        items={resp.names}
        active={resp.active}
        {onDelete}
        {onCreate}
        {onClick}
        {onRename}
      >
        {#if resp.config}
          <Config config={resp.config} name={resp.active} />
        {/if}
      </Tabs>
    {/await}
  {/if}
</div>

<style>
  .configs {
    width: 100%;
  }
  .delete {
    position: fixed;
    right: 0;
    top: 0;
    margin: 1em;
  }
</style>
