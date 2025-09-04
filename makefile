CXX = g++
CXXFLAGS = -std=c++11 -g -Wall 
TARGET = my_shell
all: $(TARGET)
$(TARGET): main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.o #link the object file to create the executable
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp #compile the src file to a object file
clean:
	rm -f *.o $(TARGET) #clean the compiled files