#!/bin/bash
cd ../pipo-config
npm run build
cd -
rm -rf ./data/webpage
cp -R ../pipo-config/dist ./data/webpage