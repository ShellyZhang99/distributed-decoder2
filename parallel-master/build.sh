cd libipt
gcc -fPIC -c -DPT_VERSION_MAJOR=2 -DPT_VERSION_MINOR=0 -DPT_VERSION_PATCH=0 -DPT_VERSION_BUILD="0" -DPT_VERSION_EXT="" -DFEATURE_THREADS ./src/*.c -I ./internal/include -I ./include/ -I ../include
mv *.o ../
cd ..
cd pevent
gcc -fPIC -c ./src/*.c -I ./include/ -I ../libipt/include
mv *.o ../
cd ..
cd sideband
gcc -fPIC -c ./src/*.c -I ./include/ -I ./internal/include/ -I../libipt/include/ -I ../pevent/include -I ../include
mv *.o ../
cd ..
cd profiler
gcc -fPIC -c ./src/*.c -I ./include -I ./internal/include -I ../libipt/include -I ../include
mv *.o ../
cd ..
gcc -c pt2_decoder.c -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I ./include
gcc *.o -lbfd -lpthread  -o pt2_decoder
#g++ -fPIC -c parallel_excl_decoder.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I ./include

#g++ -fPIC -c parallel_decoder.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I ./include -I ./
#g++ -fPIC -c c_wrapper.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I ./include -I ./
#g++ *.o -lbfd -lpthread -fPIC -shared -o cpplib_shared.so
rm *.o
