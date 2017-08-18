# project name (generate executable with this name)
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token


SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
LIBDIR   = lib
PROVIDERCONFIG = issuer.config

CC       = gcc
# compiling flags here
CFLAGS   = -Wall -Wextra -g -I$(LIBDIR) -I/usr/local/include

LINKER   = gcc
# linking flags here
LFLAGS   = -lcurl -L /usr/local/lib -lsodium -L$(LIBDIR)/jsmn -ljsmn

INSTALL_PATH?=/usr

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS := $(filter-out $(OBJDIR)/$(ADD).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
GEN_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(ADD).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
ADD_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
CLIENT_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(ADD).o, $(OBJECTS))
rm       = rm -r -f

all: dependecies build oidcdir

oidcdir:
	@[ -d ~/.config ] && mkdir -p ~/.config/oidc-agent || mkdir -p ~/.oidc-agent
	@[ -d ~/.config ] && cp $(PROVIDERCONFIG) ~/.config/oidc-agent/$(PROVIDERCONFIG) -n || cp $(PROVIDERCONFIG) ~/.oidc-agent/$(PROVIDERCONFIG) -n
	@echo "Created oidc dir"

dependecies: 
	@[ -d $(LIBDIR)/jsmn ] || git clone https://github.com/zserge/jsmn.git $(LIBDIR)/jsmn 
	@[ -f $(LIBDIR)/jsmn/libjsmn.a ] || (cd $(LIBDIR)/jsmn && make)
	@echo "Dependecies OK"

build: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

install: 
	@install -D $(BINDIR)/$(AGENT) $(INSTALL_PATH)/bin/$(AGENT)
	@install -D $(BINDIR)/$(GEN) $(INSTALL_PATH)/bin/$(GEN)
	@install -D $(BINDIR)/$(ADD) $(INSTALL_PATH)/bin/$(ADD)
	@install -D $(BINDIR)/$(CLIENT) $(INSTALL_PATH)/bin/$(CLIENT)
	@echo "Installation complete!"


$(BINDIR)/$(AGENT): $(AGENT_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(AGENT_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(GEN): $(GEN_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(GEN_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(ADD): $(ADD_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(ADD_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(CLIENT): $(CLIENT_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(CLIENT_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJDIR)

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)
	@echo "Executable removed!"
	@$(rm) $(LIBDIR)
	@echo "Dependencies removed!"

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_PATH)/bin/$(AGENT)
	@echo "Uninstalled "$(AGENT)
	@rm $(INSTALL_PATH)/bin/$(GEN)
	@echo "Uninstalled "$(GEN)
	@rm $(INSTALL_PATH)/bin/$(ADD)
	@echo "Uninstalled "$(ADD)
	@rm $(INSTALL_PATH)/bin/$(CLIENT)
	@echo "Uninstalled "$(CLIENT)

