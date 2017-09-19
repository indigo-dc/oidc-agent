# project name (generate executable with this name)
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token

VERSION   = 1.1.1
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
CFLAGS   = -g -I$(LIBDIR) #-Wall -Wextra 

LINKER   = gcc
# linking flags here
LFLAGS   = -lcurl -lsodium -L$(LIBDIR)/jsmn -ljsmn

INSTALL_PATH?=/usr
MAN_PATH?=/usr/share/man

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS := $(filter-out $(OBJDIR)/$(ADD).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
GEN_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(ADD).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
ADD_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
CLIENT_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(ADD).o, $(OBJECTS))
rm       = rm -r -f

all: dependecies build man oidcdir

oidcdir:
	@[ -d ~/.config ] && mkdir -p ~/.config/oidc-agent || mkdir -p ~/.oidc-agent
	@[ -d ~/.config ] && cp $(PROVIDERCONFIG) ~/.config/oidc-agent/$(PROVIDERCONFIG) -n || cp $(PROVIDERCONFIG) ~/.oidc-agent/$(PROVIDERCONFIG) -n
	@echo "Created oidc dir"

dependecies: 
	@[ -d $(LIBDIR)/jsmn ] || git clone https://github.com/zserge/jsmn.git $(LIBDIR)/jsmn 
	@[ -f $(LIBDIR)/jsmn/libjsmn.a ] || (cd $(LIBDIR)/jsmn && make)
	@echo "Dependecies OK"

build: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

install: install_man
	@install -D $(BINDIR)/$(AGENT) $(INSTALL_PATH)/bin/$(AGENT)
	@install -D $(BINDIR)/$(GEN) $(INSTALL_PATH)/bin/$(GEN)
	@install -D $(BINDIR)/$(ADD) $(INSTALL_PATH)/bin/$(ADD)
	@install -D $(BINDIR)/$(CLIENT) $(INSTALL_PATH)/bin/$(CLIENT)
	@echo "Installation complete!"

install_man: man
	@install -D $(MANDIR)/$(AGENT).1 $(MAN_PATH)/man1/$(AGENT).1
	@install -D $(MANDIR)/$(GEN).1 $(MAN_PATH)/man1/$(GEN).1
	@install -D $(MANDIR)/$(ADD).1 $(MAN_PATH)/man1/$(ADD).1
	@install -D $(MANDIR)/$(CLIENT).1 $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Installed man pages!"


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
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\"
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJDIR)
	@$(rm) $(LIBDIR)
	@$(rm) -r debian/.debhelper
	@$(rm) -r rpm/rpmbuild

distclean: clean
	@$(rm) -r $(BINDIR)

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)
	@echo "Executable removed!"
	@$(rm) $(LIBDIR)
	@echo "Dependencies removed!"

.PHONY: uninstall
uninstall: uninstall_man
	@rm $(INSTALL_PATH)/bin/$(AGENT)
	@echo "Uninstalled "$(AGENT)
	@rm $(INSTALL_PATH)/bin/$(GEN)
	@echo "Uninstalled "$(GEN)
	@rm $(INSTALL_PATH)/bin/$(ADD)
	@echo "Uninstalled "$(ADD)
	@rm $(INSTALL_PATH)/bin/$(CLIENT)
	@echo "Uninstalled "$(CLIENT)

uninstall_man:
	@rm $(MAN_PATH)/man1/$(AGENT).1
	@rm $(MAN_PATH)/man1/$(GEN).1
	@rm $(MAN_PATH)/man1/$(ADD).1
	@rm $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Removed man pages!"

man: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)
	@mkdir -p $(MANDIR)
	@-help2man $(BINDIR)/$(AGENT) -o $(MANDIR)/$(AGENT).1 --name="OIDC token agent" -s 1 -N
	@-help2man $(BINDIR)/$(GEN) -o $(MANDIR)/$(GEN).1 --name="generates account configurations for oidc-agent" -s 1 -N
	@-help2man $(BINDIR)/$(ADD) -o $(MANDIR)/$(ADD).1 --name="adds account configurations to oidc-agent" -s 1 -N
	@-help2man $(BINDIR)/$(CLIENT) -o $(MANDIR)/$(CLIENT).1 --name="gets OIDC access token from oidc-agent" -s 1 -N
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
