#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Hou Shaohui
# 
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou Shaohui <houshao55@gmail.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import gst
from player import Player
from logger import Logger


class Karaoke(Logger):
    def __init__(self):
        self.audiokaraoke = gst.element_factory_make('audiokaraoke')
        
    def release(self):    
        Player.bin.xfade_remove_filter(self.audiokaraoke)
        
    def enable_karaoke(self):    
        Player.bin.xfade_add_filter(self.audiokaraoke)
        
audio_karaoke = Karaoke()
        
def enable(exaile):
    audio_karaoke.enable_karaoke()

def disable(exaile):
    audio_karaoke.release()

