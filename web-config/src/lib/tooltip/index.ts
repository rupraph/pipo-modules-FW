import Tooltip from "./tooltip-cmp.svelte";

export function tooltip(element: HTMLElement) {
  let title: string;
  let tooltip: Tooltip;
  function mouseOver(event: MouseEvent) {
    // NOTE: remove the `title` attribute, to prevent showing the default browser tooltip
    // remember to set it back on `mouseleave`
    title = element.getAttribute("title");
    element.removeAttribute("title");

    tooltip = new Tooltip({
      props: {
        title: title,
        x: event.pageX,
        y: event.pageY,
      },
      target: document.body,
    });
  }
  function mouseMove(event: MouseEvent) {
    tooltip.$set({
      x: event.pageX,
      y: event.pageY,
    });
  }
  function mouseLeave() {
    tooltip.$destroy();
    // NOTE: restore the `title` attribute
    element.setAttribute("title", title);
  }

  element.addEventListener("mouseover", mouseOver);
  element.addEventListener("mouseleave", mouseLeave);
  element.addEventListener("mousemove", mouseMove);

  return {
    destroy() {
      element.removeEventListener("mouseover", mouseOver);
      element.removeEventListener("mouseleave", mouseLeave);
      element.removeEventListener("mousemove", mouseMove);
    },
  };
}
