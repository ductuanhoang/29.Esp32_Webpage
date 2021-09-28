#!/bin/bash
get_idf
idf.py set-target esp32
idf.py build
