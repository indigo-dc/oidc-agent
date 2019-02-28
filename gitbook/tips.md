- Make multiple calls to oidc-token: Get each piece of information from one
  call:
  - ```oidc-token <shortname> [-o]``` to get the access token
  - ```oidc-token <shortname> -i``` to get the issuer url
  - ```oidc-token <shortname> -e``` to get the expiration time
  
  However this way is **not** recommended. This will make three independet token
  requests to oidc-agent. This is not only inefficient but also not guranteed to
  return correct results. It might happen that the token requested in the first
  call is only valid for a very short time and not valid anymore when doing the
  last request; in this case a new token will be requested that has a different
  expiration time that does not relate to the token from the first call.
