#!/bin/bash

# build_and_archive_for_mappy_jenkins - A script to build mapboxgl in release mode and archive on Mappy's build server.

export LC_ALL=en_US.UTF-8

MSG="Required argument should either be 'static' or 'dynamic'"
if [ $# != 1 ]; then
    echo $MSG
    exit
elif [[ $1 != "static" && $1 != "dynamic" ]]; then
    echo $MSG
    exit
fi 

git submodule init
git submodule update
make clean && make distclean
make ipackage-strip FORMAT=$1 BUILDTYPE=Release

if [[ $1 = "static" ]]; then
	cd build/ios/pkg
	mv static/Mapbox.framework mapbox-ios-sdk
	cp -r static/Mapbox.bundle mapbox-ios-sdk
	tar -zcf mapbox-ios-sdk-static.tar.gz mapbox-ios-sdk
elif [[ $1 = "dynamic" ]]; then
	cd build/ios
	zip -r mapbox-ios-sdk-dynamic pkg
fi 

