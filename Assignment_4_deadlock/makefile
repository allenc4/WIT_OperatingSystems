# project name (generate executable with this name)
TARGET   = lockManager

CC       = gcc
# compiling flags here
CFLAGS   = -g -Wall
INCL     = -I./inc


LINKER   = gcc -o
# linking flags here
LFLAGS   = -lm

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES   := $(wildcard $(SRCDIR)/*.c)
OBJECTS   := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm         = rm -f


.PHONY: default
default: compile clean

.PHONY: compile
compile: makedirs $(BINDIR)/$(TARGET)

makedirs:
	@mkdir -p obj bin

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) $(INCL) -c $< -o $@
	@echo "Compiled "$<" successfully!"


.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
