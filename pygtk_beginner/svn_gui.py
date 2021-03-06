#!/usr/bin/env python

import pygtk,os
pygtk.require('2.0')
import gtk


class svn_gui:
    def func_destroy_main(self, widget, data=None):
        gtk.main_quit()
    
    def func_delete_event(widget, event, data=None):
        return False

    def func_check_out(self, widget, data=None):
        addr = self.entry.get_text()
        os.system("svn checkout https://phoenixforge.cs.uchicago.edu/svn/%s" % addr)

    def func_open_folder(self, widget, data=None):
        os.system("nautilus .")

    def func_svn_add(self, widget, data=None):
        filename = self.filew.get_filename()
        os.system("svn add %s" % filename)
    
    def func_svn_commit(self, widget, data=None):
        message = self.entry.get_text()
        filename = self.filec.get_filename()
        '''
        from subprocess import PIPE,Popen
        proc = Popen(["svn commit %s -m'%s'" % (filename,message)], stdout=PIPE)
        proc = Popen(["ls -al"], stdout=PIPE)
        proc = proc.communicate()[0].split()
        print "LOOK HERE" + proc
        buff.set_text("Committed sucessfully. We are now at revision %s" % proc[proc.index("revision")])
        self.msgbox.set_buffer(buff)
        '''
        self.printmsg("Committing. Please wait...")
        os.system("svn commit %s -m'%s'" % (filename, message))

    def func_choose_file_add(self, widget, data=None):
        if self.allfiles.get_active() is True:
            os.system("svn add ./*")
        else:
            self.filew = gtk.FileSelection("Choose file(s) to add...")
            # connect the ok_button to svn_add method
            self.filew.ok_button.connect("clicked", self.func_svn_add)
            self.filew.connect("destroy", lambda w: self.filew.destroy())
            self.filew.cancel_button.connect("clicked", lambda w: self.filew.destroy())
            self.filew.show()

    def func_check_message(self, widget, data=None):
        self.commit_resume = 0
        if self.entry.get_text():
            self.commit_resume = 1
            return
        else:
            self.warning = gtk.MessageDialog(type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_OK)
            self.warning.set_markup("You need to enter a commit message")
            # connect ok button to destroying the dialogue box
            self.warning.action_area.get_children()[0].connect("clicked", lambda w: self.warning.destroy())
            self.warning.run()
            
    def func_choose_file_commit(self, widget, data=None):
        if self.commit_resume == 0:
            return
        message = self.entry.get_text()
        if self.allfiles.get_active() is True:
            self.printmsg("Committing. Please wait...")
            os.system("svn commit ./* -m'%s'" % message)
        else:
            self.printmsg("Choosing file(s)...")
            self.filec = gtk.FileSelection("Choose file(s) to commit...")
            # connect the ok_button to svn_commit method
            self.filec.ok_button.connect("clicked", self.func_svn_commit)
            self.filec.connect("destroy", lambda w: self.filec.destroy())
            self.filec.cancel_button.connect("clicked", lambda w: self.filec.destroy())
            self.filec.show()

    def func_svn_update(self, widget, data=None):
        os.system("svn update")

    # make all widgets within the window show up
    def show(self):
        self.entry.show()
        self.msgbox.show()
        self.vbox.show()
        self.hbox1.show()
        self.hbox2.show()
        self.checkout.show()
        self.openfolder.show()
        self.add.show()
        self.commit.show()
        self.update.show()
        self.allfiles.show()
        self.closewindow.show()
        self.window.show()

    # bind event handlers
    def bind(self):
        self.window.connect("destroy", self.func_destroy_main)
        self.window.connect("delete_event", self.func_delete_event)
        self.checkout.connect("clicked", self.func_check_out)
        self.openfolder.connect("clicked", self.func_open_folder)
        self.add.connect("clicked", self.func_choose_file_add)
        self.commit.connect("clicked", self.func_check_message)
        self.commit.connect("clicked", self.func_choose_file_commit)
        self.update.connect("clicked", self.func_svn_update)
        self.closewindow.connect("clicked", self.func_destroy_main)

    # pack widgets into the window
    def pack(self):
        self.window.add(self.vbox)
        self.vbox.pack_start(self.entry,True, True,0)
        self.vbox.pack_start(self.msgbox,True, True,0)
        self.vbox.pack_start(self.hbox1,False, False,0)
        self.vbox.pack_start(self.hbox2,False, False,0)
        self.hbox1.pack_start(self.checkout,False,True,0)
        self.hbox1.pack_start(self.openfolder,False,True,0)
        self.hbox1.pack_start(self.add,False,True,0)
        self.hbox1.pack_start(self.commit,False,True,0)
        self.hbox1.pack_start(self.update,False,True,0)
        self.hbox2.pack_start(self.allfiles,False,True,0)
        self.hbox2.pack_end(self.closewindow,False,True,0)

    # initialize all widgets
    def widgets_init(self):
        # initialze window
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_title("SVN Graphical Interface")
        self.window.set_size_request(400,200)
        # initialize containers
        self.vbox = gtk.VBox(False, 5)
        self.hbox1 = gtk.HBox(False,10)
        self.hbox2 = gtk.HBox(False,10)
        # initialze functional widgets
        self.checkout = gtk.Button("Check Out")
        self.openfolder = gtk.Button("Open Folder")
        self.add = gtk.Button( "Add...")
        self.commit = gtk.Button( "Commit...")
        self.update = gtk.Button( "Update")
        self.allfiles = gtk.CheckButton("Add/Commit All Files")
        self.closewindow = gtk.Button("Close Window")
        self.entry = gtk.Entry(200)
        self.msgbox = gtk.TextView()
        self.msgbox.set_editable(False)
        self.buff = self.msgbox.get_buffer()

    # show text message in msgbox
    def printmsg(self, s):
        self.buff.set_text(s)

    def __init__(self):
        '''
        Widget Hierarchy:
        window - vbox - entry field
                      - msgbox
                      - hbox1        - svn check out button
                                     - openfolder button
                                     - svn add button
                                     - svn commit button
                                     - svn update button
                      - hbox2        - checkbutton for whether all files
                                     - close window button
        '''
        # a global variable to keep track of whether a commit message is present
        self.commit_resume = 0
        # initialize all widgets
        self.widgets_init()
        # handler bindings
        self.bind()
        # widget packing
        self.pack()
        # show up
        self.show()

    def main(self):
        gtk.main()

if __name__ == "__main__":
    svn_gui = svn_gui()
    svn_gui.main()
