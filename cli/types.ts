export type Board = "motion" | "range" | "analog";
export type Mode = "debug" | "production";
export type Target = "build" | "upload" | "buildfs" | "uploadfs" | "clean";
export type Options = {
  env?: string;
  board?: Board;
  debug?: boolean;
};
