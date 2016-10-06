#!/bin/bash

if [[ $# != 1 ]]; then
    echo -e "error!\nplease specify the certificate name as:"
    echo "  $>$0 bob"
    exit
fi

#openssl req -x509 -nodes -days 1000 -newkey rsa:1024 -keyout $FNAME-key.pem -out $FNAME-cert.pem
BNAME=$1
openssl genrsa -out "$BNAME-key.pem" 2048
openssl req -new -key "$BNAME-key.pem" -out "$BNAME.csr"
openssl x509 -req -days 1000 -in "$BNAME.csr" -signkey "$BNAME-key.pem" \
    -out "$BNAME-cer.pem"
