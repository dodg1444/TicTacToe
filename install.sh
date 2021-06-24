#! /bin/bash

mkdir client/build && cd client/build 
cmake ../
make
mv client ../client.exe

cd ../../

mkdir server/build && cd server/build
cmake ../
make 

mv server ../server.exe

echo "FINISHED BUILDING THE PROJECT"
