#!/bin/bash
systemctl restart getty@tty1.service
connmanctl tether wifi on
