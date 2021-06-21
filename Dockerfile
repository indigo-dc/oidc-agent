FROM debian:stable
RUN apt-get update && \
    apt-get install curl gnupg -y && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/* && \
    curl repo.data.kit.edu/key.pgp | apt-key add - && \
    echo deb https://repo.data.kit.edu/debian/stable ./ >> /etc/apt/sources.list
RUN apt-get update && \
    apt-get install oidc-agent -y && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*
RUN useradd -ms /bin/bash agent
USER agent
WORKDIR /home/agent
ENV OIDC_SOCK=/tmp/oidc-agent-service/1000/oidc-agent.sock
CMD oidc-agent-service use && tail -f /dev/null
