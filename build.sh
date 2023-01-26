pushd build
g++ -I../glad ../main.cpp -o Boat $(sdl2-config --cflags --libs)
popd
