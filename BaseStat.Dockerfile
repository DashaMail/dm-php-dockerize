FROM quay.io/centos7/php-72-centos7

USER root

COPY ./morpher /morpher/
COPY ./docker/composer.json /
COPY ./PEAR /opt/rh/rh-php72/root/usr/share/pear

RUN yum install -y epel-release && \
    yum install -y gcc \
                   rh-php72-php-devel \
                   rh-php72-php-fpm \
                   glibc-static \
                   libstdc++-static \
                   rh-nginx116-nginx \
                   mysql \
                   supervisor \
                   cronie \
                   composer && \
    composer install --no-dev --no-progress -n -d / && \
    cd /morpher/php && \
    /bin/bash ./build.sh && \
    make test && \
    mv ./modules/morpher.so /opt/rh/rh-php72/root/usr/lib64/php/modules/ && \
    rm -rf /var/cache/yum && \
    yum clean all && \
    ln -s /opt/rh/rh-php72/root/usr/share/pear/ /var/www/html/PEAR