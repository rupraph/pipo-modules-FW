import { spawnSync } from "child_process";
import chalk from "chalk";

export function pageOutput(text: string) {
  const pager = process.env.PAGER || "less";
  const args = pager === "less" ? ["-R", "-u"] : [];

  const isTTY = process.stdout.isTTY;
  if (!isTTY) {
    console.warn(chalk.yellow("⚠️ Not a TTY — displaying plain output.\n"));
    console.log(text);
    return;
  }

  try {
    const result = spawnSync(pager, args, {
      input: text,
      stdio: ["pipe", "inherit", "inherit"],
      env: {
        ...process.env,
        // Only force UTF-8 locale if not already set
        LANG: process.env.LANG || "en_US.UTF-8",
        LC_ALL: process.env.LC_ALL || "en_US.UTF-8",
      },
    });

    if (result.error) throw result.error;
    if (result.status !== 0)
      throw new Error(`Pager exited with code ${result.status}`);
  } catch (err) {
    console.warn(
      chalk.yellow("⚠️ Pager failed — falling back to plain output.\n")
    );
    console.log(text);
  }
}
