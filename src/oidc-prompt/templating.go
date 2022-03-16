package main

import (
	"embed"
	"fmt"
	"io/fs"
	"os"
	"strings"

	"github.com/cbroglie/mustache"
	"github.com/webview/webview"
)

//go:embed html
var _htmlFiles embed.FS

func internalTemplate(w webview.WebView, path string, bindData interface{}) {
	path = fmt.Sprintf("html/sites/%s.mustache", path)
	data, err := _htmlFiles.ReadFile(path)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
		os.Exit(1)
	}
	html, err := mustache.RenderInLayoutPartials(string(data), layoutData, staticPartialProvider, bindData)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
		os.Exit(1)
	}
	dataURL := getDataURL([]byte(html))
	w.Navigate(dataURL)
}

var layoutData string
var staticPartialProvider mustache.PartialProvider

func init() {
	layoutD, err := _htmlFiles.ReadFile("html/layouts/main.mustache")
	if err != nil {
		panic(err)
	}
	layoutData = string(layoutD)
	pp := &mustache.StaticProvider{
		Partials: make(map[string]string),
	}
	if err = fs.WalkDir(_htmlFiles, "html", func(path string, d fs.DirEntry, err error) error {
		if d.IsDir() {
			return nil
		}
		content, err := _htmlFiles.ReadFile(path)
		if err != nil {
			return err
		}
		pp.Partials[path] = string(content)
		name := d.Name()
		if strings.HasSuffix(name, ".mustache") {
			name = name[:len(name)-9]
		}
		pp.Partials[name] = string(content)
		return nil
	}); err != nil {
		panic(err)
	}
	staticPartialProvider = pp
}
