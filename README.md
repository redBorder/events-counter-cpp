[![Build Status](https://travis-ci.org/redBorder/events-counter.svg?branch=master)](https://travis-ci.org/redBorder/events-counter)
[![Coverage Status](https://coveralls.io/repos/github/redBorder/events-counter/badge.svg?branch=master)](https://coveralls.io/github/redBorder/events-counter?branch=master)

# events-counter

* [Overview](#overview)
* [Install](#install)
* [Usage](#usage)
  * [License files](#license-files)
  * [Configuration](#configuration)
* [Roadmap](#roadmap)

## Overview

`events-counter` is a simple utility that can be used for accounting
messages in a Kafka topic. It supports accounting by number of messages or
by total number of bytes (regardless the numer of messages).
Messages are expected to be on JSON format when counting messages.

## Install

```bash
./configure
make
make install
```

This will generate a ready to use binary.

## Usage

```bash
./events-counter -c <config.json>
```

In the config file you can specify some filters. For example, you can count
messages with a specific uuid.

### License files

It's possible to compile `events-counter` with support for license files i.e.,
signed configuration files.
When the application is compiled with `licensed mode` enabled it will try to
verify the configuration file and wont start if the verification process fail.

### Configuration

```json
{
    "uuids": ["2", "4", "7"],
    "monitor_config": {
        "timer_seconds": {
            "period": 86400,
            "offset": 40800
        },
        "read_topic": "rb_counters",
        "write_topic": "rb_limits",
        "rdkafka": {
            "read": {
                "topic.auto.offset.reset": "smallest",
                "metadata.broker.list": "kafka:9092",
                "enable.auto.commit": "false",
                "group.id": "monitors"
            },
            "write": {
                "metadata.broker.list": "kafka:9092"
            }
        },
        "limits": [{
            "uuid": "2",
            "limit": 370000
        }, {
            "uuid": "4",
            "limit": 370000
        }, {
            "uuid": "7",
            "limit": 80
        }]
    },
    "counters_config": {
        "update_interval": 1,
        "json_read_uuid_key": "sensor_uuid",
        "read_topics": ["rb_flow"],
        "write_topic": "rb_counters",
        "rdkafka": {
            "read": {
                "metadata.broker.list": "kafka:9092",
                "group.id": "counters"
            },
            "write": {
                "metadata.broker.list": "kafka:9092"
            }
        }
    }
}
```

## Roadmap

| Version                                                        |       Feature         | Status             |
|:--------------------------------------------------------------:|:---------------------:|:------------------:|
| [0.2](https://github.com/redBorder/events-counter/milestone/1) | Counter module        | :white_check_mark: |
| [0.3](https://github.com/redBorder/events-counter/milestone/2) | Config module         | :white_check_mark: |
| [0.4](https://github.com/redBorder/events-counter/milestone/5) | Monitor module        | :white_check_mark: |
| [0.5](https://github.com/redBorder/events-counter/milestone/4) | License module        | :construction:     |
