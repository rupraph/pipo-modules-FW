<script>
  export let color = "#000";
</script>

<i class="spinner loader" style="--color: {color};"> </i>

<style>
  .spinner {
    top: 50%;
    width: 2em;
    height: 2em;
    border: 5px solid var(--color);
    border-bottom-color: transparent;
    border-radius: 50%;
    display: inline-block;
    box-sizing: border-box;
    animation: rotation 1s linear infinite;
  }

  @keyframes rotation {
    0% {
      transform: rotate(0deg);
    }
    100% {
      transform: rotate(360deg);
    }
  }
</style>
