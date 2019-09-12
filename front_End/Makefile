CC = clang++
SOURCE = ./src/Main.cpp ./src/token.cpp ./src/parser.cpp ./src/genIR.cpp
TARGET = ./bin/toy_Front

$(TARGET) : $(SOURCE)
	$(CC) $(SOURCE) -o  $(TARGET) -g -O0 `llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native`

clean :
	rm $(TARGET)
