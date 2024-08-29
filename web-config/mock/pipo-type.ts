export const pipoType = process.env.motion
  ? "motion"
  : process.env.analog
  ? "analaog"
  : process.env.range
  ? "range"
  : false;
