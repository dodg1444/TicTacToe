#! /bin/bash

success_build_flag=true

mkdir bin
mkdir client/build && cd client/build 
if cmake ../ && make ; then
	mv client ../../bin/client.exe
else
	success_build_flag=false
fi

cd ../../

mkdir server/build && cd server/build
if cmake ../ && make ; then
	mv server ../../bin/server.exe
else
	success_build_flag=false
fi

if [ "$success_build_flag" = true  ] ; then
	echo "FINISHED BUILDING THE PROJECT"
	echo "Binary files have been written to bin directory"
else
	echo "SOMETHING'S WRONG I CAN FEEL IT"
fi
