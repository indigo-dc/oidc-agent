SRC_TAR  = oidc-agent-4.1.1.tar.gz
PKG_NAME = oidc-agent
VERSION  = 4.1.1
outdir  ?= /tmp
srpm:
	@echo "cwd: $(shell pwd)"
	@echo "ls: $(shell ls -la|sed s/$$/\\\\n/g)"

	#mkdir -p $(outdir)
	(cd ../..; \
		tar czf $(outdir)/$(SRC_TAR) \
			--exclude-vcs \
			--exclude=.pc \
			--exclude $(PGK_NAME)/config \
			--transform='s_${PKG_NAME}_${PKG_NAME}-$(VERSION)_' \
			$(PKG_NAME) \
		)
	@echo "ls: $(shell ls -la $(outdir)/ |sed s/$$/\\\\n/g)"
	@echo "ls: $(shell ls -la $(outdir)/$(SRC_TAR)/ |sed s/$$/\\\\n/g)"
