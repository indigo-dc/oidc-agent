# Executable names
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token

# Version information
VERSION   ?= 2.0.3
LIBMAJORVERSION ?= 2
LIBMINORVERSION ?= 0
# Generated lib version / name
LIBVERSION = $(LIBMAJORVERSION).$(LIBMINORVERSION)
SHARED_LIB_NAME = liboidc-agent.so.$(LIBMAJORVERSION)
SONAME = liboidc-agent.so.$(LIBMAJORVERSION)

# These are needed for the RPM build target:
BASEDIR   = $(PWD)
BASENAME := $(notdir $(PWD))
SRC_TAR   = oidc-agent.tar
PKG_NAME  = oidc-agent

# Local dir names
SRCDIR   = src
OBJDIR   = obj
PICOBJDIR= pic-obj
BINDIR   = bin
LIBDIR   = lib
APILIB   = $(LIBDIR)/api
MANDIR 	 = man
CONFDIR  = config
PROVIDERCONFIG = issuer.config

# Compiler options
CC       = gcc
# compiling flags here
CFLAGS   = -g -std=c99 -I$(SRCDIR) -I$(LIBDIR) #-Wall -Wextra 

# Linker options
LINKER   = gcc
LFLAGS   = -lsodium -lseccomp
AGENT_LFLAGS = $(LFLAGS) -lcurl -lmicrohttpd
GEN_LFLAGS = $(LFLAGS) -lmicrohttpd
ADD_LFLAGS = $(LFLAGS)
CLIENT_LFLAGS = -loidc-agent -lseccomp

# Install paths
BIN_PATH             ?=/usr
LIB_PATH 	           ?=/usr/lib
MAN_PATH             ?=/usr/share/man
CONFIG_PATH          ?=/etc
BASH_COMPLETION_PATH ?=/usr/share/bash-completion/completions

# Define sources
SRC_SOURCES := $(shell find $(SRCDIR) -name "*.c")
LIB_SOURCES := $(LIBDIR)/cJSON/cJSON.c $(LIBDIR)/list/list.c $(LIBDIR)/list/list_iterator.c $(LIBDIR)/list/list_node.c  
SOURCES  := $(SRC_SOURCES) $(LIB_SOURCES)
INCLUDES := $(shell find $(SRCDIR) -name "*.h") $(LIBDIR)/cJSON/cJSON.h $(LIBDIR)/list/list.h 

GENERAL_SOURCES := $(shell find $(SRCDIR)/utils -name "*.c") $(shell find $(SRCDIR)/account -name "*.c") $(shell find $(SRCDIR)/ipc -name "*.c") $(shell find $(SRCDIR)/privileges -name "*.c") 
AGENT_SOURCES := $(shell find $(SRCDIR)/$(AGENT) -name "*.c")
GEN_SOURCES := $(shell find $(SRCDIR)/$(GEN) -name "*.c")
ADD_SOURCES := $(shell find $(SRCDIR)/$(ADD) -name "*.c")
CLIENT_SOURCES := $(shell find $(SRCDIR)/$(CLIENT) -name "*.c")

