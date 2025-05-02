#!/usr/bin/env node
import chalk from "chalk";
import dotenv from "dotenv";
import { Command } from "commander";
import inquirer, { type Question } from "inquirer";
import { execSync } from "child_process";
import { version } from "../package.json";
import { Target, Options, Board } from "./types";
import { pageOutput } from "./utils";

const program = new Command();

program
  .name("cli")
  .version(version)
  .description("A CLI to build and upload Pipo firmware to your board")
  .helpOption(false);

program
  .command("help")
  .description("Show help")
  .action(() => {
    console.clear();

    const output = [];

    output.push(chalk.bold.blue("\n💡 Pipo CLI - Help Guide\n"));

    output.push(chalk.cyan("📦 What is this?\n"));
    output.push(
      "  This CLI helps you build and upload firmware to your Pipo boards (ESP32-based).\n"
    );

    output.push(chalk.cyan("⚙️  Command structure\n"));
    output.push(
      "  All commands follow the same logic:\n" +
        "  1️⃣  Default values are loaded from `.env`\n" +
        "  2️⃣  CLI arguments override `.env`\n" +
        "  3️⃣  Missing values are asked via interactive prompts\n"
    );

    output.push(chalk.cyan("🛠️  Available commands\n"));
    output.push(`  ${chalk.green("build")}       → Build your project`);
    output.push(
      `  ${chalk.green("upload")}      → Upload the firmware to your board`
    );
    output.push(`  ${chalk.green("buildfs")}     → Build LittleFS image`);
    output.push(`  ${chalk.green("uploadfs")}    → Upload LittleFS image`);
    output.push(`  ${chalk.green("clean")}       → Clean build artifacts\n`);

    output.push(chalk.cyan("📘 Command syntax\n"));
    output.push(
      `  ${chalk.green("npx cli <command> [options]")}\n\n` +
        "  Options:\n" +
        `    ${chalk.yellow(
          "--board, -b"
        )}   → Target board: motion | range | analog\n` +
        `    ${chalk.yellow(
          "--debug, -d"
        )}   → Use debug build and env (appends 'debug' to board name)\n` +
        `    ${chalk.yellow(
          "--env, -e"
        )}     → Path to .env file (default: .env)\n`
    );

    output.push(chalk.cyan("🔁 Examples\n"));
    output.push(
      chalk.gray("› Build the default board (from .env):") +
        `\n  ${chalk.green("npx cli build")}\n`
    );
    output.push(
      chalk.gray("› Build 'range' board in debug mode (no .env needed):") +
        `\n  ${chalk.green("npx cli build -b range -d")}\n`
    );
    output.push(
      chalk.gray("› Upload firmware to 'motion' board using .env file:") +
        `\n  ${chalk.green("npx cli upload -e .env")}\n`
    );
    output.push(
      chalk.gray("› Build filesystem for 'analog' board:") +
        `\n  ${chalk.green("npx cli buildfs -b analog")}\n`
    );
    output.push(
      chalk.gray("› Clean the build for 'range' debug target:") +
        `\n  ${chalk.green("npx cli clean -b range -d")}\n`
    );

    output.push(chalk.cyan("\n📁 .env file example\n"));
    output.push(chalk.green("  BOARD=motion"));
    output.push(chalk.green("  MODE=production\n"));

    output.push(chalk.gray("👋 That's all! Happy flashing ✨\n"));

    pageOutput(output.join("\n"));
  });

async function run(target: Target, cliOptions: Options) {
  const envPath = cliOptions.env || ".env";
  dotenv.config({ path: envPath });
  const env = process.env;

  // Determine board
  let board = (cliOptions.board || env.BOARD) as Board;
  let debug = cliOptions.debug || env.MODE === "debug";

  const questions: Question[] = [];
  if (!board) {
    questions.push({
      type: "list",
      name: "board",
      message: "What board are you using?",
      choices: [
        { name: "Motion", value: "motion" },
        { name: "Range", value: "range" },
        { name: "Analog", value: "analog" },
      ],
      default: "motion",
    });
  }
  // if (cliOptions.debug === undefined && env.MODE === undefined) {
  //   questions.push({
  //     type: "confirm",
  //     name: "debug",
  //     message: "Do you want to build in debug mode?",
  //     default: false,
  //   });
  // }

  const answers = questions.length > 0 ? await inquirer.prompt(questions) : {};

  board = board || answers.board || "motion";
  debug =
    cliOptions.debug !== undefined
      ? cliOptions.debug
      : answers.debug !== undefined
      ? answers.debug
      : env.MODE === "debug";

  const envSuffix = debug ? "debug" : "";
  const command = `./scripts/pio.sh run -t ${target} -e ${board}${envSuffix}`;
  switch (target) {
    case "build":
      console.log(chalk.cyan("\n🔨 Building your project..."));
      break;
    case "upload":
      console.log(chalk.cyan("\n🚀 Uploading your project..."));
      break;
    case "buildfs":
      console.log(chalk.cyan("\n🗂️  Building filesystem..."));
      break;
    case "uploadfs":
      console.log(chalk.cyan("\n🚀 Uploading filesystem..."));
      break;
    case "clean":
      console.log(chalk.cyan("\n🧹 Cleaning your project..."));
      break;
    default:
      console.log(chalk.red("❌ Unknown target."));
      return;
  }
  console.log(chalk.green("  Board: " + board));
  console.log(chalk.green("  Debug: " + (debug ? "yes" : "no")));
  console.log(chalk.gray(`  Executing: ${command}\n`));

  try {
    execSync(command, { stdio: "inherit" });
  } catch (error) {
    console.error(chalk.red(`❌ Build failed.`));
    process.exit(1);
  }
}

const targets: Target[] = ["build", "upload", "buildfs", "uploadfs", "clean"];
const targetDescriptions: Record<Target, string> = {
  build: "Builds your project",
  upload: "Uploads your project",
  buildfs: "Builds the filesystem image",
  uploadfs: "Uploads the filesystem image",
  clean: "Cleans the project",
};
for (const target of targets) {
  program
    .command(target)
    .description(targetDescriptions[target])
    .option("-e, --env <path>", "Choose your .env file path")
    .option(
      "-b, --board <board>",
      "Choose your board (motion, range or analog)"
    )
    .option("-d, --debug", "Build in debug mode (verbose output)")
    .action(async (cliOptions) => {
      await run(target, cliOptions);
    });
}

process.on("uncaughtException", (error) => {
  if (error instanceof Error && error.name === "ExitPromptError") {
    console.log("👋 until next time!");
  } else {
    // Rethrow unknown errors
    throw error;
  }
});

program.parse(process.argv);
