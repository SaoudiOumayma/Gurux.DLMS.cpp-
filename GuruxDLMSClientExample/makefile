# project name (generate executable with this name)
TARGET   = gurux.dlms.client.bin

CC       = g++

# compiling flags here
CFLAGS   = -c

LINKER   = g++ -o

# linking flags here
LFLAGS   = -L../development/lib

# change these to set the proper directories where each files shoould be

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
INCDIR   = indlude

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES += -I$(abspath include)
# INCLUDES += -I$(abspath include/tclap)

OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm       = rm -f

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) -lgurux_dlms_cpp
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@$(rm) $(OBJDIR)/$(OBJECTS)
	@echo "Cleanup complete!"
	@echo $(OBJECTS)

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
