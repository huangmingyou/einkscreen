#!/bin/bash
set -x
convert -background white -fill black \
  -rotate 90 -transverse -font 微软雅黑 -pointsize 64  \
  -gravity Center -extent 1872x1404  $1 \
  -depth 8 gray:- > ~/down/t1.png

curl -X POST -F "action=upload" -F "x=0" -F "y=0" -F "w=1872" -F "h=1404" -F "file=@./t1.png" http://10.3.165.240:9988/showupload
