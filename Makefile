# project name (generate executable with this name)
TARGET   = oidcd
CC       = gcc
# compiling flags here
CFLAGS   = -Wall -Wextra -g -Ilib -I/usr/local/include

LINKER   = gcc
# linking flags here
LFLAGS   = -lcurl -L /usr/local/lib -lsodium -L lib -ljsmn

# change these to proper directories where each file should be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

all: $(BINDIR)/$(TARGET)
	cd oidc-prompt && make
	cd oidc-token && make 

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	cd oidc-prompt && make clean
	cd oidc-token && make clean

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
	cd oidc-prompt && make remove
	cd oidc-token && make remove

