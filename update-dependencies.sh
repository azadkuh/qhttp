#!/bin/sh

echo "preparing the latest version of 3rd-party dependencies ...\n"

mkdir -p 3rdparty
cd 3rdparty

echo "--> joyent / http-parser"
if [ -e http-parser ]; then
    cd http-parser
    git pull origin master
else
    git clone --depth=1 https://github.com/joyent/http-parser.git -b master
fi

cd ..
echo "\ndone."
