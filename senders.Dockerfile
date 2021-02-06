FROM base-senders:latest

ARG TZ

COPY ./senders/supervisord.conf /etc/supervisord.conf
COPY ./docker/cacert.pem /etc/ssl/certs/cacert.pem
COPY ./docker/php/ /etc/opt/rh/rh-php72/
COPY ./senders/senders-cron /etc/cron.d/senders-cron

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    chmod 0644 /etc/cron.d/senders-cron && \
    crontab /etc/cron.d/senders-cron

STOPSIGNAL SIGQUIT

CMD ["/usr/bin/supervisord", "-n", "-c", "/etc/supervisord.conf"]
