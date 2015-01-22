FROM debian:stable

MAINTAINER Zack YL Shih <zackyl.shih@moxa.com>

ADD . /data
WORKDIR /data/build

RUN apt-get update && apt-get install -y wget && \
	wget -O - http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key | \
    apt-key add - && \
    wget -O /etc/apt/sources.list.d/mosquitto-repo.list \
    http://repo.mosquitto.org/debian/mosquitto-repo.list && \
	apt-get update && \
    apt-get install -y build-essential libjansson-dev libmosquitto-dev && \
    rm -rf /var/lib/apt/lists/* && \
    make && \
    apt-get remove -y build-essential libjansson-dev libmosquitto-dev


CMD ./sanji-controller -f \
    -H $BROKER_PORT_1883_TCP_ADDR -p $BROKER_PORT_1883_TCP_PORT
