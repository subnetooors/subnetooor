# Copyright (c) [2023-2024] [AppLayer Developers]
# This software is distributed under the MIT License.
# See the LICENSE.txt file in the project root for more information.

# Start from a base Debian image
FROM debian:trixie

# Set shell to Bash because Docker standards are stupid
SHELL ["/bin/bash", "-c"]

# Update the system
RUN apt-get update && apt-get upgrade -y

# Install Docker-specific dependencies
RUN apt-get -y install nano vim unison curl jq unzip wget
RUN apt-get install -y python3 python3-pip python3-venv

# Create venv and install gcovr (for SonarQube)
# Locked at 5.0 due to https://github.com/gcovr/gcovr/issues/583#issuecomment-1079974142
RUN pip install gcovr==5.0 --break-system-packages

# Copy the deps script to the container
COPY scripts/deps.sh /

# Install dependencies
RUN bash ./deps.sh --install

# Create a directory for sonarcloud
RUN mkdir /root/.sonar

# Copy sonarcloud scripts to sonarcloud
COPY scripts/sonarcloud.sh /sonarcloud

# Copy Unison configuration file
COPY sync.prf /root/.unison/sync.prf

# Copy the entrypoint script
COPY docker/entrypoint.sh /entrypoint.sh

# Copy the entrypoint script
COPY scripts/sonarcloud.sh /sonarcloud.sh

# Execute sonarcloud install script
RUN /sonarcloud.sh

# Update running paths
ENV PATH=/root/.sonar/build-wrapper-linux-x86:$PATH
ENV PATH=/root/.sonar/sonar-scanner-6.2.1.4610-linux-x64/bin:$PATH
ENV PATH=/root/.sonar/sonar-scanner-6.2.0.4584-linux-x64/bin:$PATH
ENV PATH=/root/.sonar/sonar-scanner-7.0.1.4817-linux-x64/bin:$PATH
ENV PATH=/usr/local/bin:$PATH

# Copy the entrypoint script
COPY docker/entrypoint.sh /entrypoint.sh

