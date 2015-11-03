#!/usr/bin/env python

# example base.py

import pygtk
pygtk.require('2.0')
import gtk

class Base:
    def destroy_handler(self, widget, data=None):
        gtk.main_quit()
    
    def delete_event_handler(widget, event, data=None):
        return False

    def print_date(self, widget, data=None):
        import os
        os.system("date")

    def __init__(self):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("destroy", self.destroy_handler)
        self.window.connect("delete_event", self.delete_event_handler)
        self.window.set_border_width(20)
        self.dateb = gtk.Button("check date!")
        self.dateb.connect("clicked", self.print_date)
        self.dateb.connect("clicked", self.destroy_handler)
        self.window.add(self.dateb)
        self.dateb.show()
        self.window.show()

    def main(self):
        gtk.main()

print __name__
if __name__ == "__main__":
    base = Base()
    base.main()
