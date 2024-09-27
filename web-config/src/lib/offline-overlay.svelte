<script lang="ts">
  import { isLive } from "../services";
  let live = false;
  isLive.subscribe((value) => {
    live = value;
  });
</script>

{#if !live}
  <div class="overlay">
    <article class="scroll">
      <h2>Pipo is offline, waiting for reconnection</h2>
      <h3>Troubleshooting:</h3>
      <ul>
        <li>
          <h4>
            Pipo could not connect to Wifi and switched to Access Point mode
          </h4>
          <ul class="instruction">
            <li>Check if you see the Pipo network in your wifi list</li>
            <li>Connect to the Pipo network</li>
            <li>Reconfigure wifi and switch back to your wifi</li>
          </ul>
        </li>
        <li>
          <h4>
            You were connected to pipo in Access Point mode and configured wifi
          </h4>
          <ul class="instruction">
            <li>
              Pipo connected to your Wifi and no longer provides axxess point:
              just connect to wifi
            </li>
          </ul>
        </li>
        <li>
          <h4>None of the above and it disconnected</h4>
          <ul class="instruction">
            <li>
              Pipo must have crashed, just reboot your Pipo by pressing the
              reset button. If it does not solve the problem, contact support.
            </li>
          </ul>
        </li>
      </ul>
      <p class="padding"></p>
    </article>
  </div>
{/if}

<style>
  .overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(50, 50, 50, 0.95);
    z-index: 1000;
    cursor: wait;
    display: flex;
    justify-content: center;
    overflow-y: auto;
    overflow-x: hidden;
    cursor: wait;
  }
  h2 {
    margin: 0;
  }
  article {
    padding: 1em;
    padding-bottom: 60px;
    max-width: min(100%, 600px);
  }
  ul,
  li {
    flex-direction: column;
    align-items: flex-start;
    text-wrap: wrap;
    text-align: left;
  }
  ul.instruction > li {
    list-style: inside;
  }
  li {
    list-style: none;
  }
  .padding {
    height: 100px;
  }
</style>
