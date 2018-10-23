# project name (generate executable with this name)
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token

VERSION   ?= 2.0.0
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
LFLAGS   = -lcurl -lsodium -lmicrohttpd -lseccomp
LFLAGS_CLIENT = -L$(APILIB) -loidc-agent

INSTALL_PATH ?=/usr
MAN_PATH     ?=/usr/share/man
CONFIG_PATH  ?=/etc

SRC_SOURCES := $(shell find $(SRCDIR) -name "*.c")
LIB_SOURCES := $(LIBDIR)/cJSON/cJSON.c $(LIBDIR)/list/src/list.c $(LIBDIR)/list/src/list_iterator.c $(LIBDIR)/list/src/list_node.c  
SOURCES  := $(SRC_SOURCES) $(LIB_SOURCES)
INCLUDES := $(shell find $(SRCDIR) -name "*.h") $(LIBDIR)/cJSON/cJSON.h $(LIBDIR)/list/src/list.h 
OBJECTS  := $(SRC_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS := $(filter-out $(OBJDIR)/$(ADD).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
GEN_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(ADD).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
ADD_OBJECTS := $(filter-out $(OBJDIR)/$(AGENT).o $(OBJDIR)/$(GEN).o $(OBJDIR)/$(CLIENT).o, $(OBJECTS))
CLIENT_OBJECTS := $(OBJDIR)/$(CLIENT).o 
API_OBJECTS := $(OBJDIR)/api.o $(OBJDIR)/ipc/ipc.o $(OBJDIR)/ipc/communicator.o $(OBJDIR)/json.o $(OBJDIR)/utils/memory.o $(OBJDIR)/utils/stringUtils.o  $(OBJDIR)/utils/colors.o $(OBJDIR)/utils/printer.o $(OBJDIR)/utils/listUtils.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

.PHONY: all
all: dependencies build man oidcdir

.PHONY: oidcdir
oidcdir:
	@[ -d ~/.config ] && mkdir -p ~/.config/oidc-agent || mkdir -p ~/.oidc-agent
	@[ -d ~/.config ] && touch ~/.config/oidc-agent/$(PROVIDERCONFIG) || touch ~/.oidc-agent/$(PROVIDERCONFIG)
	@echo "Created oidc dir"

.PHONY: dependencies
dependencies: 
	@git submodule init
	@git submodule update
	@echo "Dependecies OK"

.PHONY: create_obj_dir_structure
create_obj_dir_structure: $(OBJDIR) dependencies
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;

.PHONY: build
build: dependencies create_obj_dir_structure $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

.PHONY: install
install: install_man
	@install -D $(BINDIR)/$(AGENT) $(INSTALL_PATH)/bin/$(AGENT)
	@install -D $(BINDIR)/$(GEN) $(INSTALL_PATH)/bin/$(GEN)
	@install -D $(BINDIR)/$(ADD) $(INSTALL_PATH)/bin/$(ADD)
	@install -D $(BINDIR)/$(CLIENT) $(INSTALL_PATH)/bin/$(CLIENT)
	@install -m 644 -D $(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG)
	@echo "Installation complete!"

.PHONY: install_man
install_man: man
	@install -D $(MANDIR)/$(AGENT).1 $(MAN_PATH)/man1/$(AGENT).1
	@install -D $(MANDIR)/$(GEN).1 $(MAN_PATH)/man1/$(GEN).1
	@install -D $(MANDIR)/$(ADD).1 $(MAN_PATH)/man1/$(ADD).1
	@install -D $(MANDIR)/$(CLIENT).1 $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Installed man pages!"


$(BINDIR)/$(AGENT): create_obj_dir_structure $(AGENT_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(AGENT_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(GEN): create_obj_dir_structure $(GEN_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(GEN_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(ADD): create_obj_dir_structure $(ADD_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(ADD_OBJECTS) $(LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(CLIENT): create_obj_dir_structure $(CLIENT_OBJECTS) api
	@mkdir -p $(BINDIR)
	@$(LINKER) $(CLIENT_OBJECTS) $(LFLAGS_CLIENT) -o $@
	@echo "Linking "$@" complete!"

$(OBJDIR):
	@mkdir -p $(OBJDIR)


$(OBJDIR)/$(CLIENT).o : api
$(OBJDIR)/%.o : $(SRCDIR)/%.c create_obj_dir_structure
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\"
	@echo "Compiled "$<" successfully!"

$(OBJDIR)/%.o : $(LIBDIR)/%.c create_obj_dir_structure
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\"
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) -r $(OBJDIR)
	@$(rm) -r debian/.debhelper
	@$(rm) -r rpm/rpmbuild

.PHONY: distclean
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

.PHONY: uninstall_man
uninstall_man:
	@$(rm) $(MAN_PATH)/man1/$(AGENT).1
	@$(rm) $(MAN_PATH)/man1/$(GEN).1
	@$(rm) $(MAN_PATH)/man1/$(ADD).1
	@$(rm) $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Removed man pages!"

.PHONY: man
man: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)
	@mkdir -p $(MANDIR)
	@help2man $(BINDIR)/$(AGENT) -o $(MANDIR)/$(AGENT).1 --name="OIDC token agent" -s 1 -N -i $(SRCDIR)/h2m/$(AGENT).h2m
	@help2man $(BINDIR)/$(GEN) -o $(MANDIR)/$(GEN).1 --name="generates account configurations for oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(GEN).h2m
	@help2man $(BINDIR)/$(ADD) -o $(MANDIR)/$(ADD).1 --name="adds account configurations to oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(ADD).h2m
	@help2man $(BINDIR)/$(CLIENT) -o $(MANDIR)/$(CLIENT).1 --name="gets OIDC access token from oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(CLIENT).h2m
	@echo "Created man pages"

.PHONY: deb
deb:
	debuild -b -uc -us
	@echo "Success: DEBs are in parent directory"
	
.PHONY: srctar
srctar:
	#@(cd ..; tar cf $(BASENAME)/$(SRC_TAR) $(BASENAME)/src $(BASENAME)/Makefile)
	@tar cf $(SRC_TAR) src Makefile issuer.config LICENSE README.MD --transform='s_^_$(PKG_NAME)-$(VERSION)/_'

.PHONY: rpm
rpm: srctar
	@mkdir -p rpm/rpmbuild/SOURCES
	#@cp -af src Makefile  rpm/rpmbuild/SOURCES
	@mv oidc-agent.tar rpm/rpmbuild/SOURCES/
	rpmbuild --define "_topdir $(BASEDIR)/rpm/rpmbuild" -bb  rpm/oidc-agent.spec
	@mv rpm/rpmbuild/RPMS/*/*rpm ..
	@echo "Success: RPMs are in parent directory"

.PHONY: api
api: dependencies create_obj_dir_structure $(OBJDIR)/api.o $(API_OBJECTS) $(LIBDIR)
	@mkdir -p $(APILIB)
	@ar -crs $(APILIB)/liboidc-agent.a $(API_OBJECTS)
	@cp $(SRCDIR)/api.h $(APILIB)/oidc-agent-api.h
	@cp $(SRCDIR)/ipc/ipc_values.h $(APILIB)/ipc_values.h
	# @cp $(SRCDIR)/oidc_error.h $(APILIB)/oidc_error.h
	@tar -zcvf ../liboidc-agent-$(VERSION).tar.gz $(APILIB)/*.h $(APILIB)/liboidc-agent.a
	@echo "Success: API-TAR is in parent directory"

.PHONY: cleanapi
cleanapi:
	@$(rm) -r $(APILIB)

.PHONY: gitbook
gitbook: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)
	@perl -0777 -pi -e 's/(\$$ $(GEN) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(GEN) --help"; $(BINDIR)\/$(GEN) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-gen.md
	@perl -0777 -pi -e 's/(\$$ $(ADD) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(ADD) --help"; $(BINDIR)\/$(ADD) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-add.md
	@perl -0777 -pi -e 's/(\$$ $(AGENT) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(AGENT) --help"; $(BINDIR)\/$(AGENT) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-agent.md
	@perl -0777 -pi -e 's/(\$$ $(CLIENT) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(CLIENT) --help"; $(BINDIR)\/$(CLIENT) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-token.md
	@echo "Updated gitbook docu with help output"
