<script lang="ts">
  import { onMount } from "svelte";
  import r from "regl";
  import mat4 from "gl-mat4";
  import { pipoio } from "../../pipoio";
  import type { PipoConfig } from "../../types";
  import { colors, cubeElements, cubePosition } from "./constants";
  export let config: PipoConfig<"motion">;
  function hexToRgba(hex: string) {
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex)!;
    return [
      parseInt(result[1], 16) / 255,
      parseInt(result[2], 16) / 255,
      parseInt(result[3], 16) / 255,
      255,
    ];
  }
  let angles = [0, 0, 0];
  onMount(() => {
    pipoio.on("controlChange", ({ control, value }) => {
      if (!config) return;
      const axes = config.engine["engine-midi"];
      const index =
        control === axes.yaw.cc_number
          ? 0
          : control === axes.pitch.cc_number
            ? 1
            : control === axes.roll.cc_number
              ? 2
              : -1;
      if (index === -1) return;
      // if (index !== 0) return;
      angles[index] =
        ((value - axes.yaw.min_input) /
          (axes.yaw.max_output - axes.yaw.min_output)) *
        Math.PI;
      drawCube();
    });
    const bgcolor = hexToRgba(
      getComputedStyle(document.body).getPropertyValue("--bg-color")
    );
    const regl = r({
      canvas: document.getElementById("cube"),
    });

    const drawCube = regl({
      frag: `
  precision mediump float;
  varying vec3 vcolor;
  void main () {
    gl_FragColor = vec4(vcolor,1.);
    }`,
      vert: `
    precision mediump float;
    attribute vec3 position;
    attribute vec4 transform;
  attribute vec3 color;
  varying vec3 vcolor;
  uniform mat4 projection, view;
  void main() {
    vcolor = color;
    gl_Position = projection * view  * vec4(position, 1);
  }`,
      attributes: {
        position: cubePosition,
        transform: () => {
          const dst = mat4.create();
          // mat4.rotateX(dst, dst, angles[0]);
          // mat4.rotateY(dst, dst, angles[1]);
          // mat4.rotateZ(dst, dst, angles[2]);
          return dst;
        },
        color: colors,
      },
      elements: regl.elements({
        primitive: "triangles",
        data: cubeElements,
      }),
      uniforms: {
        view: ({ tick }) => {
          const t = 0.01 * tick;
          const cam = mat4.lookAt(
            [],
            [2, 0, 0],
            // [5 * Math.cos(t), 2.5 * Math.sin(t), 5 * Math.sin(t)],
            [0, 0.0, 0],
            [0, 1, 0]
          );
          mat4.rotateX(cam, cam, angles[0]);
          mat4.rotateY(cam, cam, angles[1]);
          mat4.rotateZ(cam, cam, angles[2]);
          return cam;
        },
        projection: ({ viewportWidth, viewportHeight }) =>
          mat4.perspective(
            [],
            Math.PI / 4,
            viewportWidth / viewportHeight,
            0.01,
            10
          ),
      },
    });
    drawCube();
  });
</script>

<canvas id="cube"></canvas>

<style>
  #cube {
    width: 100%;
    height: 100%;
  }
</style>
