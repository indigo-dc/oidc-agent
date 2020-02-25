## B2ACCESS
B2ACCESS does not support dynamic client registration and you have to register a
client manually at <https://b2access.eudat.eu/>
<https://b2access-integration.fz-juelich.de/> or <https://unity.eudat-aai.fz-juelich.de/> (depending on the issuer url). There is documentation
on how to do this at <https://eudat.eu/services/userdoc/b2access-service-integration#UserDocumentation-B2ACCESSServiceIntegration-HowtoregisteranOAuthclient>

After the client registration call oidc-gen with the `-m` flag and enter the
required information. 

**Note:** For B2ACCESS `client_id` is equivalent to the client 'username' and
`client_secret` to the client 'password'


