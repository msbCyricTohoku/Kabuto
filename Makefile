CXX = g++
CXXFLAGS = `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

Kabuto: main.o viewer.o
	$(CXX) -o Kabuto main.o viewer.o $(LDFLAGS)

main.o: main.cpp viewer.h
	$(CXX) -c main.cpp $(CXXFLAGS)

viewer.o: viewer.cpp viewer.h
	$(CXX) -c viewer.cpp $(CXXFLAGS)

clean:
	rm -f Kabuto main.o viewer.o
