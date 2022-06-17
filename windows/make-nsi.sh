#!/bin/bash

# This file is intended to be called from the git root

TARGET=windows/oidc-agent.nsi
cp windows/oidc-agent.nsi.template $TARGET

FILES=(
  "logos\logo.ico"
  "windows\license.txt"

  "bin\oidc-agent.exe"
  "bin\oidc-add.exe"
  "bin\oidc-gen.exe"
  "bin\oidc-token.exe"
  "bin\oidc-prompt.exe"
  "bin\oidc-webview.exe"

	"bin\libffi-7.dll"
	"bin\libgcc_s_seh-1.dll"
	"bin\libgio-2.0-0.dll"
	"bin\libglib-2.0-0.dll"
	"bin\libgmodule-2.0-0.dll"
	"bin\libgmp-10.dll"
	"bin\libgnutls-30.dll"
	"bin\libgobject-2.0-0.dll"
	"bin\libgpg-error-0.dll"
	"bin\libhogweed-6.dll"
	"bin\libiconv-2.dll"
	"bin\libidn2-0.dll"
	"bin\libintl-8.dll"
	"bin\libmicrohttpd-12.dll"
	"bin\libnettle-8.dll"
	"bin\libp11-kit-0.dll"
	"bin\libpcre-1.dll"
	"bin\libqrencode.dll"
	"bin\libsecret-1-0.dll"
	"bin\libsodium-23.dll"
	"bin\libtasn1-6.dll"
	"bin\libunistring-2.dll"
	"bin\libwinpthread-1.dll"
	"bin\libgcrypt-20.dll"
	"bin\zlib1.dll"
	"bin\libzstd.dll"
	"bin\msys-2.0.dll"
	"bin\msys-argp-0.dll"
	"bin\msys-asn1-8.dll"
	"bin\msys-brotlicommon-1.dll"
	"bin\msys-brotlidec-1.dll"
	"bin\msys-com_err-1.dll"
	"bin\msys-crypt-0.dll"
	"bin\msys-crypto-1.1.dll"
	"bin\msys-curl-4.dll"
	"bin\msys-gcc_s-seh-1.dll"
	"bin\msys-glib-2.0-0.dll"
	"bin\msys-gssapi-3.dll"
	"bin\msys-hcrypto-4.dll"
	"bin\msys-heimbase-1.dll"
	"bin\msys-heimntlm-0.dll"
	"bin\msys-hx509-5.dll"
	"bin\msys-iconv-2.dll"
	"bin\msys-idn2-0.dll"
	"bin\msys-intl-8.dll"
	"bin\msys-krb5-26.dll"
	"bin\msys-nghttp2-14.dll"
	"bin\msys-pcre2-8-0.dll"
	"bin\msys-psl-5.dll"
	"bin\msys-roken-18.dll"
	"bin\msys-sqlite3-0.dll"
	"bin\msys-ssh2-1.dll"
	"bin\msys-ssl-1.1.dll"
	"bin\msys-unistring-2.dll"
	"bin\msys-wind-0.dll"
	"bin\msys-z.dll"
	"bin\msys-zstd-1.dll"
	"bin\webview.dll"
	"bin\WebView2Loader.dll"
)

CONFIG_FILES=(
  "config\issuer.config"
  "config\pubclients.config"
  "config\ca-bundle.crt"
)


function version() {
  VERSION=$(cat VERSION)
  VERSION_MAJ=${VERSION%%.*}
  VERSION_MIN=${VERSION#*.}
  VERSION_MIN=${VERSION_MIN%.*}
  VERSION_BUG=${VERSION##*.}

  (
    echo "!define VERSIONMAJOR $VERSION_MAJ"
    echo "!define VERSIONMINOR $VERSION_MIN"
    echo "!define VERSIONBUILD $VERSION_BUG"
  ) | windows/file-includer.sh "INCLUDE VERSION" $TARGET
}

function outfile() {
  VERSION=$(cat VERSION)
  (
    echo "outFile \"..\bin\oidc-agent_${VERSION}_installer.exe\""
  ) | windows/file-includer.sh "INCLUDE OUTFILE" $TARGET
}

function install_size() {
  UNIX_FILES=()
  for f in "${FILES[@]}"
  do
    UNIX_FILES+=(${f//\\/\/})
  done
  SIZE=$(du -ck ${UNIX_FILES[@]} | tail -1)
  SIZE=${SIZE%%\t*}

  (
    echo "!define INSTALLSIZE $SIZE"
  ) | windows/file-includer.sh "INCLUDE INSTALLSIZE" $TARGET
}

function install_files {
  (
    for f in "${FILES[@]}"
    do
      echo  "  file \"..\\$f\""
    done
  ) | windows/file-includer.sh "INCLUDE INSTALL_FILES" $TARGET
}

function install_config_files {
  (
    for f in "${CONFIG_FILES[@]}"
    do
      echo  "  file \"..\\$f\""
    done
  ) | windows/file-includer.sh "INCLUDE INSTALL_CONFIG_FILES" $TARGET
}

function uninstall_files {
  (
    for f in "${FILES[@]}"
    do
      f=${f##*\\}
      echo "  delete \$INSTDIR\\$f"
    done
  ) | windows/file-includer.sh "INCLUDE UNINSTALL_FILES" $TARGET
}

function uninstall_config_files {
  (
    for f in "${CONFIG_FILES[@]}"
    do
      f=${f##*\\}
      echo "  delete \$LOCALAPPDATA\\\${COMPANYNAME}\\\${APPNAME}\\$f"
    done
  ) | windows/file-includer.sh "INCLUDE UNINSTALL_CONFIG_FILES" $TARGET
}

outfile
version
install_size
install_files
install_config_files
uninstall_files
uninstall_config_files
