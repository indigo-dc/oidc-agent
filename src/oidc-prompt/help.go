package main

import (
	"fmt"
	"os"
)

const VERSION = "2.0.0"

func usage() {
	fmt.Fprintln(os.Stderr, `Usage: oidc-prompt TYPE TITLE TEXT LABEL [INIT] [LIST_ELEMENTS ...]`)
}
func help() {
	usage()
	fmt.Fprintf(os.Stderr, `oidc-prompt -- An interface for prompting the user.

This tool is intended as an internal tool of oidc-agent. Different oidc-agent components use it to prompt
the user for information. The user should not call oidc-prompt.
`)
}
