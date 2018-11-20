# project name (generate executable with this name)
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token

VERSION   ?= 2.0.2
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
CONFDIR  = config
PROVIDERCONFIG = issuer.config

CC       = gcc
# compiling flags here
CFLAGS   = -g -std=c99 -I$(SRCDIR) -I$(LIBDIR) #-Wall -Wextra 

LINKER   = gcc
# linking flags here
LFLAGS   = -lsodium -lseccomp
AGENT_LFLAGS = $(LFLAGS) -lcurl -lmicrohttpd
GEN_LFLAGS = $(LFLAGS) -lmicrohttpd
ADD_LFLAGS = $(LFLAGS)
CLIENT_LFLAGS = -L$(APILIB) -loidc-agent -lseccomp

INSTALL_PATH ?=/usr
MAN_PATH     ?=/usr/share/man
CONFIG_PATH  ?=/etc
BASH_COMPLETION_PATH ?=/usr/share/bash-completion/completions

SRC_SOURCES := $(shell find $(SRCDIR) -name "*.c")
LIB_SOURCES := $(LIBDIR)/cJSON/cJSON.c $(LIBDIR)/list/list.c $(LIBDIR)/list/list_iterator.c $(LIBDIR)/list/list_node.c  
SOURCES  := $(SRC_SOURCES) $(LIB_SOURCES)
INCLUDES := $(shell find $(SRCDIR) -name "*.h") $(LIBDIR)/cJSON/cJSON.h $(LIBDIR)/list/list.h 

GENERAL_SOURCES := $(shell find $(SRCDIR)/utils -name "*.c") $(shell find $(SRCDIR)/account -name "*.c") $(shell find $(SRCDIR)/ipc -name "*.c") $(shell find $(SRCDIR)/privileges -name "*.c") 
AGENT_SOURCES := $(shell find $(SRCDIR)/$(AGENT) -name "*.c")
GEN_SOURCES := $(shell find $(SRCDIR)/$(GEN) -name "*.c")
ADD_SOURCES := $(shell find $(SRCDIR)/$(ADD) -name "*.c")
CLIENT_SOURCES := $(shell find $(SRCDIR)/$(CLIENT) -name "*.c")

