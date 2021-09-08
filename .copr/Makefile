srpm:
	@echo "cwd: $(shell pwd)"
	@echo "ls: $(shell ls -la|sed s/$$/\\\\n/g)"
	(cd ..; pwd; make rpmsource)
