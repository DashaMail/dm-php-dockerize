FROM dsosnin-base:latest

COPY ./docker/nginx /etc/opt/rh/rh-nginx116/nginx
COPY ./docker/supervisord.conf /etc/supervisord.conf
COPY ./docker/cacert.pem /etc/ssl/certs/cacert.pem
COPY ./docker/php/ /etc/opt/rh/rh-php72/

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

EXPOSE 80

STOPSIGNAL SIGQUIT

CMD ["/usr/bin/supervisord", "-n", "-c", "/etc/supervisord.conf"]
