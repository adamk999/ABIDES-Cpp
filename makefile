# Compiler
CXX = g++-11

# Compiler flags
CXXFLAGS = 

# Target executable
TARGET = my_project

# All target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): Kernel.o agents/Agent.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) Kernel.o agents/Agent.o

# Compile Kernel.cpp to Kernel.o
Kernel.o: Kernel.cpp
	$(CXX) $(CXXFLAGS) -c Kernel.cpp

# Compile agents/Agent.cpp to agents/Agent.o
agents/Agent.o: agents/Agent.cpp
	$(CXX) $(CXXFLAGS) -c agents/Agent.cpp -o agents/Agent.o

# Clean the build files
clean:
	rm -f $(TARGET) Kernel.o agents/Agent.o
