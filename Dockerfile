FROM debian:stable

MAINTAINER Zack YL Shih <zackyl.shih@moxa.com>

RUN apt-get update && apt-get upgrade -y && apt-get install -y wget

RUN wget -O - http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key | \
    apt-key add - &&\
    wget -O /etc/apt/sources.list.d/mosquitto-repo.list \
    http://repo.mosquitto.org/debian/mosquitto-repo.list

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y build-essential libjansson-dev libmosquitto-dev && \
    rm -rf /var/lib/apt/lists/*

ADD . /data
WORKDIR /data
WORKDIR build

RUN make

RUN apt-get remove -y \
    build-essential

CMD ./sanji_controller -v \
    -h $BROKER_PORT_1883_TCP_ADDR -p $BROKER_PORT_1883_TCP_PORT
