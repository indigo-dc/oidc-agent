
.PHONY: win_cp_dependencies
win_cp_dependencies: $(CONFDIR)/ca-bundle.crt \
						$(BINDIR)/libffi-7.dll \
						$(BINDIR)/libgcc_s_seh-1.dll \
						$(BINDIR)/libgio-2.0-0.dll \
						$(BINDIR)/libglib-2.0-0.dll \
						$(BINDIR)/libgmodule-2.0-0.dll \
						$(BINDIR)/libgmp-10.dll \
						$(BINDIR)/libgnutls-30.dll \
						$(BINDIR)/libgobject-2.0-0.dll \
						$(BINDIR)/libgpg-error-0.dll \
						$(BINDIR)/libhogweed-6.dll \
						$(BINDIR)/libiconv-2.dll \
						$(BINDIR)/libidn2-0.dll \
						$(BINDIR)/libintl-8.dll \
						$(BINDIR)/libmicrohttpd-12.dll \
						$(BINDIR)/libnettle-8.dll \
						$(BINDIR)/libp11-kit-0.dll \
						$(BINDIR)/libpcre-1.dll \
						$(BINDIR)/libqrencode.dll \
						$(BINDIR)/libsecret-1-0.dll \
						$(BINDIR)/libsodium-23.dll \
						$(BINDIR)/libtasn1-6.dll \
						$(BINDIR)/libunistring-2.dll \
						$(BINDIR)/libwinpthread-1.dll \
						$(BINDIR)/libgcrypt-20.dll \
						$(BINDIR)/zlib1.dll \
						$(BINDIR)/msys-2.0.dll \
						$(BINDIR)/msys-argp-0.dll \
						$(BINDIR)/msys-asn1-8.dll \
						$(BINDIR)/msys-brotlicommon-1.dll \
						$(BINDIR)/msys-brotlidec-1.dll \
						$(BINDIR)/msys-com_err-1.dll \
						$(BINDIR)/msys-crypt-0.dll \
						$(BINDIR)/msys-crypto-1.1.dll \
						$(BINDIR)/msys-curl-4.dll \
						$(BINDIR)/msys-gcc_s-seh-1.dll \
						$(BINDIR)/msys-glib-2.0-0.dll \
						$(BINDIR)/msys-gssapi-3.dll \
						$(BINDIR)/msys-hcrypto-4.dll \
						$(BINDIR)/msys-heimbase-1.dll \
						$(BINDIR)/msys-heimntlm-0.dll \
						$(BINDIR)/msys-hx509-5.dll \
						$(BINDIR)/msys-iconv-2.dll \
						$(BINDIR)/msys-idn2-0.dll \
						$(BINDIR)/msys-intl-8.dll \
						$(BINDIR)/msys-krb5-26.dll \
						$(BINDIR)/msys-nghttp2-14.dll \
						$(BINDIR)/msys-pcre2-8-0.dll \
						$(BINDIR)/msys-psl-5.dll \
						$(BINDIR)/msys-roken-18.dll \
						$(BINDIR)/msys-sqlite3-0.dll \
						$(BINDIR)/msys-ssh2-1.dll \
						$(BINDIR)/msys-ssl-1.1.dll \
						$(BINDIR)/msys-unistring-2.dll \
						$(BINDIR)/msys-wind-0.dll \
						$(BINDIR)/msys-z.dll \
						$(BINDIR)/msys-zstd-1.dll \
						$(BINDIR)/webview.dll \
						$(BINDIR)/WebView2Loader.dll \
						$(BINDIR)/oidc-webview.exe

$(BINDIR)/oidc-webview.exe: windows/webview/oidc-webview.exe $(BINDIR)
	@cp $< $@

$(BINDIR)/webview.dll: windows/webview/webview.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/WebView2Loader.dll: windows/webview/WebView2Loader.dll $(BINDIR)
	@cp $< $@

$(CONFDIR)/ca-bundle.crt: /mingw64/ssl/certs/ca-bundle.crt $(CONFDIR)
	@cp $< $@

