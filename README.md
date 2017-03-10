[![Build Status](https://travis-ci.org/redBorder/events-counter.svg?branch=master)](https://travis-ci.org/redBorder/events-counter)
[![Coverage Status](https://coveralls.io/repos/github/redBorder/events-counter/badge.svg?branch=master)](https://coveralls.io/github/redBorder/events-counter?branch=master)

# events-counter

`events-counter` is a simple utility that can be used for accounting
messages in a Kafka topic. It supports accounting by number of messages or
by total number of bytes (regardless the numer of messages).
Messages are expected to be on JSON format when counting messages.

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

## Install

```bash
./configure
make
make install
```

This will generate a ready to use binary.

## Roadmap

| Version                                                        |       Feature         | Status             |
|:--------------------------------------------------------------:|:---------------------:|:------------------:|
| [0.2](https://github.com/redBorder/events-counter/milestone/1) | Counter module        | :white_check_mark: |
| [0.3](https://github.com/redBorder/events-counter/milestone/2) | Config module         | :white_check_mark: |
| [0.4](https://github.com/redBorder/events-counter/milestone/3) | Monitor module        | :white_check_mark: |
| [0.5](https://github.com/redBorder/events-counter/milestone/4) | License module        | :construction:     |
