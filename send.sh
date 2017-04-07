#!/bin/bash
 
ftp -n 10.42.0.212<<EOF
user me 123
lcd web
put index.html
EOF
