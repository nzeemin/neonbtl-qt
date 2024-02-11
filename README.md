# neonbtl-qt
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![Build Status](https://github.com/nzeemin/neonbtl-qt/actions/workflows/push-matrix.yml/badge.svg?branch=main)](https://github.com/nzeemin/neonbtl-qt/actions/workflows/push-matrix.yml)

**Neon Back to Life!** is an emulator of Soviet computer **Sojuz-Neon PK 11/16**.
This computer was manufactured in 1991-1992 at the "Kvant" plant in Zelenograd (near Moscow), and a total of 200-1000 units were produced.

Soyuz-Neon is based on the N1806VM2 processor, so it is partially compatible with machines such as DVK, UKNC, NEMIGA, and in general inherits the instruction set and architecture from the DEC PDP-11 line of machines.

This repository is for Qt version of the NeonBTL emulator.

Emulator status: Work in progress.


## Как запустить под Linux

### Собрать из исходников

 1. Установить пакеты: Qt 5<br>
    `sudo apt install qtbase5-dev qt5-qmake`
 2. Скачать исходники: либо через<br>
    `git clone https://github.com/nzeemin/neonbtl-qt.git`<br>
    либо скачать как .zip и распаковать
 3. Выполнить команды:<br>
   `cd emulator`<br>
   `qmake "CONFIG+=release" QtNeonBtl.pro`<br>
   `make`<br>
 4. Дать права на выполнение: `chmod +x QtNeonBtl`
 5. Запустить `QtNeonBtl`

### Используя готовый AppImage

 1. Зайти в [Releases](https://github.com/nzeemin/neonbtl-qt/releases) найти последний AppImage-релиз и скачать `*.AppImage` файл
 2. Дать права на выполнение: `chmod +x NeonBTL_Qt-xxxxxxx-x86_64.AppImage` (подставить тут правильное название AppImage файла)
 3. Запустить AppImage файл
