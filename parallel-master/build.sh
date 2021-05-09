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
#gcc -c pt2_decoder.c -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I ./include
#gcc *.o -lbfd -lpthread  -o pt2_decoder
g++ -shared -fPIC -c parallel_excl_decoder.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I profiler/internal/include -I ./include  -I/usr/include -I$JAVA_HOME/include -I$JAVA_HOME/include/linux

g++ -shared -fPIC -c parallel_decoder.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I ./include -I profiler/internal/include -I ./  -I/usr/include -I$JAVA_HOME/include -I$JAVA_HOME/include/linux
#gcc -fPIC -c c_wrapper.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I profiler/internal/include -I ./include -I ./
#g++ *.o -lbfd -lm -lpthread -shared -fPIC -I /usr/include -I /usr/lib/java/jdk1.8.0_281/include/ -I /usr/lib/java/jdk1.8.0_281/include/linux -o cpplib_shared.so
#gcc -fPIC -c c_wrapper.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I profiler/internal/include -I ./include -I ./
#g++ *.o -lbfd -lpthread -o Main
gcc -fPIC -c org_example_SimpDecoder.cpp -I libipt/internal/include/ -I libipt/include/ -I sideband/include/ -I profiler/include/ -I profiler/internal/include -I ./include -I ./ -I /usr/lib/java/jdk1.8.0_281/include -I /usr/lib/java/jdk1.8.0_281/include/linux
g++ *.o -lbfd -lpthread -shared -fPIC  -lm -Wl,-rpath /home/bigdataflow/DistributedDecoder/ -I /usr/include -I /usr/lib/java/jdk1.8.0_281/include -I /usr/lib/java/jdk1.8.0_281/include/linux -o libSimpDecoder.so
rm *.o
