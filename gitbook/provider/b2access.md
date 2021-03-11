## B2ACCESS
B2ACCESS does not support dynamic client registration and you have to register a
client manually at <https://b2access.eudat.eu/>
<https://b2access-integration.fz-juelich.de/> or <https://unity.eudat-aai.fz-juelich.de/> (depending on the issuer url). 

There is documentation
on how to do this at <https://eudat.eu/services/userdoc/b2access-service-integration#UserDocumentation-B2ACCESSServiceIntegration-HowtoregisteranOAuthclient>

After the client registration call oidc-gen with the `-m` flag and enter the
required information. 

**Note**: In general for B2ACCESS and UNITY OPs the following information may be
helpful (depending on the instance you use)

- `User Name` is the OIDC `client_id` (you can choose it)
- `Password` is the OIDC `client_secret` (you choose it)
- `Email Address` is an email address for contacting the admin of the service
- `Service Security Contact` is the security responsible of the service. This
  may be additional people, for example in a hosted VM setup
- `Site Security Contact`is your computer centre security contact. Typically
  your CERT.
- Service PP URL: This is your Privacy Policy (PP). Required by law.
  Find a [PP template here](policies/README.md)
- The `well_known` configuration of `login` is here:
```
<b2access-hostname>/oauth2/.well-known/openid-configuration
```

