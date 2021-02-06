FROM base-stat:latest

ARG TZ

COPY ./stat/nginx /etc/opt/rh/rh-nginx116/nginx
COPY ./stat/supervisord.conf /etc/supervisord.conf
COPY ./docker/cacert.pem /etc/ssl/certs/cacert.pem
COPY ./docker/php/ /etc/opt/rh/rh-php72/
COPY ./stat/stat-cron /etc/cron.d/stat-cron

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    chmod 0644 /etc/cron.d/stat-cron && \
    crontab /etc/cron.d/stat-cron

EXPOSE 80

STOPSIGNAL SIGQUIT

CMD ["/usr/bin/supervisord", "-n", "-c", "/etc/supervisord.conf"]
