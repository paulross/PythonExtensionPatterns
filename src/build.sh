rm -rf build/
python3 setup.py build
cp build/lib.macosx-10.6-intel-3.3/*.so .
ls -l

