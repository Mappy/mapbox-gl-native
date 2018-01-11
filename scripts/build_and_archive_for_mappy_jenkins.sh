#!/bin/bash

export LC_ALL=en_US.UTF-8

git submodule init
git submodule update
make clean && make distclean
make ipackage-strip FORMAT=static BUILDTYPE=Release

cd build/ios/pkg
mv static/Mapbox.framework mapbox-ios-sdk
cp -r static/Mapbox.bundle/* mapbox-ios-sdk
tar -zcf mapbox-ios-sdk.tar.gz mapbox-ios-sdk
