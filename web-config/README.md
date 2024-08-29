# Pipo config

This repo is a web app that allows you to configure your [Pipo device](https://github.com/rupraph/pipo-single).

<!-- ## Getting started

- Clone the repo

```bash
 git clone git@github.com:rupraph/pipo-single.git
```

- Install the dependencies

```bash
npm install
```

- Start the dev server

```bash
npm run dev
```

## Building the app

```bash
npm run build
```

## Uploading it to your Pipo device

### Setup
You will need to clone the [pipo-single](https://github.com/rupraph/pipo-single) repo, and place aside this one: 
```bash
any-folder
  |- pipo-single
  |- pipo-config
```

Then, you will need to run `npm install` within the `pipo-single` folder, to install platformio and make availiable as a global command.

```bash
cd pipo-single
npm install
```

### Make Pipo device available for upload

By default, when pipo starts, it cannot be uploaded to. You need to press the `BO1` button while pressing the `RESET1` button to make it available for upload. In other words, pipo has to start with the `BO1` button pressed to switch to upload mode.

### Uploading the app

Now, from the `pipo-config` folder, you can run the following command to build the app and upload it to your Pipo device:

```bash
npm run upload
```

### Working without a Pipo

If you dont have a Pipo board or if you want to quickly develop and test without having to upload the code to your pipo all the time, you can simply run:

```bash
motion=1 npm run dev
```
or 
```bash
analog=1 npm run dev
```
or
```bash
range=1 npm run dev
```

Thoose three commands will start a mock-server imitating Pipo's behavior. -->