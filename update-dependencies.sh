#!/bin/sh

echo "preparing the latest version of 3rd-party dependencies ...\n"

mkdir -p 3rdparty
cd 3rdparty

echo "--> joyent / http-parser"
if [ -e http-parser ]; then
    cd http-parser
    git pull https://github.com/joyent/http-parser.git
else
    git clone --depth=1 https://github.com/joyent/http-parser.git
fi

cd ..
echo "\ndone."