# Define objects
ALL_OBJECTS  := $(SRC_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS  := $(AGENT_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
GEN_OBJECTS  := $(GEN_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/oidc-agent/httpserver/termHttpserver.o $(OBJDIR)/oidc-agent/httpserver/running_server.o $(OBJDIR)/oidc-agent/oidc/device_code.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
ADD_OBJECTS  := $(ADD_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
CLIENT_OBJECTS := $(OBJDIR)/$(CLIENT)/$(CLIENT).o $(OBJDIR)/privileges/privileges.o $(OBJDIR)/privileges/token_privileges.o $(OBJDIR)/utils/file_io/file_io.o $(OBJDIR)/utils/disableTracing.o
API_OBJECTS := $(OBJDIR)/$(CLIENT)/api.o $(OBJDIR)/ipc/ipc.o $(OBJDIR)/ipc/communicator.o $(OBJDIR)/utils/json.o $(OBJDIR)/utils/memory.o $(OBJDIR)/utils/stringUtils.o  $(OBJDIR)/utils/colors.o $(OBJDIR)/utils/printer.o $(OBJDIR)/utils/listUtils.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
PIC_OBJECTS := $(API_OBJECTS:$(OBJDIR)/%=$(PICOBJDIR)/%)
rm       = rm -f

# RULES

.PHONY: all
all: build man

# Compiling

.PHONY: build
build: create_obj_dir_structure $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

## pull in dependency info for *existing* .o files
-include $(ALL_OBJECTS:.o=.d)

## Compile and generate depencency info
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

## Compile lib sources 
$(OBJDIR)/%.o : $(LIBDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

## Compile position independent code
$(PICOBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -fpic -c $< -o $@ -DVERSION=\"$(VERSION)\" -DCONFIG_PATH=\"$(CONFIG_PATH)\"
	@echo "Compiled "$<" with pic successfully!"

$(PICOBJDIR)/%.o : $(LIBDIR)/%.c
	@$(CC) $(CFLAGS) -fpic -c $< -o $@
	@echo "Compiled "$<" with pic successfully!"


# Linking

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

# Phony Installer

.PHONY: install
install: install_bin install_man install_conf install_bash install_priv 
	@echo "Installation complete!"

.PHONY: install_bin
install_bin: $(BIN_PATH)/bin/$(AGENT) $(BIN_PATH)/bin/$(GEN) $(BIN_PATH)/bin/$(ADD) $(BIN_PATH)/bin/$(CLIENT)
	@echo "Installed binaries"

.PHONY: install_conf
install_conf: $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG)
	@echo "Installed issuer.config"
	
.PHONY: install_priv
install_priv: $(CONFDIR)/privileges/
	@install -d $(CONFIG_PATH)/oidc-agent/privileges/
	@install -m 644 -D $(CONFDIR)/privileges/* $(CONFIG_PATH)/oidc-agent/privileges/
	@echo "installed privileges files"

.PHONY: install_bash
install_bash: $(BASH_COMPLETION_PATH)/$(AGENT) $(BASH_COMPLETION_PATH)/$(GEN) $(BASH_COMPLETION_PATH)/$(ADD) $(BASH_COMPLETION_PATH)/$(CLIENT)
	@echo "Installed bash completion"

.PHONY: install_man
install_man: $(MANDIR)/$(AGENT).1 $(MANDIR)/$(GEN).1 $(MANDIR)/$(ADD).1 $(MANDIR)/$(CLIENT).1
	@echo "Installed man pages!"

.PHONY: install_lib
install_lib: $(LIB_PATH)/$(SHARED_LIB_NAME) #TODO
	@echo "Installed library"

.PHONY: install_lib-dev
install_lib-dev: $(LIB_PATH)/$(SHARED_LIB_NAME) $(LIB_PATH)/liboidc-agent.a #TODO symlinks, include
	@echo "Installed library dev"


# Install files
## Binaries
$(BIN_PATH)/bin/$(AGENT): $(BINDIR)/$(AGENT)
	@install -D $< $@

$(BIN_PATH)/bin/$(GEN): $(BINDIR)/$(GEN)
	@install -D $< $@

$(BIN_PATH)/bin/$(ADD): $(BINDIR)/$(ADD)
	@install -D $< $@

$(BIN_PATH)/bin/$(CLIENT): $(BINDIR)/$(CLIENT)
	@install -D $< $@

## Config
$(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG): $(CONFDIR)/$(PROVIDERCONFIG)
	@install -m 644 -D $< $@ 

## Bash completion
$(BASH_COMPLETION_PATH)/$(AGENT): $(CONFDIR)/bash-completion/oidc-agent
	@install -d $(BASH_COMPLETION_PATH)/
	@install -m 744 -D $< $@

$(BASH_COMPLETION_PATH)/$(GEN): $(BASH_COMPLETION_PATH)/$(AGENT)
	@ln -s $< $@

$(BASH_COMPLETION_PATH)/$(ADD): $(BASH_COMPLETION_PATH)/$(AGENT)
	@ln -s $< $@

$(BASH_COMPLETION_PATH)/$(CLIENT): $(BASH_COMPLETION_PATH)/$(AGENT)
	@ln -s $< $@

## Man pages
$(MAN_PATH)/man1/$(AGENT).1: $(MANDIR)/$(AGENT).1
	@install -D $< $@
$(MAN_PATH)/man1/$(GEN).1: $(MANDIR)/$(GEN).1
	@install -D $< $@
$(MAN_PATH)/man1/$(ADD).1: $(MANDIR)/$(ADD).1
	@install -D $< $@
$(MAN_PATH)/man1/$(CLIENT).1: $(MANDIR)/$(CLIENT).1
	@install -D $< $@

## Lib
$(LIB_PATH)/$(SHARED_LIB_NAME): $(APILIB)/$(SHARED_LIB_NAME)
	@install -D $< $@

$(LIB_PATH)/liboidc-agent.a: $(APILIB)/liboidc-agent.a
	@install -D $< $@

# Man pages

.PHONY: create_man
create_man: $(MANDIR)/$(AGENT).1 $(MANDIR)/$(GEN).1 $(MANDIR)/$(ADD).1 $(MANDIR)/$(CLIENT).1
	@echo "Created man pages"
$(MANDIR)/$(AGENT).1: $(MANDIR) $(BINDIR)/$(AGENT) $(SRCDIR)/h2m/$(AGENT).h2m
	@help2man $(BINDIR)/$(AGENT) -o $(MANDIR)/$(AGENT).1 --name="OIDC token agent" -s 1 -N -i $(SRCDIR)/h2m/$(AGENT).h2m

$(MANDIR)/$(GEN).1: $(MANDIR) $(BINDIR)/$(GEN) $(SRCDIR)/h2m/$(GEN).h2m
	@help2man $(BINDIR)/$(GEN) -o $(MANDIR)/$(GEN).1 --name="generates account configurations for oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(GEN).h2m

$(MANDIR)/$(ADD).1: $(MANDIR) $(BINDIR)/$(ADD) $(SRCDIR)/h2m/$(ADD).h2m
	@help2man $(BINDIR)/$(ADD) -o $(MANDIR)/$(ADD).1 --name="adds account configurations to oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(ADD).h2m

$(MANDIR)/$(CLIENT).1: $(MANDIR) $(BINDIR)/$(CLIENT) $(SRCDIR)/h2m/$(CLIENT).h2m
	@help2man $(BINDIR)/$(CLIENT) -o $(MANDIR)/$(CLIENT).1 --name="gets OIDC access token from oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(CLIENT).h2m

# Library

../liboidc-agent-$(LIBVERSION).tar.gz: $(APILIB)/liboidc-agent.a $(APILIB)/oidc-agent-api.h $(APILIB)/ipc_values.h 
	@tar -zcvf $@ $(APILIB)/*.h $(APILIB)/liboidc-agent.a
	@echo "Success: API-TAR is in parent directory"

$(APILIB)/liboidc-agent.a: $(APILIB) $(API_OBJECTS)
	@ar -crs $@ $(API_OBJECTS)

$(APILIB)/oidc-agent-api.h:$(SRCDIR)/$(CLIENT)/api.h
	@cp $(SRCDIR)/$(CLIENT)/api.h $@

$(APILIB)/ipc_values.h:$(SRCDIR)/ipc/ipc_values.h
	@cp $(SRCDIR)/ipc/ipc_values.h $@

$(APILIB)/oidc_error.h:$(SRCDIR)/oidc_error.h
	@cp $(SRCDIR)/utils/oidc_error.h $@

$(APILIB)/$(SHARED_LIB_NAME): $(APILIB) $(PIC_OBJECTS)
	@gcc -shared -fpic -Wl,-soname,$(SONAME) -o $@ $(PIC_OBJECTS) -lc

.PHONY: shared_lib
shared_lib: create_picobj_dir_structure $(APILIB)/$(SHARED_LIB_NAME)
	@echo "Created shared library"



# Helpers

$(MANDIR):
	@mkdir -p $(MANDIR)

$(APILIB):
	@mkdir -p $(APILIB)
$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(PICOBJDIR):
	@mkdir -p $(PICOBJDIR)

.PHONY: create_obj_dir_structure
create_obj_dir_structure: $(OBJDIR) 
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;

.PHONY: create_picobj_dir_structure
create_picobj_dir_structure: $(PICOBJDIR) 
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(PICOBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(PICOBJDIR)/{} \;

# Cleaners

.PHONY: clean
clean:
	@$(rm) -r $(OBJDIR)
	@$(rm) -r debian/.debhelper
	@$(rm) -r rpm/rpmbuild

.PHONY: distclean
distclean: clean
	@$(rm) -r $(BINDIR)
	@$(rm) -r $(MANDIR)

.PHONY: cleanapi
cleanapi:
	@$(rm) -r $(APILIB)

.PHONY: remove
remove: distclean

# Uninstall

.PHONY: uninstall
uninstall: uninstall_man
	@$(rm) $(BIN_PATH)/bin/$(AGENT)
	@echo "Uninstalled "$(AGENT)
	@$(rm) $(BIN_PATH)/bin/$(GEN)
	@echo "Uninstalled "$(GEN)
	@$(rm) $(BIN_PATH)/bin/$(ADD)
	@echo "Uninstalled "$(ADD)
	@$(rm) $(BIN_PATH)/bin/$(CLIENT)
	@echo "Uninstalled "$(CLIENT)

.PHONY: uninstall_man
uninstall_man:
	@$(rm) $(MAN_PATH)/man1/$(AGENT).1
	@$(rm) $(MAN_PATH)/man1/$(GEN).1
	@$(rm) $(MAN_PATH)/man1/$(ADD).1
	@$(rm) $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Removed man pages!"

# Packaging

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

# Release

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

