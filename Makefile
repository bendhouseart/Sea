gcc_opts=-Wall -fPIC -c -std=c++0x
debug_opts=-g -ggdb

all: build/sea.so
test: build/sea.so test/test_custom

build/config.o: src/config.cpp
	gcc ${gcc_opts} ${debug_opts} src/config.cpp -o build/config.o
build/functions.o: src/functions.cpp src/functions.h
	gcc ${gcc_opts} ${debug_opts} src/functions.cpp -o build/functions.o
build/logger.o: src/logger.cpp src/logger.h
	gcc ${gcc_opts} ${debug_opts} src/logger.cpp -o build/logger.o
build/passthrough.o: src/passthrough.cpp src/passthrough.h
	gcc ${gcc_opts} ${debug_opts} src/passthrough.cpp -o build/passthrough.o
build/sea.o: src/sea.cpp src/sea.h
	gcc ${gcc_opts} ${debug_opts} src/sea.cpp -o build/sea.o
build/sea.so: build/passthrough.o build/functions.o build/logger.o build/config.o build/sea.o
	gcc -shared build/passthrough.o build/functions.o build/logger.o build/config.o build/sea.o ${debug_opts} -o build/sea.so -ldl -lpthread -lstdc++ -liniparser
test/test_custom: tests/test_custom.cpp build/passthrough.o build/sea.o build/config.o build/logger.o
	gcc ${debug_opts} -std=c++0x -Wall -fPIC tests/test_custom.cpp build/passthrough.o build/sea.o build/config.o build/logger.o -o tests/test_custom -lstdc++ -liniparser -ldl -lgtest -lpthread 
clean:
	\rm -f build/logger.o build/functions.o build/passthrough.o build/config.o build/sea.o build/sea.so build/*.gcda build/*.gcov build/*.gcno tests/test_custom
