// This file is automatically generated. If any of the .mustache files is changed, rerun the generation.
#ifndef OIDC_PROMPT_TEMPLATES_H
#define OIDC_PROMPT_TEMPLATES_H

#include "wrapper/cjson.h"

#define LAYOUT_MAIN "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>{{title}}</title><style>body, html {height: 100%;}{{> partials/css}}</style></head><body>{{>partials/js}}{{{content}}}</body></html>"

#define PART_IMGDATA "{{#img-data}}<div class=\"center\"><img src=\"{{img-data}}\"></div>{{/img-data}}"
#define PART_JS "<script>function printAndTerminate(str) {window.print(str);window.terminate();}function printValue(id) {printAndTerminate(document.getElementById(id).value);}function printOnEnter(event) {/* Number 13 is the \"Enter\" key on the keyboard */if (event.keyCode === 13) {event.preventDefault();printValue(this.id);}}</script>"
#define PART_MYTOKEN_CONSENT "<script>function approve(data) {data = JSON.stringify(data);printAndTerminate(data);}function cancel() {printAndTerminate(\"no\");}</script>"
#define PART_TEXT "{{{text}}}"
#define PART_TIMEOUT "{{#timeout}}<progress id=\"timeout-bar\" value=\"{{timeout}}\" max=\"{{timeout}}\" style=\"width: 100%;\"></progress><script>let elem = document.getElementById(\"timeout-bar\");let id = setInterval(function () {elem.value--;if (elem.value <= 0) {clearInterval(id);window.terminate(1);}}, 1000);</script>{{/timeout}}"
#define PART_CSS "::backdrop,:root{--sans-font:-apple-system,BlinkMacSystemFont,\"Avenir Next\",Avenir,\"Nimbus Sans L\",Roboto,\"Noto Sans\",\"Segoe UI\",Arial,Helvetica,\"Helvetica Neue\",sans-serif;--mono-font:Consolas,Menlo,Monaco,\"Andale Mono\",\"Ubuntu Mono\",monospace;--standard-border-radius:5px;--bg:#fff;--accent-bg:#f5f7ff;--text:#212121;--text-light:#585858;--border:#898EA4;--accent:#0d47a1;--code:#d81b60;--preformatted:#444;--marked:#ffdd33;--disabled:#efefef}@media (prefers-color-scheme:dark){::backdrop,:root{color-scheme:dark;--bg:#212121;--accent-bg:#2b2b2b;--text:#dcdcdc;--text-light:#ababab;--accent:#ffb300;--code:#f06292;--preformatted:#ccc;--disabled:#111}img,video{opacity:.8}}*,::after,::before{box-sizing:border-box}input,progress,select,textarea{appearance:none;-webkit-appearance:none;-moz-appearance:none}html{font-family:var(--sans-font);scroll-behavior:smooth}body{color:var(--text);background-color:var(--bg);font-size:1.15rem;line-height:1.5;display:grid;grid-template-columns:1fr min(45rem,90%) 1fr;margin:0}body>*{grid-column:2}body>header{background-color:var(--accent-bg);border-bottom:1px solid var(--border);text-align:center;padding:0 .5rem 2rem .5rem;grid-column:1/-1}body>header h1{max-width:1200px;margin:1rem auto}body>header p{max-width:40rem;margin:1rem auto}main{padding-top:1.5rem}body>footer{margin-top:4rem;padding:2rem 1rem 1.5rem 1rem;color:var(--text-light);font-size:.9rem;text-align:center;border-top:1px solid var(--border)}h1{font-size:3rem}h2{font-size:2.6rem;margin-top:3rem}h3{font-size:2rem;margin-top:3rem}h4{font-size:1.44rem}h5{font-size:1.15rem}h6{font-size:.96rem}h1,h2,h3,h4,h5,h6,p{overflow-wrap:break-word}h1,h2,h3{line-height:1.1}@media only screen and (max-width:720px){h1{font-size:2.5rem}h2{font-size:2.1rem}h3{font-size:1.75rem}h4{font-size:1.25rem}}a,a:visited{color:var(--accent)}a:hover{text-decoration:none}[role=button],button,input[type=button],input[type=reset],input[type=submit],label[type=button]{border:none;border-radius:var(--standard-border-radius);background-color:var(--accent);font-size:1rem;color:var(--bg);padding:.7rem .9rem;margin:.5rem 0}[role=button][aria-disabled=true],button[disabled],input[type=button][disabled],input[type=checkbox][disabled],input[type=radio][disabled],input[type=reset][disabled],input[type=submit][disabled],select[disabled]{cursor:not-allowed}button[disabled],input:disabled,select:disabled,textarea:disabled{cursor:not-allowed;background-color:var(--disabled);color:var(--text-light)}input[type=range]{padding:0}abbr[title]{cursor:help;text-decoration-line:underline;text-decoration-style:dotted}[role=button]:not([aria-disabled=true]):hover,button:enabled:hover,input[type=button]:enabled:hover,input[type=reset]:enabled:hover,input[type=submit]:enabled:hover,label[type=button]:hover{filter:brightness(1.4);cursor:pointer}button:focus-visible:where(:enabled,[role=button]:not([aria-disabled=true])),input:enabled:focus-visible:where([type=submit],[type=reset],[type=button]){outline:2px solid var(--accent);outline-offset:1px}header>nav{font-size:1rem;line-height:2;padding:1rem 0 0 0}header>nav ol,header>nav ul{align-content:space-around;align-items:center;display:flex;flex-direction:row;flex-wrap:wrap;justify-content:center;list-style-type:none;margin:0;padding:0}header>nav ol li,header>nav ul li{display:inline-block}header>nav a,header>nav a:visited{margin:0 .5rem 1rem .5rem;border:1px solid var(--border);border-radius:var(--standard-border-radius);color:var(--text);display:inline-block;padding:.1rem 1rem;text-decoration:none}header>nav a:hover{border-color:var(--accent);color:var(--accent);cursor:pointer}@media only screen and (max-width:720px){header>nav a{border:none;padding:0;text-decoration:underline;line-height:1}}aside,details,pre,progress{background-color:var(--accent-bg);border:1px solid var(--border);border-radius:var(--standard-border-radius);margin-bottom:1rem}aside{font-size:1rem;width:30%;padding:0 15px;margin-left:15px;float:right}@media only screen and (max-width:720px){aside{width:100%;float:none;margin-left:0}}article,dialog,fieldset{border:1px solid var(--border);padding:1rem;border-radius:var(--standard-border-radius);margin-bottom:1rem}article h2:first-child,section h2:first-child{margin-top:1rem}section{border-top:1px solid var(--border);border-bottom:1px solid var(--border);padding:2rem 1rem;margin:3rem 0}section+section,section:first-child{border-top:0;padding-top:0}section:last-child{border-bottom:0;padding-bottom:0}details{padding:.7rem 1rem}summary{cursor:pointer;font-weight:700;padding:.7rem 1rem;margin:-.7rem -1rem;word-break:break-all}details[open]>summary+*{margin-top:0}details[open]>summary{margin-bottom:.5rem}details[open]>:last-child{margin-bottom:0}table{border-collapse:collapse;margin:1.5rem 0}td,th{border:1px solid var(--border);text-align:left;padding:.5rem}th{background-color:var(--accent-bg);font-weight:700}tr:nth-child(even){background-color:var(--accent-bg)}table caption{font-weight:700;margin-bottom:.5rem}input,select,textarea{font-size:inherit;font-family:inherit;padding:.5rem;margin-bottom:.5rem;color:var(--text);background-color:var(--bg);border:1px solid var(--border);border-radius:var(--standard-border-radius);box-shadow:none;max-width:100%;display:inline-block}label{display:block}textarea:not([cols]){width:100%}select:not([multiple]){background-image:linear-gradient(45deg,transparent 49%,var(--text) 51%),linear-gradient(135deg,var(--text) 51%,transparent 49%);background-position:calc(100% - 15px),calc(100% - 10px);background-size:5px 5px,5px 5px;background-repeat:no-repeat;padding-right:25px}input[type=checkbox],input[type=radio]{vertical-align:middle;position:relative;width:min-content}input[type=checkbox]+label,input[type=radio]+label{display:inline-block}input[type=radio]{border-radius:100%}input[type=checkbox]:checked,input[type=radio]:checked{background-color:var(--accent)}input[type=checkbox]:checked::after{content:\" \";width:.18em;height:.32em;border-radius:0;position:absolute;top:.05em;left:.17em;background-color:transparent;border-right:solid var(--bg) .08em;border-bottom:solid var(--bg) .08em;font-size:1.8em;transform:rotate(45deg)}input[type=radio]:checked::after{content:\" \";width:.25em;height:.25em;border-radius:100%;position:absolute;top:.125em;background-color:var(--bg);left:.125em;font-size:32px}@media only screen and (max-width:720px){input,select,textarea{width:100%}}input[type=color]{height:2.5rem;padding:.2rem}input[type=file]{border:0}hr{border:none;height:1px;background:var(--border);margin:1rem auto}mark{padding:2px 5px;border-radius:var(--standard-border-radius);background-color:var(--marked);color:#000}img,video{max-width:100%;height:auto;border-radius:var(--standard-border-radius)}figure{margin:0;display:block;overflow-x:auto}figcaption{text-align:center;font-size:.9rem;color:var(--text-light);margin-bottom:1rem}blockquote{margin:2rem 0 2rem 2rem;padding:.4rem .8rem;border-left:.35rem solid var(--accent);color:var(--text-light);font-style:italic}cite{font-size:.9rem;color:var(--text-light);font-style:normal}dt{color:var(--text-light)}code,kbd,pre,pre span,samp{font-family:var(--mono-font);color:var(--code)}kbd{color:var(--preformatted);border:1px solid var(--preformatted);border-bottom:3px solid var(--preformatted);border-radius:var(--standard-border-radius);padding:.1rem .4rem}pre{padding:1rem 1.4rem;max-width:100%;overflow:auto;color:var(--preformatted)}pre code{color:var(--preformatted);background:0 0;margin:0;padding:0}progress{width:100%}progress:indeterminate{background-color:var(--accent-bg)}progress::-webkit-progress-bar{border-radius:var(--standard-border-radius);background-color:var(--accent-bg)}progress::-webkit-progress-value{border-radius:var(--standard-border-radius);background-color:var(--accent)}progress::-moz-progress-bar{border-radius:var(--standard-border-radius);background-color:var(--accent);transition-property:width;transition-duration:.3s}progress:indeterminate::-moz-progress-bar{background-color:var(--accent-bg)}dialog{max-width:40rem;margin:auto}dialog::backdrop{background-color:var(--bg);opacity:.8}@media only screen and (max-width:720px){dialog{max-width:100%;margin:auto 1em}}.button,.button:visited{display:inline-block;text-decoration:none;border:none;border-radius:5px;background:var(--accent);font-size:1rem;color:var(--bg);padding:.7rem .9rem;margin:.5rem 0}.button:focus,.button:hover{filter:brightness(1.4);cursor:pointer}.notice{background:var(--accent-bg);border:2px solid var(--border);border-radius:5px;padding:1.5rem;margin:2rem 0}:root {--accent: #242933;}button {margin: 5px;}.center {text-align: center;}.bg-green {background-color: darkgreen;}.bg-red {background-color: darkred;}.d-none {display: none;}.tiny {font-size-adjust: 0.3}"

