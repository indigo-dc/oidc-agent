# project name (generate executable with this name)
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token

VERSION   ?= 1.3.0
# These are needed for the RPM build target:
BASEDIR   = $(PWD)
BASENAME := $(notdir $(PWD))
SRC_TAR   = oidc-agent.tar
PKG_NAME  = oidc-agent


SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
LIBDIR   = lib
APILIB   = $(LIBDIR)/api
MANDIR 	 = man
PROVIDERCONFIG = issuer.config

CC       = gcc
# compiling flags here
CFLAGS   = -g -std=c99 -I$(LIBDIR) #-Wall -Wextra 

LINKER   = gcc
# linking flags here
LFLAGS   = -lcurl -lsodium -L$(LIBDIR)/jsmn -ljsmn -L$(LIBDIR)/list/build -llist -lmicrohttpd 

INSTALL_PATH ?=/usr
MAN_PATH     ?=/usr/share/man
CONFIG_PATH  ?=/etc

SOURCES  := $(shell find $(SRCDIR) -name "*.c")
INCLUDES := $(shell find $(SRCDIR) -name "*.h")
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS := $(filter-out $(OBJDIR)/$(ADD).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
GEN_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(ADD).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
ADD_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
CLIENT_OBJECTS := $(OBJDIR)/$(CLIENT).o $(OBJDIR)/utils/cleaner.o
API_OBJECTS := $(OBJDIR)/api.o $(OBJDIR)/ipc/ipc.o $(OBJDIR)/ipc/communicator.o $(OBJDIR)/json.o $(OBJDIR)/utils/cleaner.o $(OBJDIR)/utils/stringUtils.o  $(OBJDIR)/utils/colors.o $(OBJDIR)/utils/listUtils.o
rm       = rm -f

all: dependencies build man oidcdir

oidcdir:
	@[ -d ~/.config ] && mkdir -p ~/.config/oidc-agent || mkdir -p ~/.oidc-agent
	@[ -d ~/.config ] && touch ~/.config/oidc-agent/$(PROVIDERCONFIG) || touch ~/.oidc-agent/$(PROVIDERCONFIG)
	@echo "Created oidc dir"

dependencies: 
	@[ -d $(LIBDIR)/jsmn ] || git clone https://github.com/zserge/jsmn.git $(LIBDIR)/jsmn 
	@[ -f $(LIBDIR)/jsmn/libjsmn.a ] || (cd $(LIBDIR)/jsmn && make)
	@git submodule init
	@git submodule update
	@cd $(LIBDIR)/list && make
	@echo "Dependecies OK"

copy_src_dir_structure:
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;

build: dependencies copy_src_dir_structure $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

install: install_man
	@install -D $(BINDIR)/$(AGENT) $(INSTALL_PATH)/bin/$(AGENT)
	@install -D $(BINDIR)/$(GEN) $(INSTALL_PATH)/bin/$(GEN)
	@install -D $(BINDIR)/$(ADD) $(INSTALL_PATH)/bin/$(ADD)
	@install -D $(BINDIR)/$(CLIENT) $(INSTALL_PATH)/bin/$(CLIENT)
	@install -m 644 -D $(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG)
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

$(BINDIR)/$(CLIENT): copy_src_dir_structure $(CLIENT_OBJECTS) api
	@mkdir -p $(BINDIR)
	@$(LINKER) $(CLIENT_OBJECTS) $(LFLAGS) -L$(APILIB) -loidc-agent -o $@
	@echo "Linking "$@" complete!"

$(OBJDIR):
	@mkdir -p $(OBJDIR)

#$(OBJECTS): $(OBJDIR)/%.o

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(OBJDIR)
ifndef NO_COLOR
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\"
else
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\" -DNO_COLOR
endif
	@echo "Compiled "$<" successfully!"

$(OBJDIR)/$(CLIENT).o: api $(OBJDIR) $(SRCDIR)/$(CLIENT).c
ifndef NO_COLOR
	@$(CC) $(CFLAGS) -c $(SRCDIR)/$(CLIENT).c -o $(OBJDIR)/$(CLIENT).o -DVERSION=\"$(VERSION)\"
else
	@$(CC) $(CFLAGS) -c $(SRCDIR)/$(CLIENT).c -o $(OBJDIR)/$(CLIENT).o -DVERSION=\"$(VERSION)\" -DNO_COLOR
endif

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
	# @$(rm) -r $(CONFIG_PATH)/oidc-agent/

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

api: dependencies copy_src_dir_structure $(OBJDIR)/api.o $(API_OBJECTS) $(LIBIDR)
	@mkdir -p $(APILIB)
	@ar -crs $(APILIB)/liboidc-agent-pre.a $(API_OBJECTS)
	@ar -M <makelib.mri
	# @ranlib $(APILIB)/liboidc-agent.a
	@cp $(SRCDIR)/api.h $(APILIB)/oidc-agent-api.h
	@cp $(SRCDIR)/ipc/ipc_values.h $(APILIB)/ipc_values.h
	# @cp $(SRCDIR)/oidc_error.h $(APILIB)/oidc_error.h
	@tar -zcvf ../liboidc-agent-$(VERSION).tar.gz $(APILIB)/*.h $(APILIB)/liboidc-agent.a
	@echo "Success: API-TAR is in parent directory"

cleanapi:
	@$(rm) -r $(APILIB)
