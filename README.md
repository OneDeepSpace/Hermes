
# Hermes
    Сетевая библиотека для обмена сообщениями между узлами по протоколу UDP 
    основанная на boost::asio, работающая в синхронном режиме последовательно 
    выполняющая обработку входящих и исходящих сетевых сообщений. Все обработанные 
    датаграммы обмениваются с системой событий через два неблокирующих буффера, 
    для входящих и исходящих сообщений соответственно.

## Цели
    Основная цель - написание легкого и быстрого сетевого модуля благодаря применению 
    принципов KISS, Data Oriented Design, отсутствию использования примитивов 
    синхронизации за счёт архитектурных решений, отсутствию указателей на разбросанные 
    по памяти объекты (уменьшение cache-miss), а так же минимально возможное 
    использование виртуальных вызовов (в идеале без них); Таким образом добиться 
    сокращения времени обмена сообщениями между клиентами и повысить уровень синхронизации
    между ними и сервером.

    Второстепенная цель - сделать на основе данной библиотеки сетевой модуль для
    репликации объектов в Godot engine.


## Статус
    Work In Progress..

## Версия
    0.5.7

## TODO
    0)  config module -> boost::program_options + {unit}.ini file
    1)  message exchange buffer class (main feature)
    2)  message ring buffer for received/send message
    3)  logger -> verbose level
    4)  event class
    5)  connection class
    6)  systemd service server/client
    7)  jitter udp control
    8)  RTT udp control
    N)  ...... other pretty stuff

## special
    - simple game with replication running by raylib
    - lib wrapper for Godot engine

## Authors

    - [I. Guskov](https://www.github.com/OneDeepSpace)
    - onegiy@gmail.com