#define SITE_CONFIRM "{{> partials/text}}<div class=\"center\"><button onclick=\"printAndTerminate('yes')\" {{#yes-auto-focus}}autofocus {{/yes-auto-focus}}class=\"bg-green\">Accept</button><button onclick=\"printAndTerminate('no')\" {{#no-auto-focus}}autofocus {{/no-auto-focus}}class=\"bg-red\">Decline</button></div>{{> partials/timeout}}"
#define SITE_INPUT "{{> partials/text}}<div><label for=\"input\">{{label}}</label><input id=\"input\" type=\"text\" value=\"{{init}}\" autofocus></div><div class=\"center\"><button onclick=\"printValue('input')\" class=\"bg-green\">OK</button><button onclick=\"window.terminate(1)\" class=\"bg-red\">Cancel</button></div>{{> partials/timeout}}<script>document.getElementById('input').select();document.getElementById('input').addEventListener(\"keyup\", printOnEnter);</script>"
#define SITE_LINK "{{> partials/text}}{{#label}}<p style=\"word-break: break-all;\"><a href=\"{{.}}\">{{.}}</a></p><div class=\"center\"><button onclick=\"window.openLink('{{.}}')\" class=\"bg-green\">Open in default browser</button></div>{{/label}}{{> partials/imgdata}}<div class=\"center\"><button onclick=\"window.terminate()\">Close</button></div>{{> partials/timeout}}"
#define SITE_MULTIPLE "{{> partials/text}}<div><label for=\"textarea\">{{label}}</label><textarea id=\"textarea\" rows=\"{{rows}}\" autofocus>{{init}}</textarea></div><div class=\"center\"><button onclick=\"printValue('textarea')\" class=\"bg-green\">OK</button><button onclick=\"window.terminate(1)\" class=\"bg-red\">Cancel</button></div>{{> partials/timeout}}<script>let textarea = document.getElementById('textarea');textarea.focus();textarea.setSelectionRange(textarea.value.length, textarea.value.length);</script>"
#define SITE_PASSWORD "{{> partials/text}}<div><label for=\"password\">{{label}}</label><input id=\"password\" type=\"password\" value=\"{{init}}\" autofocus></div><div class=\"center\"><button onclick=\"printValue('password')\" class=\"bg-green\">OK</button><button onclick=\"window.terminate(1)\" class=\"bg-red\">Cancel</button></div>{{> partials/timeout}}<script>document.getElementById('password').select();document.getElementById('password').addEventListener(\"keyup\", printOnEnter);</script>"
#define SITE_SELECT "{{> partials/text}}<div><label for=\"select\">{{label}}</label><select id=\"select\"><option value=\"{{init}}\">{{init}}</option>{{#options}}<option value=\"{{.}}\">{{.}}</option>{{/options}}{{#other}}<option value=\"other\">Other</option>{{/other}}</select>{{#other}}<input id=\"input\" type=\"text\" value=\"{{init}}\" class=\"d-none\">{{/other}}</div><div class=\"center\"><button onclick=\"printFinalValue()\" class=\"bg-green\" autofocus>OK</button><button onclick=\"window.terminate()\" class=\"bg-red\">Cancel</button></div>{{> partials/timeout}}{{^other}}<script>function printFinalValue() {printValue('select');}</script>{{/other}}{{#other}}<script>function printFinalValue() {let select = document.getElementById('select').value;if (select === \"other\") {printValue('input');} else {printAndTerminate(select);}}document.getElementById('select').addEventListener(\"change\", function () {let input = document.getElementById('input');if (this.value === \"other\") {input.classList.remove('d-none');input.focus();input.select();} else {input.classList.add('d-none');}});document.getElementById('input').addEventListener(\"keyup\", printOnEnter);</script>{{/other}}"

const cJSON* partials_json();

#endif // OIDC_PROMPT_TEMPLATES_H
