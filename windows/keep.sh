#!/usr/bin/env bash

for D in `find . -type d`
                                      do
                                          touch "${D}"/.gitkeep
                                      done
