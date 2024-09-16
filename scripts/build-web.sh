#!/bin/bash
cd ./web-config
npm run build
cd -
rm -rf ./data/webpage
cp -R ./web-config/dist ./data/webpage