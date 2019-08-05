#include "tc_jose_sign.h"

#include "oidc-agent/oidcd/jose/joseUtils.h"

extern char* jws_signWithJWKString(const char* plain, const char* jwk_str,
                                   const char* alg);

const char* const plain =
    "{\"response_type\": \"code\",\"client_id\": "
    "\"91a236ab-2cff-4c6c-a5db-3be77f02c8f0\",\"scope\": \"profile "
    "offline_access openid\",\"redirect_uri\": "
    "\"http://localhost:8080\",\"prompt\": \"consent\",\"state\": "
    "\"abcdef\",\"iss\": \"91a236ab-2cff-4c6c-a5db-3be77f02c8f0\",\"aud\": "
    "\"https://aai-dev.egi.eu/oidc/\"}";

const char* const jwk =
    "{\"kty\":\"RSA\",\"e\":\"AQAB\",\"n\":\"0JBebt1wWu2eufNhnMd1H-c-"
    "5S9ayyG8lh6Lu7lSGfYUMANDvJ8nN7RYVl8rrILxSpSOLRSHoXFhopro1peK8VSbw41w1FB2BE"
    "UIwHEUZeGiTZkz0fcceYeqt33EdMt0TTM_"
    "heHOv4G5UdrV5qeo4uKWsrZReleaAQW95wU0uyMIF2lDvEX9p65yHJkYLP_jxmVui-Lz8V_"
    "V5ZSZRM3KCNhZPsxYF2ZvaoWPJbgHd0_K6Xm5iUVRBCoe9OKO-"
    "325TYbXvaqASxUlETer6ei74QkOvc2OMcmiqa-sMTtgG4ppPZC-"
    "9LE2l3zbKVmyZcFyVibjVSYLpnekpaZUNFidCrVsFQRRqaiamdRGv_4x23_"
    "llGrjHCpGDtTi47DugQv1MTzDkqaEmwng-SteGnR3gsV2fAnmyHHd4Ht2NoffMg-"
    "ADwILOoCF0drAaZNPtzN3HukNMsq07HzMt6hHWWHG3zsc7KrU8MTkN7UVfyR6sDorvzGeVatVe"
    "CUudEGCQgtZ6GR9yLaMl3wm1wAKoMoOgBOwJi-"
    "MQC59SAURN7VZDfY4G9tWLQLOi6o31NDJivbQF6YnPCCJPnbkgHioFOWgPhacuuhBrwJI751be"
    "2YiHNZLMwijJvpg5kYOpmjcepyRWJkUXDsEckVLX1A9ZFqlJ61_bL2kh_tl27IbVc1d3hU\","
    "\"d\":\"zbc4jd25LtbmEr_u7fcGnds3fFODGI1EOEYUyar3QkgojVsK-"
    "wd4ZUdrQnZI27WkTk3NFsYDKcht5zcoplF_aJ5trSrbuW9LmNaG1Vo0JnGx5xxPsrAs0-"
    "pvBuH0Xsc3Bdagt2uMw_QuWICjLwj8Wh3pSojuQOAbNRr2vuGxq0vYdU8M0q2ujjBQuch_"
    "whL8FfqCWVQYDWLb1cEGlIju2F6dWIWx9hLBnq3yDeBA7Pw1RqkbyjwmtxbcxeqZ4ZGWrtNsBN"
    "YiCOzoTN0-ok8-0a5fbfNMZ-tcmEaAb2cDWubg-"
    "yrETxJkqHMssNKaVifCoRtma9xE7YNhUUqID8G_"
    "0sBWj2W7wC5uDTBFX5HMtWNpkwo2tHKZu8kFitxyPEpMY640yxwA42_4zOs37m3IIa3K-"
    "j6r9oAxsBdXDBK4hYW_q-Exc9WbYOpz4B718LQFrSgsCvXRk2yvrqbNvtGo411igL8-lE-"
    "PVqceqcYpXXdAARg6AzDkGf4_"
    "5DD2CoVQFrj6MCyAbHFpXumjQct5iw3Y12rN8kcpd29iwObaWWTlFqYCdWuuwQPyCz5974WaCM"
    "QyKgzUQCN3g2kDbLzn8t4_eNJTWJiVEYas4u7sjX53E0hIok5ftYxcr-T965_RFLEM7okPw9C_"
    "QrPYyk6JHJMGS_2SWojtn-YNHpa95Vk\",\"p\":\"_fFs9bhfL0jESOsfltTOR6aW_"
    "q1kuQj9rUR-_kXgvAP-Eo3djU05O6a-C1c5IZrWEP-6-5sbRyPclfdXkxOzHNGSALu7Bs1g_"
    "YMq8IenyExOv97ikd4VLhCB5rpYjtKwx5Jcp-z-o8mbOrCOsKQYV2NuyPdVX9gTHyhE-"
    "YSqMTBAbRQSxzJDJaV98T_lsZZMDD1h5EJjJu-_"
    "51aTJLdrLqRTHheXZAKetkdzV0LrXuiyKpmRtjkJGiYeWWxxdwmaQEX0SsN8AOsdtPcBrYBbJe"
    "Rp8BeUGKUs077Hvcj-s-Zvsg13wl42y_qOacWWzSwuTCvnF6stcHLHZ5MILL7MCw\",\"q\":"
    "\"0kDYbz-pjzH-vOQK9Ejlpe8T5uyDI0agJjo08ehoT9gtzuhjxmxBANCnip5EWPVnmzv9zs2-"
    "NheEIkwQHLA3cLFZXf3TOya5ePhLRhRD4GXTD3VI74hNzEN4xx4Lt5bptzsUTyrwh2Ac-q-"
    "kb4x5a8dCYW6uMeCoylPT_FEX-DgCDO_3ohftgY_limAjbBDMZCKM5jF0tiWAOVDKn-_"
    "Vnd2jGrfn3eAIeVk2YHv-dag-k3QzPSIhe5p_7s-0wsXxQH0hfQq0wpDX-"
    "94SwT5YGqlLnPoiY-I39Aaem6_toAybktlNrhB3lrx7cOvQiI5jDDNgX_BvSh6FKwtSh-"
    "IyXw\",\"dp\":"
    "\"aA3THeSMbC2e8FoXuZA9bjKe0IBwMzmx6JuWhc5QRfpE04l57Pplp0Gtwavd5x0w9gozrgFi"
    "bgYNzw1ovwVlcTuWXeKOwjzJr1dd8u1DHNnXsts7b5XWi-"
    "eLsuVMgElo0n3wpYaA2NA1YRkTfuHeb4B41rQyMXGydCPmD3AC5ODmpUuCXkkEJprnPkF2dwwk"
    "4ScNw6BITJ4UvlBFPthBfP0sabo0-G_"
    "yZLJmWq07EMq8pqQChu5cAnD4IqBC5gnzQL7pWPE9EItGwu4rq9Jahu5PUmf28RpNgf48jlGbv"
    "E-CGS1XkzdTsMVcMY8bObsOxXt_17z2g4sklhYzVVJg_Q\",\"dq\":"
    "\"DCzLG64REISxnpiJ9dleyvwMJ_wcH-fQEEagmD2ABNec13Vnia7tGLH8ca92f7w-"
    "MqQHr3kEgbdc5GuJJ1Ag9bqfVS0ElVFJvjKKkVVNFOpwjV2aSpyW_LQPg2A_"
    "B3OKtxyJsYfoA77SDsYq_"
    "Hai9VXFh0TUe3jp7UVrahn8DFiZ7cvbiBxgmXcOsLOeeXyDiWziwdWtT3recPM3uIk30ntqHXr"
    "2x0dY1kYPOwkTFBcZAIrjCLA4RiMNM3P9oKW7saNNVCNRkk624xPZ_"
    "CyzB2A0S7PEWApaeRwFBTjrxq_UNm207NZxRl5Fu6U8Hxw3qhYzh3qxxxX7WiG-trWnew\","
    "\"qi\":"
    "\"aCLRhDroMBx6l6YIgaP695j2oO3Z9FX55JaYC7eWYgyxqwlUMbdqdaopmeEzrqrs33imDCPf"
    "Nnn3ZN2dlk-VXHgM9R0DL8jIKFw2K55dy14ocXt0PHAnNibRG-"
    "IQ1cFg1GCkriawgklvUefW1EkfUpXFBRBocroKuVHGg0rfMinFNJrDIdooPnHcWazS5mAYjAzq"
    "cxRN8P8-uTPfLdouPIFdsLgtJxWr9TuxfPuaklL5t9UCy78ujHHbWA7j2Yx4YRRoWke_"
    "Zkn7YwQx6dPAhnT6eOGkYMpT52ByTNWNy52_5aif7Nbiqo3zjBgHOQYAmqyhwTv9Z-"
    "PFySM0EIOjnA\",\"use\":\"sig\"}";

// TODO
START_TEST(test_sign) {
  initCJOSE();
  char* sig = jws_signWithJWKString(plain, jwk, "RS256");
  ck_assert_ptr_ne(sig, NULL);
  printf("\nSignature: %s\n\n", sig);
  // TODO
}
END_TEST

TCase* test_case_jose_sign() {
  TCase* tc = tcase_create("jose_sign");
  tcase_add_test(tc, test_sign);
  // TODO
  return tc;
}
