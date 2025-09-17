CXX = g++
CXXFLAGS = -std=c++17 -g -pedantic -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer

# List all source files
SRCS = prog1.cpp prog2.cpp

# Convert .cpp to .exe for targets
BINS = $(SRCS:.cpp=.exe)

# Optional dependencies (headers)
DEPS = 

all: $(BINS)

# Rule to build .o files (if needed)
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Rule to build executables
%.exe: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f $(BINS) *.o