ALL_OBJECTS  := $(SRC_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS  := $(AGENT_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
GEN_OBJECTS  := $(GEN_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/oidc-agent/httpserver/termHttpserver.o $(OBJDIR)/oidc-agent/httpserver/running_server.o $(OBJDIR)/oidc-agent/oidc/device_code.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
ADD_OBJECTS  := $(ADD_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
CLIENT_OBJECTS := $(OBJDIR)/$(CLIENT)/$(CLIENT).o $(OBJDIR)/privileges/privileges.o $(OBJDIR)/privileges/token_privileges.o $(OBJDIR)/utils/file_io/file_io.o
API_OBJECTS := $(OBJDIR)/$(CLIENT)/api.o $(OBJDIR)/ipc/ipc.o $(OBJDIR)/ipc/communicator.o $(OBJDIR)/utils/json.o $(OBJDIR)/utils/memory.o $(OBJDIR)/utils/stringUtils.o  $(OBJDIR)/utils/colors.o $(OBJDIR)/utils/printer.o $(OBJDIR)/utils/listUtils.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f


.PHONY: all
all: build man

.PHONY: create_obj_dir_structure
create_obj_dir_structure: $(OBJDIR) 
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;

.PHONY: build
build: create_obj_dir_structure $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

.PHONY: install
install: install_man
	@install -D $(BINDIR)/$(AGENT) $(INSTALL_PATH)/bin/$(AGENT)
	@install -D $(BINDIR)/$(GEN) $(INSTALL_PATH)/bin/$(GEN)
	@install -D $(BINDIR)/$(ADD) $(INSTALL_PATH)/bin/$(ADD)
	@install -D $(BINDIR)/$(CLIENT) $(INSTALL_PATH)/bin/$(CLIENT)
	@install -m 644 -D $(CONFDIR)/$(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG)
	@install -d $(CONFIG_PATH)/oidc-agent/privileges/
	@install -m 644 -D $(CONFDIR)/privileges/* $(CONFIG_PATH)/oidc-agent/privileges/
	@install -d $(BASH_COMPLETION_PATH)/
	@install -m 744 -D $(CONFDIR)/bash-completion/oidc-agent $(BASH_COMPLETION_PATH)/
	@ln -s $(BASH_COMPLETION_PATH)/$(AGENT) $(BASH_COMPLETION_PATH)/$(GEN)
	@ln -s $(BASH_COMPLETION_PATH)/$(AGENT) $(BASH_COMPLETION_PATH)/$(ADD)
	@ln -s $(BASH_COMPLETION_PATH)/$(AGENT) $(BASH_COMPLETION_PATH)/$(CLIENT)
	@echo "Installation complete!"

.PHONY: install_man
install_man: create_man
	@install -D $(MANDIR)/$(AGENT).1 $(MAN_PATH)/man1/$(AGENT).1
	@install -D $(MANDIR)/$(GEN).1 $(MAN_PATH)/man1/$(GEN).1
	@install -D $(MANDIR)/$(ADD).1 $(MAN_PATH)/man1/$(ADD).1
	@install -D $(MANDIR)/$(CLIENT).1 $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Installed man pages!"


$(BINDIR)/$(AGENT): create_obj_dir_structure $(AGENT_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(AGENT_OBJECTS) $(AGENT_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(GEN): create_obj_dir_structure $(GEN_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(GEN_OBJECTS) $(GEN_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(ADD): create_obj_dir_structure $(ADD_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LINKER) $(ADD_OBJECTS) $(ADD_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(CLIENT): create_obj_dir_structure $(CLIENT_OBJECTS) $(APILIB)/liboidc-agent.a $(APILIB)/oidc-agent-api.h
	@mkdir -p $(BINDIR)
	@$(LINKER) $(CLIENT_OBJECTS) $(CLIENT_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(OBJDIR):
	@mkdir -p $(OBJDIR)

# pull in dependency info for *existing* .o files
-include $(ALL_OBJECTS:.o=.d)

# Compile and generate depencency info
$(OBJDIR)/$(CLIENT)/$(CLIENT).o : $(APILIB)/liboidc-agent.a $(APILIB)/oidc-agent-api.h
$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\" -DCONFIG_PATH=\"$(CONFIG_PATH)\"
	@# Create dependency infos
	@{ \
	set -e ;\
	depFileName=$(OBJDIR)/$*.d ;\
	$(CC) -MM $(CFLAGS) $< -o $${depFileName} ;\
	mv -f $${depFileName} $${depFileName}.tmp ;\
	sed -e 's|.*:|$@:|' < $${depFileName}.tmp > $${depFileName} ;\
	cp -f $${depFileName} $${depFileName}.tmp ;\
	sed -e 's/.*://' -e 's/\\$$//' < $${depFileName}.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $${depFileName} ;\
	rm -f $${depFileName}.tmp ;\
	}
	@echo "Compiled "$<" successfully!"

# Compile lib sources 
$(OBJDIR)/%.o : $(LIBDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
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
remove: distclean

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

.PHONY: create_man
create_man: $(MANDIR)/$(AGENT).1 $(MANDIR)/$(GEN).1 $(MANDIR)/$(ADD).1 $(MANDIR)/$(CLIENT).1
	@echo "Created man pages"

$(MANDIR):
	@mkdir -p $(MANDIR)

$(MANDIR)/$(AGENT).1: $(MANDIR) $(BINDIR)/$(AGENT) $(SRCDIR)/h2m/$(AGENT).h2m
	@help2man $(BINDIR)/$(AGENT) -o $(MANDIR)/$(AGENT).1 --name="OIDC token agent" -s 1 -N -i $(SRCDIR)/h2m/$(AGENT).h2m

$(MANDIR)/$(GEN).1: $(MANDIR) $(BINDIR)/$(GEN) $(SRCDIR)/h2m/$(GEN).h2m
	@help2man $(BINDIR)/$(GEN) -o $(MANDIR)/$(GEN).1 --name="generates account configurations for oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(GEN).h2m

$(MANDIR)/$(ADD).1: $(MANDIR) $(BINDIR)/$(ADD) $(SRCDIR)/h2m/$(ADD).h2m
	@help2man $(BINDIR)/$(ADD) -o $(MANDIR)/$(ADD).1 --name="adds account configurations to oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(ADD).h2m

$(MANDIR)/$(CLIENT).1: $(MANDIR) $(BINDIR)/$(CLIENT) $(SRCDIR)/h2m/$(CLIENT).h2m
	@help2man $(BINDIR)/$(CLIENT) -o $(MANDIR)/$(CLIENT).1 --name="gets OIDC access token from oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(CLIENT).h2m


.PHONY: deb
deb: create_obj_dir_structure
	debuild -b -uc -us
	@echo "Success: DEBs are in parent directory"
	
.PHONY: srctar
srctar:
	@#@(cd ..; tar cf $(BASENAME)/$(SRC_TAR) $(BASENAME)/src $(BASENAME)/Makefile)
	@tar cf $(SRC_TAR) src Makefile issuer.config LICENSE README.MD --transform='s_^_$(PKG_NAME)-$(VERSION)/_'

.PHONY: rpm
rpm: srctar
	@mkdir -p rpm/rpmbuild/SOURCES
	@#@cp -af src Makefile  rpm/rpmbuild/SOURCES
	@mv oidc-agent.tar rpm/rpmbuild/SOURCES/
	rpmbuild --define "_topdir $(BASEDIR)/rpm/rpmbuild" -bb  rpm/oidc-agent.spec
	@mv rpm/rpmbuild/RPMS/*/*rpm ..
	@echo "Success: RPMs are in parent directory"

../liboidc-agent-$(VERSION).tar.gz: $(APILIB)/liboidc-agent.a $(APILIB)/oidc-agent-api.h $(APILIB)/ipc_values.h 
	@tar -zcvf ../liboidc-agent-$(VERSION).tar.gz $(APILIB)/*.h $(APILIB)/liboidc-agent.a
	@echo "Success: API-TAR is in parent directory"

$(APILIB):
	@mkdir -p $(APILIB)

$(APILIB)/liboidc-agent.a: $(APILIB) $(API_OBJECTS)
	@ar -crs $(APILIB)/liboidc-agent.a $(API_OBJECTS)

$(APILIB)/oidc-agent-api.h:$(SRCDIR)/$(CLIENT)/api.h
	@cp $(SRCDIR)/$(CLIENT)/api.h $(APILIB)/oidc-agent-api.h

$(APILIB)/ipc_values.h:$(SRCDIR)/ipc/ipc_values.h
	@cp $(SRCDIR)/ipc/ipc_values.h $(APILIB)/ipc_values.h

$(APILIB)/oidc_error.h:$(SRCDIR)/oidc_error.h
	@cp $(SRCDIR)/utils/oidc_error.h $(APILIB)/oidc_error.h

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

.PHONY: agent-lib
agent-lib: ../liboidc-agent-$(VERSION).tar.gz

.PHONY: release
release: agent-lib deb gitbook