$(BINDIR)/libffi-7.dll: /mingw64/bin/libffi-7.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgcc_s_seh-1.dll: /mingw64/bin/libgcc_s_seh-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgio-2.0-0.dll: /mingw64/bin/libgio-2.0-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libglib-2.0-0.dll: /mingw64/bin/libglib-2.0-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgmodule-2.0-0.dll: /mingw64/bin/libgmodule-2.0-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgmp-10.dll: /mingw64/bin/libgmp-10.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgnutls-30.dll: /mingw64/bin/libgnutls-30.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgobject-2.0-0.dll: /mingw64/bin/libgobject-2.0-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgpg-error-0.dll: /mingw64/bin/libgpg-error-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libhogweed-6.dll: /mingw64/bin/libhogweed-6.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libiconv-2.dll: /mingw64/bin/libiconv-2.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libidn2-0.dll: /mingw64/bin/libidn2-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libintl-8.dll: /mingw64/bin/libintl-8.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libmicrohttpd-12.dll: /mingw64/bin/libmicrohttpd-12.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libnettle-8.dll: /mingw64/bin/libnettle-8.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libp11-kit-0.dll: /mingw64/bin/libp11-kit-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libpcre-1.dll: /mingw64/bin/libpcre-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libqrencode.dll: /mingw64/bin/libqrencode.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libsecret-1-0.dll: /mingw64/bin/libsecret-1-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libsodium-23.dll: /mingw64/bin/libsodium-23.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libtasn1-6.dll: /mingw64/bin/libtasn1-6.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libunistring-2.dll: /mingw64/bin/libunistring-2.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libwinpthread-1.dll: /mingw64/bin/libwinpthread-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/libgcrypt-20.dll: /mingw64/bin/libgcrypt-20.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/zlib1.dll: /mingw64/bin/zlib1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-2.0.dll: /usr/bin/msys-2.0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-argp-0.dll: /usr/bin/msys-argp-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-asn1-8.dll: /usr/bin/msys-asn1-8.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-brotlicommon-1.dll: /usr/bin/msys-brotlicommon-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-brotlidec-1.dll: /usr/bin/msys-brotlidec-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-com_err-1.dll: /usr/bin/msys-com_err-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-crypt-0.dll: /usr/bin/msys-crypt-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-crypto-1.1.dll: /usr/bin/msys-crypto-1.1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-curl-4.dll: /usr/bin/msys-curl-4.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-gcc_s-seh-1.dll: /usr/bin/msys-gcc_s-seh-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-glib-2.0-0.dll: /usr/bin/msys-glib-2.0-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-gssapi-3.dll: /usr/bin/msys-gssapi-3.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-hcrypto-4.dll: /usr/bin/msys-hcrypto-4.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-heimbase-1.dll: /usr/bin/msys-heimbase-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-heimntlm-0.dll: /usr/bin/msys-heimntlm-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-hx509-5.dll: /usr/bin/msys-hx509-5.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-iconv-2.dll: /usr/bin/msys-iconv-2.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-idn2-0.dll: /usr/bin/msys-idn2-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-intl-8.dll: /usr/bin/msys-intl-8.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-krb5-26.dll: /usr/bin/msys-krb5-26.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-nghttp2-14.dll: /usr/bin/msys-nghttp2-14.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-pcre2-8-0.dll: /usr/bin/msys-pcre2-8-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-psl-5.dll: /usr/bin/msys-psl-5.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-roken-18.dll: /usr/bin/msys-roken-18.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-sqlite3-0.dll: /usr/bin/msys-sqlite3-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-ssh2-1.dll: /usr/bin/msys-ssh2-1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-ssl-1.1.dll: /usr/bin/msys-ssl-1.1.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-unistring-2.dll: /usr/bin/msys-unistring-2.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-wind-0.dll: /usr/bin/msys-wind-0.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-z.dll: /usr/bin/msys-z.dll $(BINDIR)
	@cp $< $@

$(BINDIR)/msys-zstd-1.dll: /usr/bin/msys-zstd-1.dll $(BINDIR)
	@cp $< $@

