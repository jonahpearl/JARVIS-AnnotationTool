mkdir ../build
cd ../build
cmake ..
make -j12
make install
cd ../deployment
dpkg-deb --build --root-owner-group deb_packages/AnnotationTool_0.3-1_amd64_1804
mv deb_packages/AnnotationTool_0.3-1_amd64_1804.deb .