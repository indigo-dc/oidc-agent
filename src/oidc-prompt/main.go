package main

import (
	"encoding/base64"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"strconv"
	"strings"

	"github.com/webview/webview"
)

func getDataURL(html []byte) string {
	data := base64.StdEncoding.EncodeToString(html)
	return fmt.Sprintf("data:text/html;base64,%s", data)
}

func getArg(n int, dfault ...string) (arg string) {
	args := os.Args
	if len(dfault) > 0 {
		arg = dfault[0]
	}
	if len(args) > n {
		arg = args[n]
	}
	return
}

func bindFunctions(w webview.WebView) {
	w.Bind(
		"terminate", func() error {
			w.Terminate()
			return nil
		},
	)
	w.Bind(
		"print", func(data string) error {
			fmt.Println(data)
			return nil
		},
	)
	w.Bind(
		"openLink", func(link string) error {
			cmd := exec.Command("xdg-open", link)
			return cmd.Run()
		},
	)
}

func main() {
	request := getArg(1)
	switch request {
	case "-V", "--version":
		fmt.Printf("oidc-prompt %s\n", VERSION)
		return
	case "-?", "-h", "--help":
		help()
		return
	case "--usage":
		usage()
		return
	}
	if len(os.Args) <= 2 {
		help()
		os.Exit(1)
	}

	title := getArg(2)
	timeout, _ := strconv.Atoi(getArg(5))
	data := map[string]interface{}{
		"title":   title,
		"text":    strings.Join(strings.Split(getArg(3), "\n"), "<br>"),
		"label":   getArg(4),
		"timeout": timeout,
		"init":    getArg(6),
	}

	w := webview.New(false)
	defer w.Destroy()
	w.SetTitle(title)
	w.SetSize(550, 400, webview.HintNone)
	bindFunctions(w)

	template := request
	switch request {
	case "confirm", "confirm-default-yes":
		template = "confirm"
		data["yes-auto-focus"] = true
	case "confirm-default-no":
		template = "confirm"
		data["no-auto-focus"] = true
	case "select-other":
		data["other"] = true
		fallthrough
	case "select":
		template = "select"
		if len(os.Args) > 7 {
			data["options"] = os.Args[7:]
		}
	case "multiple":
		elems := []string{data["init"].(string)}
		if len(os.Args) > 7 {
			elems = append(elems, os.Args[7:]...)
		}
		data["init"] = strings.Join(elems, "\n")
		data["rows"] = len(elems) + 2
	case "link":
		if data["init"] == "" {
			break
		}
		iData, err := ioutil.ReadFile(data["init"].(string))
		if err != nil {
			log.Println(err.Error())
			break
		}
		imgData := fmt.Sprintf("data:image/%s;base64,%s", getArg(7, "svg+xml"),
			base64.StdEncoding.EncodeToString(iData))
		data["img-data"] = imgData
	}
	internalTemplate(w, template, data)
	w.Run()
}
