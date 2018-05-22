# project name (generate executable with this name)
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token

VERSION   ?= 1.2.0
# These are needed for the RPM build target:
BASEDIR   = $(PWD)
BASENAME := $(notdir $(PWD))
SRC_TAR   = oidc-agent.tar
PKG_NAME  = oidc-agent


SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
LIBDIR   = lib
MANDIR 	 = man
PROVIDERCONFIG = issuer.config

CC       = gcc
# compiling flags here
CFLAGS   = -g -std=c99 -I$(LIBDIR) #-Wall -Wextra 

LINKER   = gcc
# linking flags here
LFLAGS   = -lcurl -lsodium -L$(LIBDIR)/jsmn -ljsmn -lmicrohttpd 

INSTALL_PATH ?=/usr
MAN_PATH     ?=/usr/share/man
CONFIG_PATH  ?=/etc

SOURCES  := $(shell find $(SRCDIR) -name "*.c")
INCLUDES := $(shell find $(SRCDIR) -name "*.h")
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS := $(filter-out $(OBJDIR)/$(ADD).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS)) $(LIBDIR)/list/src/*.o
GEN_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(ADD).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS)) $(LIBDIR)/list/src/*.o
ADD_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS)) $(LIBDIR)/list/src/*.o
CLIENT_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(ADD).o, $(OBJECTS)) $(LIBDIR)/list/src/*.o
rm       = rm -f

all: dependecies build man oidcdir

oidcdir:
	@[ -d ~/.config ] && mkdir -p ~/.config/oidc-agent || mkdir -p ~/.oidc-agent
	@[ -d ~/.config ] && touch ~/.config/oidc-agent/$(PROVIDERCONFIG) || touch ~/.oidc-agent/$(PROVIDERCONFIG)
	@echo "Created oidc dir"

dependecies: 
	@[ -d $(LIBDIR)/jsmn ] || git clone https://github.com/zserge/jsmn.git $(LIBDIR)/jsmn 
	@[ -f $(LIBDIR)/jsmn/libjsmn.a ] || (cd $(LIBDIR)/jsmn && make)
	@git submodule init
	@git submodule update
	@cd $(LIBDIR)/list && make
	@echo "Dependecies OK"

copy_src_dir_structure:
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;

build: copy_src_dir_structure $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

install: install_man
	@install -D $(BINDIR)/$(AGENT) $(INSTALL_PATH)/bin/$(AGENT)
	@install -D $(BINDIR)/$(GEN) $(INSTALL_PATH)/bin/$(GEN)
	@install -D $(BINDIR)/$(ADD) $(INSTALL_PATH)/bin/$(ADD)
	@install -D $(BINDIR)/$(CLIENT) $(INSTALL_PATH)/bin/$(CLIENT)
	@install -D $(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG)
	@echo "Installation complete!"

install_man: man
	@install -D $(MANDIR)/$(AGENT).1 $(MAN_PATH)/man1/$(AGENT).1
	@install -D $(MANDIR)/$(GEN).1 $(MAN_PATH)/man1/$(GEN).1
	@install -D $(MANDIR)/$(ADD).1 $(MAN_PATH)/man1/$(ADD).1
	@install -D $(MANDIR)/$(CLIENT).1 $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Installed man pages!"


$(BINDIR)/$(AGENT): copy_src_dir_structure $(AGENT_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(AGENT_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(GEN): copy_src_dir_structure $(GEN_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(GEN_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(ADD): copy_src_dir_structure $(ADD_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(ADD_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(CLIENT): copy_src_dir_structure $(CLIENT_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(CLIENT_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
ifndef NO_COLOR
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\"
else
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\" -DNO_COLOR
endif
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) -r $(OBJDIR)
	@$(rm) -r debian/.debhelper
	@$(rm) -r rpm/rpmbuild

distclean: clean
	@$(rm) -r $(BINDIR)
	@$(rm) -r $(MANDIR)

.PHONY: remove
remove: clean
	@$(rm) -r $(BINDIR)
	@echo "Executable removed!"
	@$(rm) -r $(LIBDIR)
	@echo "Dependencies removed!"

.PHONY: uninstall
uninstall: uninstall_man
	@$(rm) $(INSTALL_PATH)/bin/$(AGENT)
	@echo "Uninstalled "$(AGENT)
	@$(rm) $(INSTALL_PATH)/bin/$(GEN)
	@echo "Uninstalled "$(GEN)
	@$(rm) $(INSTALL_PATH)/bin/$(ADD)
	@echo "Uninstalled "$(ADD)
	@$(rm) $(INSTALL_PATH)/bin/$(CLIENT)
	@echo "Uninstalled "$(CLIENT)
	@$(rm) -r $(CONFIG_PATH)/oidc-agent/

uninstall_man:
	@$(rm) $(MAN_PATH)/man1/$(AGENT).1
	@$(rm) $(MAN_PATH)/man1/$(GEN).1
	@$(rm) $(MAN_PATH)/man1/$(ADD).1
	@$(rm) $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Removed man pages!"

man: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)
	@mkdir -p $(MANDIR)
	@help2man $(BINDIR)/$(AGENT) -o $(MANDIR)/$(AGENT).1 --name="OIDC token agent" -s 1 -N -i $(SRCDIR)/h2m/$(AGENT).h2m
	@help2man $(BINDIR)/$(GEN) -o $(MANDIR)/$(GEN).1 --name="generates account configurations for oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(GEN).h2m
	@help2man $(BINDIR)/$(ADD) -o $(MANDIR)/$(ADD).1 --name="adds account configurations to oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(ADD).h2m
	@help2man $(BINDIR)/$(CLIENT) -o $(MANDIR)/$(CLIENT).1 --name="gets OIDC access token from oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(CLIENT).h2m

	@echo "Created man pages"
deb:
	debuild -b -uc -us
	@echo "Success: DEBs are in parent directory"
	
srctar:
	#@(cd ..; tar cf $(BASENAME)/$(SRC_TAR) $(BASENAME)/src $(BASENAME)/Makefile)
	@tar cf $(SRC_TAR) src Makefile issuer.config LICENSE README.MD --transform='s_^_$(PKG_NAME)-$(VERSION)/_'


rpm: srctar
	@mkdir -p rpm/rpmbuild/SOURCES
	#@cp -af src Makefile  rpm/rpmbuild/SOURCES
	@mv oidc-agent.tar rpm/rpmbuild/SOURCES/
	rpmbuild --define "_topdir $(BASEDIR)/rpm/rpmbuild" -bb  rpm/oidc-agent.spec
	@mv rpm/rpmbuild/RPMS/*/*rpm ..
	@echo "Success: RPMs are in parent directory"
