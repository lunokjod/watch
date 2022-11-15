# lilyGo TWatch 2020 v3 firmware

* This folder contains the core of lunokIoT Watch

## System

**SystemEvents** manage the interrupts and system bus

**esp32-ulp** ulp tasks (@TODO)

## User interface:

**Application** Describes *LunokIoTApplication* the base for any UI application

**Activator/ActiveRect** Define in cartesian coordinates an active area with callback (base for widgets) can be used as circular area too

**UI** Helpers for build user interface and the UI event loop

**BootSplash** Simple TFT eyecandy during boot process (theme choose color)

## Datasources:

**KVO** This class can monitor system bus messages and launch callback when changes
