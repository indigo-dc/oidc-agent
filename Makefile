# project name (generate executable with this name)
TARGET   = oidcd
CC       = gcc
# compiling flags here
CFLAGS   = -Wall -Wextra -g -Ilib -I/usr/local/include

LINKER   = gcc
# linking flags here
LFLAGS   = -lcurl -L /usr/local/lib -lsodium -ljsmn

# change these to proper directories where each file should be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

all: $(BINDIR)/$(TARGET)
	cd oidc-add && make
	cd oidc-gen && make 
	cd api && make

$(BINDIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	cd oidc-add && make clean
	cd oidc-gen && make clean
	cd api && make clean 

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
	cd oidc-add && make remove
	cd oidc-gen && make remove
	cd api && make remove

