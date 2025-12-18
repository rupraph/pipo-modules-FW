<script lang="ts">
  import { isLive } from "../services";
  let live = false;

  isLive.subscribe((value) => {
    console.log(
      "OfflineOverlay: isLive changed to",
      value,
      "overlay will be",
      value ? "hidden" : "visible"
    );
    live = value;
  });
</script>

{#if !live}
  <div class="overlay">
    <article class="scroll">
      <h2>Connecting to Pipo...</h2>
      <p style="font-size: x-large; text-align: center;">¯\_(ツ)_/¯</p>
      <p style="margin-bottom: 1em; text-align: center;">If it persists:</p>
      <ul
        style="text-align: left; margin: 0 auto; max-width: 400px; font-size: 0.9em;"
      >
        <li>• Check your WiFi connection to Pipo</li>
        <li>• Try restarting Pipo if the issue persists</li>
        <li>• Make sure you only have one config page open</li>
        <li>• Make sure you only have 1 device connected to the Pipo Wifi</li>
      </ul>
      <ul>
        <li>
          <!-- <h4>Pipo could not be reached</h4> -->
          <!-- <ul class="instruction">
            <li>Check if you see the Pipo network in your wifi list</li>
            <li>Connect to the Pipo network</li>
            <li>Reconfigure wifi and switch back to your wifi</li>
          </ul> -->
        </li>
        <li>
          <!-- <h4>
            You were connected to pipo in Access Point mode and configured wifi
          </h4> -->
          <!-- <ul class="instruction">
            <li>
              Pipo connected to your Wifi and no longer provides axxess point:
              just connect to wifi
            </li>
          </ul> -->
        </li>
        <li>
          <!-- <h4>None of the above and it disconnected</h4>
          <ul class="instruction">
            <li>
              Pipo must have crashed, just reboot your Pipo by pressing the
              reset button. If it does not solve the problem, contact support.
            </li>
          </ul> -->
        </li>
      </ul>
    </article>
  </div>
{/if}

<style>
  .overlay {
    color: white;
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(50, 50, 50, 1);
    z-index: 1000;
    cursor: wait;
    display: flex;
    justify-content: center;
    overflow-y: auto;
    overflow-x: hidden;
    cursor: wait;
  }
  h2 {
    padding-top: 10vw;
    margin: 0;
    font-size: x-large;
    text-align: center;
  }

  h3 {
    margin: 0;
    font-size: large;
    text-align: center;
  }
  article {
    padding: 1em;
    padding-bottom: 60px;
    max-width: min(100%, 600px);
    text-align: center;
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
