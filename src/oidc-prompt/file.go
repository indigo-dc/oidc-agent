package main

import (
	"fmt"
	"os"

	"github.com/webview/webview"
)

func internalFile(w webview.WebView, path string) {
	html, err := _htmlFiles.ReadFile(path)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
		os.Exit(1)
	}
	dataURL := getDataURL(html)
	w.Navigate(dataURL)
}

func file(w webview.WebView, path string) {
	p := path
	if p[0] != '/' {
		var basedir string
		tmp := os.Getenv("OIDC_PROMPT_DIR")
		if tmp != "" {
			basedir = tmp
		}
		p = basedir + "/" + path
	}
	w.Navigate("file://" + p)
}
