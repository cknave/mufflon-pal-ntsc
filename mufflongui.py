#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Mufflon converter GUI
#
# (c) 2010 Martin Wendt
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Martin Wendt c64@enthusi.de

import tkinter
import time
import sys
import tkinter.filedialog
import tkinter.messagebox
from PIL import Image, ImageTk
import tempfile
import os
import shutil
import time
import subprocess
#import world peace
#import enthusi
#import onslaught
#--------------------
global image
global backup
global result_mode
global source_mode
global dest_mode
global preview_mode
global converter_mode
global source_photo
global preview_photo
global result_photo
global error_photo

global m_image
global m_backup
global m_result_mode
global m_source_mode
global m_dest_mode
global m_preview_mode
global m_errormap
global m_converter_mode
global m_source_photo
global m_preview_photo
global m_result_photo
global m_error_photo
global m_flicker_photo

global tmp_path
global fontsize
global shaded
global unshaded
global activated
global inactivated
global mufflon_file
global infobox
global allhopelost

allhopelost=0
fontsize=10
smallfontsize=8

#Usage: mufflon <filename> --otype <output-type> --flibug --multipass --shutup --no-truncate --block-sprite-col <0-15> --itype <input-type> --imode <rastered/blended> [-o <filename>.nuf -p [--min <number> --max <number> --pname <name>.bmp]] --solid --src-palette <deekay/pepto> --dest-palette <deekay/pepto> --bugcol <0-15> --2ink --2paper --2sprite --anything-goes -b -d
global fbugcolors
fbugcolors=["Black","White","Red","Cyan","Purple","Green","Blue","Yellow","Orange","Brown",
            "Light Red","Dark Grey","Grey","Light Green","Light Blue","Light Grey"]

global native_formats
native_formats=["gun","fun","fp2","drl"]

global interlace_formats
interlace_formats=["gun","fun","fp2","drl"]

global ifli_formats
ifli_formats=["gun","fun","fp2"]

global drl_formats
drl_formats=["drl"]

global accepted_types
accepted_types = (".bmp",".gun",".fun",".fp2",".drl",".jpg",".jpeg",".png",".gif",
                    ".BMP",".GUN",".FUN",".FP2",".DRL",".JPG",".JPEG",".PNG",".GIF" )


def call(*args, **kwargs):
          print('start')
          p=subprocess.Popen(*args, **kwargs)
          while p.poll() == None:
            #infobox.update()
            time.sleep(0.01)

          #return subprocess.Popen(*args, **kwargs).poll()

#------------------------
class notebook:

    def __init__(self, master, side=tkinter.LEFT):

        self.active_fr = None
        self.count = 0
        self.choice = tkinter.IntVar(0)
        if side in (tkinter.TOP, tkinter.BOTTOM):
            self.side = tkinter.LEFT
        else:
            self.side = tkinter.TOP
        
        self.rb_fr = tkinter.Frame(master, borderwidth=2, relief=tkinter.RIDGE)
        self.rb_fr.pack(side=side, fill=tkinter.BOTH)
        self.screen_fr = tkinter.Frame(master, borderwidth=2, relief=tkinter.RIDGE)
        self.screen_fr.pack(fill=tkinter.BOTH)
        

    def __call__(self):

        return self.screen_fr

    def add_screen(self, fr, title):
        
        b = tkinter.Radiobutton(self.rb_fr, text=title, \
            variable=self.choice, value=self.count, \
            command=lambda: self.display(fr))
        b.pack(fill=tkinter.BOTH, side=self.side)
        if not self.active_fr:
            fr.pack(fill=tkinter.BOTH, expand=1)
            self.active_fr = fr

        self.count += 1
        return b
    def display(self, fr):
        
        self.active_fr.forget()
        fr.pack(fill=tkinter.BOTH, expand=1)
        self.active_fr = fr
#------------------------------------
#nufli commands
def Preview_On():
    global preview_photo
    Do_Preview()
    LPreview.config(image=preview_photo)
    LPreview.image=preview_photo

def Preview_Off():
    global source_photo
    LPreview.config(image=source_photo)
    LPreview.image=source_photo

def Resultview_On():
    global result_photo
    RPreview.config(image=result_photo)
    RPreview.image=result_photo

def Resultview_Off():
    global error_photo
    RPreview.config(image=error_photo)
    RPreview.image=error_photo

def SMode_changed():
    smode=source_mode.get()
    if W9.cget('state')=='disabled':
      source_mode.set(dest_mode.get())

    Do_Preview()
    if smode=='p':
      Wa.config(state='normal')
      Wb.config(state='normal')
      S1.config(state='normal')
      S2.config(state='normal')

    else:
      Wa.config(state='disabled')
      Wb.config(state='disabled')
      S1.config(state='disabled')
      S2.config(state='disabled')

def Do_Preview(*bla):
    lmin=lmin_scale.get()
    lmax=lmax_scale.get()
    if lmax==0:
      lmax_scale.set(2)
    elif lmin>=lmax:
      lmin_scale.set(lmax-2)
    Convert(1)
    
def SavePreview():
    Convert(2)
    
def Convert(*optional):
    global result_photo
    global error_photo
    global preview_photo
    global preview_mode
    global result_mode
    global errormap
    global tmp_path
    global mufflon_file
    global infobox
    pcomm=[]

    if W5.get()=='':
      return
    
    #preset to 0
    p_only=0
    if len(optional) > 0:
      p_only=int(optional[0])
      preview_mode.set('on')

    pcomm.append(mufflon_file)

    source_file = W5.get()
    source_path = os.path.dirname(source_file)
    source_file_name = os.path.basename(source_file)
    
    #OK (puuh) now check if the input-entry is a native one
    if source_file[-3:].lower() not in native_formats:
       #print 'checking bmp in %s' % tmp_path
       source_path = tmp_path
       source_file_name=source_file_name[:-4]+'.bmp'
       source_file=os.path.join(source_path,source_file_name)

    target_file=W6.get()
      
    #is suffix= '.nuf'?  
    if target_file[-4:]!='.nuf':
       target_file+='.nuf'
      
    target_path=os.path.dirname(target_file)
    target_file_name=os.path.basename(target_file)

    #update entry
    W6.delete(0, tkinter.END)
    W6.insert(0,target_file)

    pcomm.append(str(source_file))

    #creat basename: '/path/image.nuf' -> 'image'
    target_base=os.path.join(tmp_path,target_file_name[0:-4])
    
    preview_file_name   = target_base+'_preconvert.bmp'
    nuf_file_name       = target_base+'.nuf'
    result_file_name    = target_base+'_result.bmp'
    error_file_name     = target_base+'_errormap.bmp'
    
    preview_file = os.path.join(tmp_path,preview_file_name)
    nuf_file     = os.path.join(tmp_path,nuf_file_name)
    result_file  = os.path.join(tmp_path,result_file_name)
    error_file    = os.path.join(tmp_path,error_file_name)

    #handle ifile format
    if source_file_name[-3:].lower() in ifli_formats:
      pcomm.extend(['--itype','ifli'])
      pcomm.extend(['--imode','%s' % inputlace.get()])

    elif source_file_name[-3:].lower() in drl_formats:
      pcomm.extend(['--itype','drl','--imode','%s' % (inputlace.get())])
      
    else:
      pcomm.extend(['--itype','bmp'])
      pcomm.extend(['--otype','nufli','--solid'])

    if source_mode.get()!='p':
      pcomm.extend(['--src-palette','%s' % (source_mode.get())])
    else:
      #trucolor
      if lmax_scale.get() <= lmin_scale.get():
        #warning
        tkinter.messagebox.showwarning("Luminance Error:","Max Luminance setting must be\nlarger than Min Luminance!")
        lmin=lmin_scale.get()
        lmax=lmax_scale.get()
        if lmax==0:
          lmax_scale.set(2)
        elif lmin>=lmax:
          lmin_scale.set(lmax-2)
        return
      pcomm.append('-p')
      pcomm.extend(['--max','%1.2f' % (float(lmax_scale.get()/50.0))])
      pcomm.extend(['--min','%1.2f' % (float(lmin_scale.get()/50.0))])

    pcomm.extend(['--dest-palette','%s' % (dest_mode.get())])

    if multipass.get()=='on':
      pcomm.append('--multipass')

    if flibug.get()=='on':
      pcomm.append('--flibug')

    pcomm.append('--no-truncate')

    if p_only!=0:
        pcomm.append('--solid')
        pcomm.extend(['--otype','bmp'])
        pcomm.extend(['-o','%s' % (str(preview_file))])
    else:
        pcomm.extend(['-o','%s' % (str(nuf_file))])
        
        infobox=tkinter.Toplevel()
        infomsg=tkinter.Message(infobox,anchor='w',width=200,padx=10,pady=10, text='''Conversion in progress!
Depending on your settings,
this may take a while...''')
        infomsg.pack()
        infobox.update()

    print(pcomm)
    subprocess.call(pcomm)
    #p1 = Popen(["mufflon"], stdout=PIPE)
    #p2 = Popen(["grep", "hda"], stdin=p1.stdout, stdout=PIPE)
    #output = p2.communicate()[0]


    if p_only==1:
      preview_image=Image.open(preview_file)
      preview_photo = ImageTk.PhotoImage(preview_image)
      LPreview.config(image=preview_photo)
      LPreview.image=preview_photo
      return
  
    if p_only==0:
      print('copy "%s" to "%s"' % (nuf_file,target_path))
      try:
        shutil.copy (nuf_file, target_path)
      except:
        tkinter.messagebox.showwarning('Ooops!','Something went wrong')
        infobox.destroy()
      infobox.destroy()
   
    #if SavePreview called this:
    if p_only==2:
      got_path = tkinter.filedialog.asksaveasfilename(title="Save Pre-Convert as", filetypes=[("bmp file",".bmp"),("All files",".*")],initialdir=target_path,initialfile=os.path.basename(preview_file))
      if not got_path:
        got_path='.'
      if got_path=='.':
        return
      save_preview_path = got_path
      print('copy "%s" to "%s"' % (preview_file,save_preview_path))
      try:
        shutil.copy (preview_file, save_preview_path)
      except:
        tkinter.messagebox.showwarning('Ooops!','Something went wrong')
        infobox.destroy()
      
    #update images
    if p_only==0:
      error_image=Image.open(error_file)
      error_photo = ImageTk.PhotoImage(error_image)
      if result_mode.get()=='off':
        RPreview.config(image=error_photo)
        RPreview.image=error_photo
 
      result_image=Image.open(result_file)
      result_photo = ImageTk.PhotoImage(result_image)
      if result_mode.get()=='on':
        RPreview.config(image=result_photo)
        RPreview.image=result_photo

    if preview_mode.get()=='on':
      preview_image=Image.open(preview_file)
      preview_photo = ImageTk.PhotoImage(preview_image)
      LPreview.config(image=preview_photo)
      LPreview.image=preview_photo
    
def Select_Open():
    global source_photo
    global accepted_types
    got_path = tkinter.filedialog.askopenfilename(title="Open file", filetypes=[("Image file",accepted_types),("All files",".*")])
    if not got_path:
      got_path='.'
    if got_path=='.':
      return
    file_path=got_path
    #print got_path
    file_path=os.path.normpath(file_path)
    original_file_path=file_path
    W5.delete(0, tkinter.END)
    W5.insert(0,file_path)
    
    
    if file_path[-3:].lower() in interlace_formats:
      LA.config(state='normal')
      L0.config(state='normal')
      preview_mode.set('on')
      W6.delete(0, tkinter.END)
      W6.insert(0,file_path[0:-4]+'.nuf')
      #source pal is set to dest_pal and inactivated
      W9.config(state='disabled')
      B_1.config(state='disabled')
      B_2.config(state='disabled')
      B_3.config(state='disabled')
      source_mode.set(dest_mode.get())
      Do_Preview()
      return

    else:
      LA.config(state='disabled')
      L0.config(state='disabled')
      W9.config(state='normal')
      B_1.config(state='normal')
      B_2.config(state='normal')
      B_3.config(state='normal')

    if file_path[-3:].lower() not in native_formats:
      print('using own loader')
      source_image=Image.open(file_path)
      variable=source_image.convert("RGB")
      #paranoia mode requested by Deekay
      if variable.size != (320,200):
        variable=variable.resize((320,200), Image.NEAREST)
        print('** resizing')
      base_name=os.path.basename(file_path)[0:-4]
      base_name12=base_name.replace('.','_')

      temporary_bmp_file=os.path.join(tmp_path,(base_name12+'.bmp'))
      file_path=temporary_bmp_file
      variable.save(temporary_bmp_file,"BMP")
      print('**** bmp: %s ' % temporary_bmp_file)
      W5.delete(0, tkinter.END)
      W5.insert(0,original_file_path)
  
    #set output if still blank
    W6.delete(0, tkinter.END)
    W6.insert(0,original_file_path[0:-4]+'.nuf')

    source_image=Image.open(file_path)
    source_photo = ImageTk.PhotoImage(source_image)
    LPreview.config(image=source_photo)
    LPreview.image=source_photo
    
    if preview_mode.get()=='on':
      Do_Preview()
    return file_path

def Select_Save():
    got_path = tkinter.filedialog.asksaveasfilename(title="Save file", filetypes=[("nuf file",".nuf"),("All files",".*")])
    if not got_path:
      got_path='.'
    if got_path=='.':
      return
    file_path = got_path
    W6.delete(0, tkinter.END)
    W6.insert(0,file_path)
    return file_path
#--------------------
#muifli commands
def m_Preview_On():
    global m_preview_photo
    m_Do_Preview()
    m_LPreview.config(image=m_preview_photo)
    m_LPreview.image=m_preview_photo

def m_Preview_Off():
    global m_source_photo
    m_LPreview.config(image=m_source_photo)
    m_LPreview.image=m_source_photo

def m_Resultview_On():
    global m_result_photo
    m_RPreview.config(image=m_result_photo)
    m_RPreview.image=m_result_photo

def m_Resultview_Off():
    global m_error_photo
    global m_flicker_photo
    global m_result_mode
    if m_result_mode.get()=='onf':
      m_RPreview.config(image=m_flicker_photo)
      m_RPreview.image=m_flicker_photo
    else:
      m_RPreview.config(image=m_error_photo)
      m_RPreview.image=m_error_photo

def m_SMode_changed():
    m_smode=m_source_mode.get()
    if m_W9.cget('state')=='disabled':
      m_source_mode.set(m_dest_mode.get())
    #always do preview:
    m_Do_Preview()
    if m_smode=='p':
      m_Wa.config(state='normal')
      m_Wb.config(state='normal')
      m_S1.config(state='normal')
      m_S2.config(state='normal')

    else:
      m_Wa.config(state='disabled')
      m_Wb.config(state='disabled')
      m_S1.config(state='disabled')
      m_S2.config(state='disabled')

def m_Do_Preview(*bla):
   
    m_Convert(1)
    
def m_SavePreview():
    m_Convert(2)
    
def m_Convert(*m_optional):
    global m_result_photo
    global m_error_photo
    global m_flicker_photo
    global m_preview_photo
    global m_preview_mode
    global m_result_mode
    global m_errormap
    global tmp_path
    global fbugcol
    global fbugcolors
    global mufflon_file
    global infobox
    m_pcomm=[]
    
    if m_W5.get()=='':
      return
    #preset to 0
    m_p_only=0
    if len(m_optional) > 0:
      m_p_only=int(m_optional[0])
      m_preview_mode.set('on')

    m_pcomm.append(mufflon_file)
        
    source_file = m_W5.get()
    source_path = os.path.dirname(source_file)
    source_file_name = os.path.basename(source_file)
    
    #OK (puuh) now check if the input-entry is a native one
    if source_file[-3:].lower() not in native_formats:
       #print 'checking bmp in %s' % tmp_path
       source_path = tmp_path
       source_file_name=source_file_name[:-4]+'.bmp'
       source_file=os.path.join(source_path,source_file_name)

    target_file=m_W6.get()
      
    #is suffix= '.nuf'?  
    if target_file[-4:]!='.mui':
       target_file+='.mui'
      
    target_path=os.path.dirname(target_file)
    target_file_name=os.path.basename(target_file)

    #update entry
    m_W6.delete(0, tkinter.END)
    m_W6.insert(0,target_file)

    m_pcomm.append(str(source_file))

    #creat basename: '/path/image.nuf' -> 'image'
    target_base=os.path.join(tmp_path,target_file_name[0:-4])
    
    preview_file_name   = target_base+'_preconvert.bmp'
    mui_file_name       = target_base+'.mui'
    result_file_name    = target_base+'_result.bmp'
    error_file_name     = target_base+'_errormap.bmp'
    flicker_file_name     = target_base+'_flickermap.bmp'
    
    preview_file = os.path.join(tmp_path,preview_file_name)
    mui_file     = os.path.join(tmp_path,mui_file_name)
    result_file  = os.path.join(tmp_path,result_file_name)
    error_file    = os.path.join(tmp_path,error_file_name)
    flicker_file    = os.path.join(tmp_path,flicker_file_name)
    
    #handle ifile format
    if source_file_name[-3:].lower() in ifli_formats:
      m_pcomm.extend(['--itype','ifli'])
      m_pcomm.extend(['--imode','%s' % m_inputlace.get()])

    elif source_file_name[-3:].lower() in drl_formats:
      m_pcomm.extend(['--itype','drl','--imode','%s' % (m_inputlace.get())])

    else:
      m_pcomm.extend(['--itype','bmp'])
    
    #set to muifli always here
    m_pcomm.extend(['--otype','muifli'])
    
    if m_source_mode.get()!='p':
      m_pcomm.extend(['--src-palette','%s' % (m_source_mode.get())])
    else:
      #trucolor
      if m_lmax_scale.get() <= m_lmin_scale.get():
        
        lmin=m_lmin_scale.get()
        lmax=m_lmax_scale.get()
        if lmax==0:
          m_lmax_scale.set(2)
        elif lmin>=lmax:
            m_lmin_scale.set(lmax-2)
        return
      m_pcomm.append('-p')
      m_pcomm.extend(['--max','%1.2f' % (float(m_lmax_scale.get()/50.0))])
      m_pcomm.extend(['--min','%1.2f' % (float(m_lmin_scale.get()/50.0))])

    m_pcomm.extend(['--dest-palette','%s' % (m_dest_mode.get())])

    m_pcomm.append('--no-truncate')

    if m_inks.get()=='on':
      m_pcomm.append('--2ink')

    if m_papers.get()=='on':
      m_pcomm.append('--2paper')

    if m_sprites.get()=='on':
      m_pcomm.append('--2sprite')

    if m_deflicker.get()=='on':
      m_pcomm.append('-d')

    if m_bruteforce.get()=='on':
      m_pcomm.append('-b')

    m_pcomm.extend(['--bugcol','%d' % (fbugcolors.index(fbugcol.get()))])

    #if preview_mode.get()=='on':
    if m_p_only!=0:
        m_pcomm.extend(['--otype','bmp'])
        m_pcomm.extend(['-o','%s' % (str(preview_file))])
    else:
        m_pcomm.extend(['-o','%s' % (str(mui_file))])
        infobox=tkinter.Toplevel()
        infomsg=tkinter.Message(infobox,anchor='w',width=200,padx=10,pady=10, text='''Conversion in progress!
Depending on your settings,
this may take a while...''')
        infomsg.pack()
        infobox.update()
    
    print("this will be done:")
		
    print(m_pcomm)
    subprocess.call(m_pcomm)

    if m_p_only==1:
      m_preview_image=Image.open(preview_file)
      m_preview_photo = ImageTk.PhotoImage(m_preview_image)
      m_LPreview.config(image=m_preview_photo)
      m_LPreview.image=m_preview_photo
      return
    #copy wanted images
    #if not preview: copy result.nuf
    if m_p_only==0:
      print('copy "%s" to "%s"' % (mui_file,target_path))
      try:
        shutil.copy (mui_file, target_path)
      except:
        tkinter.messagebox.showwarning('Ooops!','Something went wrong')
        infobox.destroy()
      infobox.destroy()
   
    #if SavePreview called this:
    if m_p_only==2:
      got_path = tkinter.filedialog.asksaveasfilename(title="Save Pre-Convert as", filetypes=[("bmp file",".bmp"),("All files",".*")],initialdir=target_path,initialfile=os.path.basename(preview_file))
      if not got_path:
        got_path='.'
      if got_path=='.':
        return
      save_preview_path = got_path
      print('copy "%s" to "%s"' % (preview_file,save_preview_path))
      try:
        shutil.copy (preview_file, save_preview_path)
      except:
        tkinter.messagebox.showwarning('Ooops!','Something went wrong')
        infobox.destroy()
      
    #update images
    if m_p_only==0:
      #1
      m_error_image=Image.open(error_file)
      m_error_photo = ImageTk.PhotoImage(m_error_image)

      if m_result_mode.get()=='off':
        m_RPreview.config(image=m_error_photo)
        m_RPreview.image=m_error_photo
      #1
 
      #2
      m_result_image=Image.open(result_file)
      m_result_photo = ImageTk.PhotoImage(m_result_image)

      if m_result_mode.get()=='on':
        m_RPreview.config(image=m_result_photo)
        m_RPreview.image=m_result_photo
      #2
      
      #3
      m_flicker_image=Image.open(flicker_file)
      m_flicker_photo = ImageTk.PhotoImage(m_flicker_image)

      if m_result_mode.get()=='onf':
        m_RPreview.config(image=m_flicker_photo)
        m_RPreview.image=m_flicker_photo
      #4

    if m_preview_mode.get()=='on':
      m_preview_image=Image.open(preview_file)
      m_preview_photo = ImageTk.PhotoImage(m_preview_image)
      m_LPreview.config(image=m_preview_photo)
      m_LPreview.image=m_preview_photo
    
def m_Select_Open():
    global m_source_photo
    global accepted_types
    global tmp_path
    got_path = tkinter.filedialog.askopenfilename(title="Open file", filetypes=[("Image file",accepted_types),("All files",".*")])
    if not got_path:
      got_path='.'
    if got_path=='.':
      return
    file_path = got_path
    #print file_path
    file_path=os.path.normpath(file_path)
    #print file_path
    original_file_path=file_path
    m_W5.delete(0, tkinter.END)
    m_W5.insert(0,file_path)
    
    if file_path[-3:].lower() in interlace_formats:
      m_LA.config(state='normal')
      m_L0.config(state='normal')
      m_preview_mode.set('on')
      m_W6.delete(0, tkinter.END)
      m_W6.insert(0,file_path[0:-4]+'.mui')
      m_W9.config(state='disabled')
      m_B_1.config(state='disabled')
      m_B_2.config(state='disabled')
      m_B_3.config(state='disabled')
      m_source_mode.set(m_dest_mode.get())
      m_Do_Preview()
      return
      
    else:
        m_LA.config(state='disabled')
        m_L0.config(state='disabled')
        m_W9.config(state='normal')
        m_B_1.config(state='normal')
        m_B_2.config(state='normal')
        m_B_3.config(state='normal')

    if file_path[-3:].lower() not in native_formats:
      print('using own loader')
      source_image=Image.open(file_path)
      variable=source_image.convert("RGB")
      #paranoia mode
      if variable.size != (320,200):
        print(variable.size)
        variable=variable.resize((320,200), Image.NEAREST)
        print('** resizing')
      base_name=os.path.basename(file_path)[0:-4]
      base_name12=base_name.replace('.','_')

      temporary_bmp_file=os.path.join(tmp_path,(base_name12+'.bmp'))
      file_path=temporary_bmp_file
      variable.save(temporary_bmp_file,"BMP")
      print('**** bmp: %s ' % temporary_bmp_file)
      m_W5.delete(0, tkinter.END)
      m_W5.insert(0,original_file_path)

    #set output if still blank
    m_W6.delete(0, tkinter.END)
    m_W6.insert(0,original_file_path[0:-4]+'.mui')

    source_image=Image.open(file_path)
    m_source_photo = ImageTk.PhotoImage(source_image)
    m_LPreview.config(image=m_source_photo)
    m_LPreview.image=m_source_photo
    
    if m_preview_mode.get()=='on':
      m_Do_Preview()
    return file_path

def m_Select_Save():
    got_path = tkinter.filedialog.asksaveasfilename(title="Save file", filetypes=[("mui file",".mui"),("All files",".*")])
    if not got_path:
      got_path='.'
    if got_path=='.':
      return
    file_path = got_path
    m_W6.delete(0, tkinter.END)
    m_W6.insert(0,file_path)
    return file_path
#----------------------------------------------------
#new defs
def  m_StoreAll():
      #store all the generated bmp-images
      #required: result,error,flicker-file + target_path
      target_file=m_W6.get()
      target_path=os.path.dirname(target_file)
      target_file_name=os.path.basename(target_file)
      target_base=os.path.join(tmp_path,target_file_name[0:-4])
      result_file_name    = target_base+'_result.bmp'
      error_file_name     = target_base+'_errormap.bmp'
      flicker_file_name     = target_base+'_flickermap.bmp'
      result_file  = os.path.join(tmp_path,result_file_name)
      error_file    = os.path.join(tmp_path,error_file_name)
      flicker_file    = os.path.join(tmp_path,flicker_file_name)
      print('copy "%s" to "%s"' % (result_file,target_path))
      print('copy "%s" to "%s"' % (error_file,target_path))
      print('copy "%s" to "%s"' % (flicker_file,target_path))
      try:
        shutil.copy (result_file, target_path)
        shutil.copy (error_file, target_path)
        shutil.copy (flicker_file, target_path)
      except:
        tkinter.messagebox.showwarning('Ooops!','Something went wrong')
        infobox.destroy()
  
def StoreAll():
      #store all the generated bmp-images
      #required: result,error,flicker-file + target_path
      target_file=W6.get()
      target_path=os.path.dirname(target_file)
      target_file_name=os.path.basename(target_file)
      target_base=os.path.join(tmp_path,target_file_name[0:-4])
      result_file_name    = target_base+'_result.bmp'
      error_file_name     = target_base+'_errormap.bmp'
      result_file  = os.path.join(tmp_path,result_file_name)
      error_file    = os.path.join(tmp_path,error_file_name)
      print('copy "%s" to "%s"' % (result_file,target_path))
      print('copy "%s" to "%s"' % (error_file,target_path))
      try:
        shutil.copy (result_file, target_path)
        shutil.copy (error_file, target_path)
      except:
        tkinter.messagebox.showwarning('Ooops!','Something went wrong')
        infobox.destroy()
#----------------------------------------------------
print('GUI Version 0.08 / 05.08.2010')
a = tkinter.Tk()
n = notebook(a, tkinter.TOP)
root=tkinter.Frame(n())
evil=tkinter.Frame(n())
#root = Tkinter.Tk()
a.title('Mufflon - High Quality Commodore 64 Imageconverter')
a.resizable(tkinter.FALSE,tkinter.FALSE)

tmp_path=tempfile.mkdtemp()

#check for mufflon binary
current_path=os.getcwd()
if os.path.exists(os.path.join(current_path,'mufflon')):
  print('found mufflon in current dir')
  mufflon_file = './mufflon'
else:
  print('assuming mufflon to be in PATH')
  mufflon_file = 'mufflon'
  
print('running on',sys.platform)
if sys.platform in ('win32','win64'):
    mufflon_file = 'mufflon.exe '
    print('windows detected')
    allhopelost=1
    if os.path.exists(os.path.join(current_path,'mufflon.exe')):
      print('%s found in current dir' % mufflon_file)
    else:
      tkinter.messagebox.showwarning('mufflon not found','"%s"\nnot found in current\ndirectory!' % mufflon_file)
      sys.exit()
       
#from here on, split nufli/muifli

preview=tkinter.StringVar()
preview.set('off')

dither=tkinter.IntVar()
dither.set(0)
row=0

W3=tkinter.Label(root,text='Input File:')
W3.grid(row=row,column=0,sticky='E')

W5=tkinter.Entry(root)
W5.grid(row=row,column=1,columnspan=6,sticky='WE')

W7=tkinter.Button(root,text='Select...',command=Select_Open)
W7.grid(row=row,column=7,sticky='WE',padx=5)
row+=1

T1=tkinter.Label(root,text='Input filetypes can be any PC gfx, IFLI (.gun/.fun/.fp2) or Drazlace (.drl)',font=("Helvetica",fontsize))
T1.grid(row=row,column=1,columnspan=6,sticky='W')
row+=1

W4=tkinter.Label(root,text='Output File (.nuf):')
W4.grid(row=row,column=0,sticky='E')

W6=tkinter.Entry(root)
W6.grid(row=row,column=1,columnspan=6,sticky='WE')

W8=tkinter.Button(root,text='Select...',command=Select_Save)
W8.grid(row=row,column=7,sticky='WE',padx=5)
row+=1

T2=tkinter.Label(root,text=' Note: Every image is also executable with SYS12288 / JMP $3000',font=("Helvetica",fontsize))
T2.grid(row=row,column=1,columnspan=5,sticky='W')
row+=1

W9=tkinter.Label(root,text='Source Palette',font=("Helvetica",fontsize))
W9.grid(row=row,column=0,sticky='W')

Wc=tkinter.Label(root,text='Destination Palette',font=("Helvetica",fontsize))
Wc.grid(row=row,column=4,sticky='W')
row+=1

F1=tkinter.Frame(root,borderwidth=5,relief='ridge')
F1.grid(row=row,column=0,columnspan=4,sticky='WE')

F2=tkinter.Frame(root,borderwidth=5,relief='ridge')
F2.grid(row=row,column=4,columnspan=1,sticky='NWE')

F3=tkinter.Frame(root)
F3.grid(row=row,column=5,columnspan=3,sticky='NWE')

flibug=tkinter.StringVar()
flibug.set('on')

multipass=tkinter.StringVar()
multipass.set('off')

preview_mode='off'
result_mode='on'

C2=tkinter.Checkbutton(F3, text='Render FLI-bug',variable=flibug,onvalue='on',offvalue='off')
C2.grid(row=5,column=6,columnspan=2,sticky='W')

C3=tkinter.Checkbutton(F3, text='FLI-bug Multiple Passes (slow)',variable=multipass,onvalue='on',offvalue='off')
C3.grid(row=6,column=6,columnspan=2,sticky='W')

row+=1

F4=tkinter.Frame(root,borderwidth=5,relief='ridge')
F4.grid(row=row,column=0,columnspan=4,sticky='NWE')

F5=tkinter.Frame(root,borderwidth=5,relief='ridge')
F5.grid(row=row,column=4,columnspan=4,sticky='NWE')

preview_mode = tkinter.StringVar()
preview_mode.set('off')

result_mode = tkinter.StringVar()
result_mode.set('on')

P1 = tkinter.Radiobutton(F4, text='Original',variable=preview_mode, value='off',command=Preview_Off)
P1.grid(row=0,column=1,sticky='E')
P2 = tkinter.Radiobutton(F4, text='Pre-Convert',variable=preview_mode, value='on',command=Preview_On)
P2.grid(row=0,column=2,sticky='W')

P3 = tkinter.Radiobutton(F5, text='Result',variable=result_mode, value='on',command=Resultview_On)
P3.grid(row=0,column=1,sticky='E')
P4 = tkinter.Radiobutton(F5, text='Errormap',variable=result_mode, value='off',command=Resultview_Off)
P4.grid(row=0,column=2,sticky='W')

source_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))
result_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))
preview_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))
error_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))

LPreview = tkinter.Label(F4)
LPreview.config(image=source_photo)
LPreview.grid(column=0,columnspan=4,row=1)

RPreview = tkinter.Label(F5)
RPreview.config(image=result_photo)
RPreview.grid(column=0,columnspan=4,row=1)

row+=1

L0=tkinter.Label(root,text='Input Interlace\nprocessing:',font=("Helvetica",fontsize),state='disabled')
L0.grid(row=row,sticky='W')

inputlace = tkinter.StringVar()
inputlace.set("rastered") # initial value

LA = tkinter.OptionMenu(root, inputlace, "rastered", "blended",command=Do_Preview)
LA.config(width=6,state='disabled')
LA.grid(row=row,column=1,sticky='WE')

L1=tkinter.Button(root,text='Save Pre-Convert',command=SavePreview)
L1.grid(row=row,column=2,columnspan=2,sticky='WE')

L2=tkinter.Button(root,text='Convert',command=Convert)
L2.grid(row=row,column=7,sticky='w',padx=20)
#L2.grid(row=row,column=7,sticky='E')

C1=tkinter.Button(root, text='Store Result-& Errormap',command=StoreAll)
C1.grid(row=row,column=4,columnspan=2,sticky='W')

MODES = [("Pepto", "pepto"),("DeeKay", "deekay"),("Prepare Truecolor", "p")]
source_mode = tkinter.StringVar()
source_mode.set("pepto")

i=0
text,mode = MODES[i]
B_1 = tkinter.Radiobutton(F1, text=text,variable=source_mode, value=mode,command=SMode_changed)
B_1.grid(row=i,column=0,sticky='W')
i+=1
text,mode = MODES[i]
B_2 = tkinter.Radiobutton(F1, text=text,variable=source_mode, value=mode,command=SMode_changed)
B_2.grid(row=i,column=0,sticky='W')
i+=1
text,mode = MODES[i]
B_3 = tkinter.Radiobutton(F1, text=text,variable=source_mode, value=mode,command=SMode_changed)
B_3.grid(row=i,column=0,sticky='W')

Wa=tkinter.Label(F1,text='Luminance Min',font=("Helvetica",smallfontsize),state='disabled')
Wa.grid(row=0,column=3,sticky='E')
lmin_scale=tkinter.IntVar()
lmin_scale.set(0)
S1 = tkinter.Scale(F1, from_=0, to=100,resolution=2,orient=tkinter.HORIZONTAL,font=("Helvetica",smallfontsize),state='disabled',variable=lmin_scale,command=Do_Preview)
S1.grid(row=0,column=4,sticky='E')

Wb=tkinter.Label(F1,text='Luminance Max',font=("Helvetica",smallfontsize),state='disabled')
Wb.grid(row=1,column=3,sticky='E')
lmax_scale=tkinter.IntVar()
lmax_scale.set(50)
S2 = tkinter.Scale(F1, from_=0, to=100,resolution=2,orient=tkinter.HORIZONTAL,font=("Helvetica",smallfontsize),state='disabled',variable=lmax_scale,command=Do_Preview)
S2.grid(row=1,column=4,sticky='E')

MODES2 = [("Pepto", "pepto"),("DeeKay", "deekay")]
dest_mode = tkinter.StringVar()
dest_mode.set("pepto")
i=0
for text, mode in MODES2:
        C = tkinter.Radiobutton(F2, text=text,
                        variable=dest_mode, value=mode,command=SMode_changed)
        C.grid(row=i,column=0,sticky='W')
        i+=1

#*********************************************************************************
#here comes the muifli
m_preview=tkinter.StringVar()
m_preview.set('off')

m_dither=tkinter.IntVar()
m_dither.set(0)
m_row=0

m_W3=tkinter.Label(evil,text='Input File:')
m_W3.grid(row=m_row,column=0,sticky='E')

m_W5=tkinter.Entry(evil)
m_W5.grid(row=m_row,column=1,columnspan=5,sticky='WE')

m_W7=tkinter.Button(evil,text='Select...',command=m_Select_Open)
m_W7.grid(row=m_row,column=6,sticky='WE',padx=5)
m_row+=1

m_T1=tkinter.Label(evil,text='Input filetypes can be any PC gfx, IFLI (.gun/.fun/.fp2) or Drazlace (.drl)',font=("Helvetica",fontsize))
m_T1.grid(row=m_row,column=1,columnspan=6,sticky='W')
m_row+=1

m_W4=tkinter.Label(evil,text='Output File (.mui):')
m_W4.grid(row=m_row,column=0,sticky='E')

m_W6=tkinter.Entry(evil)
m_W6.grid(row=m_row,column=1,columnspan=5,sticky='WE')

m_W8=tkinter.Button(evil,text='Select...',command=m_Select_Save)
m_W8.grid(row=m_row,column=6,sticky='WE',padx=5)
m_row+=1

m_T2=tkinter.Label(evil,text='Note: Every image is also executable with SYS12288 / JMP $3000',font=("Helvetica",fontsize))
m_T2.grid(row=m_row,column=1,columnspan=5,sticky='W')
m_row+=1

m_W9=tkinter.Label(evil,text='Source Palette',font=("Helvetica",fontsize))
m_W9.grid(row=m_row,column=0,sticky='W')

m_Wc=tkinter.Label(evil,text='Frame-individual Colors',font=("Helvetica",fontsize))
m_Wc.grid(row=m_row,column=4,sticky='W')

m_Wc=tkinter.Label(evil,text='Destination Palette',font=("Helvetica",fontsize))
m_Wc.grid(row=m_row,column=5,sticky='W')
m_row+=1

m_F1=tkinter.Frame(evil,borderwidth=5,relief='ridge')
m_F1.grid(row=m_row,column=0,columnspan=4,sticky='NWES')

m_F2=tkinter.Frame(evil,borderwidth=5,relief='ridge')
m_F2.grid(row=m_row,column=4,columnspan=1,sticky='NWES')

m_F2b=tkinter.Frame(evil)
m_F2b.grid(row=m_row,column=5,columnspan=2,sticky='NWE')

m_extra_1=tkinter.Frame(m_F2b,borderwidth=5,relief='ridge')
m_extra_1.grid(row=0,column=0,columnspan=1,sticky='WE')

m_extra_2=tkinter.Frame(m_F2b)
m_extra_2.grid(row=1,column=0,columnspan=1,sticky='WE')

m_F3=tkinter.Frame(evil)
m_F3.grid(row=m_row,column=6,columnspan=2,sticky='NWE')

m_inks=tkinter.StringVar()
m_inks.set('off')

m_papers=tkinter.StringVar()
m_papers.set('off')

m_sprites=tkinter.StringVar()
m_sprites.set('off')

m_MN1=tkinter.Checkbutton(m_F2, text='Different Inks',variable=m_inks,onvalue='on',offvalue='off')
m_MN1.grid(row=1,column=0,sticky='W')

m_MN2=tkinter.Checkbutton(m_F2, text='Different Papers',variable=m_papers,onvalue='on',offvalue='off')
m_MN2.grid(row=2,column=0,sticky='W')

m_MN3=tkinter.Checkbutton(m_F2, text='Different Sprites',variable=m_sprites,onvalue='on',offvalue='off')
m_MN3.grid(row=3,column=0,sticky='W')

m_MN4=tkinter.Label(m_F2,text='Note: Selecting checkboxes here\nincreases rendertime and flicker,\n but reduces bugs!',font=("Helvetica",smallfontsize))
m_MN4.grid(row=4,column=0,sticky='W')

m_bruteforce=tkinter.StringVar()
m_bruteforce.set('on')

m_preview_mode='off'
m_result_mode='on'

m_deflicker=tkinter.StringVar()
m_deflicker.set('on')

m_M1=tkinter.Label(m_extra_2,text='FLI-bug color:',font=("Helvetica",fontsize))
m_M1.grid(row=0,column=0,sticky='W')

fbugcol= tkinter.StringVar()
fbugcol.set("Black")

m_ELA = tkinter.OptionMenu(m_extra_2, fbugcol, *fbugcolors)
m_ELA.config(width=6)
m_ELA.grid(row=0,column=1,sticky='WE')

m_C1=tkinter.Checkbutton(m_extra_2, text='Deflicker',variable=m_deflicker,onvalue='on',offvalue='off')
m_C1.grid(row=1,column=0,sticky='W')

m_C2=tkinter.Checkbutton(m_extra_2, text='Brute force Spritecolor',variable=m_bruteforce,onvalue='on',offvalue='off')
m_C2.grid(row=2,column=0,columnspan=2,sticky='W')

m_row+=1

m_F4=tkinter.Frame(evil,borderwidth=5,relief='ridge')
m_F4.grid(row=m_row,column=0,columnspan=4,sticky='NWE')

m_F5=tkinter.Frame(evil,borderwidth=5,relief='ridge')
m_F5.grid(row=m_row,column=4,columnspan=4,sticky='NW')

m_preview_mode = tkinter.StringVar()
m_preview_mode.set('off')

m_result_mode = tkinter.StringVar()
m_result_mode.set('on')

m_P1 = tkinter.Radiobutton(m_F4, text='Original',variable=m_preview_mode, value='off',command=m_Preview_Off)
m_P1.grid(row=0,column=1,sticky='E')
m_P2 = tkinter.Radiobutton(m_F4, text='Pre-Convert',variable=m_preview_mode, value='on',command=m_Preview_On)
m_P2.grid(row=0,column=2,sticky='W')

m_P3 = tkinter.Radiobutton(m_F5, text='Result',variable=m_result_mode, value='on',command=m_Resultview_On)
m_P3.grid(row=0,column=1,sticky='e')

m_P3b = tkinter.Radiobutton(m_F5, text='Flickermap',variable=m_result_mode, value='onf',command=m_Resultview_Off)
m_P3b.grid(row=0,column=2,sticky='')

m_P4 = tkinter.Radiobutton(m_F5, text='Errormap',variable=m_result_mode, value='off',command=m_Resultview_Off)
m_P4.grid(row=0,column=3,sticky='W')

m_source_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))
m_result_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))
m_preview_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))
m_error_photo=ImageTk.PhotoImage(Image.new("RGB", (320,200)))

m_LPreview = tkinter.Label(m_F4)
m_LPreview.config(image=m_source_photo)
m_LPreview.grid(column=0,columnspan=4,row=1)

m_RPreview = tkinter.Label(m_F5)
m_RPreview.config(image=m_result_photo,anchor=tkinter.CENTER)
m_RPreview.grid(column=0,columnspan=4,row=1)

m_row+=1

m_L0=tkinter.Label(evil,text='Input Interlace\nprocessing:',font=("Helvetica",fontsize),state='disabled')
m_L0.grid(row=m_row,sticky='W')

m_inputlace = tkinter.StringVar()
m_inputlace.set("rastered") # initial value

m_LA = tkinter.OptionMenu(evil, m_inputlace, "rastered", "blended",command=m_Do_Preview)
m_LA.config(width=6,state='disabled')
m_LA.grid(row=m_row,column=1,sticky='WE')

m_L1=tkinter.Button(evil,text='Save Pre-Convert',command=m_SavePreview)
m_L1.grid(row=m_row,column=2,columnspan=2,sticky='WE')

m_L2=tkinter.Button(evil,text='Convert',command=m_Convert)
m_L2.grid(row=m_row,column=6,sticky='e',padx=20)

m_SR=tkinter.Button(evil,text='Store Result-/Flicker- & Errormap-BMP',command=m_StoreAll)
m_SR.grid(row=m_row,column=4,columnspan=2,sticky='W')

m_MODES = [("Pepto", "pepto"),("DeeKay", "deekay"),("Prepare Truecolor", "p")]
m_source_mode = tkinter.StringVar()
m_source_mode.set("pepto")
i=0
text,mode = m_MODES[i]
m_B_1 = tkinter.Radiobutton(m_F1, text=text,variable=m_source_mode, value=mode,command=m_SMode_changed)
m_B_1.grid(row=i,column=0,sticky='W')
i+=1
text,mode = m_MODES[i]
m_B_2 = tkinter.Radiobutton(m_F1, text=text,variable=m_source_mode, value=mode,command=m_SMode_changed)
m_B_2.grid(row=i,column=0,sticky='W')
i+=1
text,mode = m_MODES[i]
m_B_3 = tkinter.Radiobutton(m_F1, text=text,variable=m_source_mode, value=mode,command=m_SMode_changed)
m_B_3.grid(row=i,column=0,sticky='W')

m_Wa=tkinter.Label(m_F1,text='Luminance Min',font=("Helvetica",smallfontsize),state='disabled')
m_Wa.grid(row=0,column=3,sticky='E')
m_lmin_scale=tkinter.IntVar()
m_lmin_scale.set(0)
m_S1 = tkinter.Scale(m_F1, from_=0, to=100,resolution=2,orient=tkinter.HORIZONTAL,font=("Helvetica",smallfontsize),state='disabled',variable=m_lmin_scale,command=m_Do_Preview)
m_S1.grid(row=0,column=4,sticky='E')

m_Wb=tkinter.Label(m_F1,text='Luminance Max',font=("Helvetica",smallfontsize),state='disabled')
m_Wb.grid(row=1,column=3,sticky='E')
m_lmax_scale=tkinter.IntVar()
m_lmax_scale.set(50)
m_S2 = tkinter.Scale(m_F1, from_=0, to=100,resolution=2,orient=tkinter.HORIZONTAL,font=("Helvetica",smallfontsize),state='disabled',variable=m_lmax_scale,command=m_Do_Preview)
m_S2.grid(row=1,column=4,sticky='E')

m_MODES2 = [("Pepto", "pepto"),("DeeKay", "deekay")]
m_dest_mode = tkinter.StringVar()
m_dest_mode.set("pepto")
i=0
for text, mode in m_MODES2:
        m_C = tkinter.Radiobutton(m_extra_1, text=text,
                        variable=m_dest_mode, value=mode,command=m_SMode_changed)
        m_C.grid(row=0,column=i,sticky='W')
        i+=1
#*****************************************************
def About():
  top = tkinter.Toplevel()
  top.resizable(tkinter.FALSE,tkinter.FALSE)
  top.title("About Mufflon")
  canv = tkinter.Canvas(top,width=395,height=250)
  canv.config(scrollregion=canv.bbox(tkinter.ALL))
  pic1 = tkinter.PhotoImage(data="""
R0lGODlhiwH6APcAAERERAEBAGpQMKqqqrKysldXVpycnG1ZRndjUKSkpIqKigGK1ZSUlNLcpDCv
9t0hAJrQ8Uy29HFxcQCY65WGcYVuV4KCgnt7e414ZTIpCK6ojzExMSFNY2lpaUM4CAJwsVlHNAB5
5WRkZACF6KRPHUk5JgGk8wEqWW/G+FqSrABq2KC7yxSJt+bqzwBx3vtlCqgiAlVCKyEgIABPr6mb
iKSRdvVOAQNNimW7796TavdwJrjIb0VBNwCd8gGi2cW7rFp6g664eABayQCn6k2o11xSSBap9gBk
zlNLQgB7xTBsjM9pM9jTzJqThglkl3dsYSmTxwA3jvT25h4YBgG49HtyZU8/F7vEjLu0pG2pzElC
J0VlcMpKA2tLHdXOxxAoNZ6ijZeepQ4RFJaouJaNgj0yImcOAEqbxbWtoqiut+yENQGw7+Pf2ZSU
m9h6RTIqHil8pjWn3k9aYOzs487MxJ6LLPv99ZyFU21xc6CeUdzdzKu0u3AwAf///wCK8F9PO3lg
M5ubk7a2tklJSo6OjiG1+qSmqmxjXImUnBm07KumpfPv7SQ4RK2rooODa25pYUgPAlVkVN3l6ail
Wp6mdEBVW+vn4356cq5tQx6l3zS9/b3P1tzX0r25tImGgI6Lf87Tznx1cJyWkzU/IYhYPMO+wXV1
dYaGhsNaJOft7ZaalNfX3G1tbYaKjXV5fufn55aandfb1U9PT8XHy2NkaiYqLQ+a4V5dXR0aGfv7
+/f39+/v78/Pz/Pz87q6uuPj49/f3+vr69fX19vb28vLy76+vtPT08PDw8fHx///++/v6vv398/L
y+fn49/f28jHwffz8vf58c/Py9vf39/b2efj4Le8xNfT066urtvb39vf2+Pj54B9gePn48C6usG+
udPX17a7s8HHxcjDvaKqssPRg/P38evr75OOj+/r6IaKgoqGinF5c351eZ2AZ/f3+8vLz8/Lz2ll
Z8/T1yQ8KC0sLNPT19fb39XX0eru3MfLx22gvcfLy9DHvGs+KDhRSSH5BAAAAAAALAAAAACLAfoA
AAj/APsIHEiwoMGDCBMqXMiwocOHECNKnEixosWLGDNq3Mixo8ePID3mGkmyJMmQKFOqXMmypcuX
MAuO1KWrV69dOHH2onkyps+fQIMKHUp04ExdOIO9Wsr0lU5dI4tKnUq1qtWpM2+++gVsmLCvwoYB
++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
""")

  canv.create_image(0,0, image = pic1, anchor='nw')
  canv.image=pic1
  canv.pack(expand = tkinter.YES, fill = tkinter.BOTH)
  #canvas.pack()
  button= tkinter.Button(top, text="Close", command=top.destroy)
  button.pack(side='right')

def Help():
  whole_readme="""
  
  Readme for Mufflon v1.0

Credits:
Bitbreaker/VOZ: Mufflon CLI code
Crossbow/Crest: NUFLI and MUIFLI format definitions and c64-editors code
DeeKay/Crest: Idea, Betatesting, converter strategies, Mufflon GUI design,
Samplepictures, this Readme
Mr.SID/HVSC: MacOS X GUI code
enthusi/Onslaught: Crossplatform Python-GUI-code
A Life in Hell: Windows Standalone EXE, CLI-debugging

Version History:
v1.0:
Known bugs:
-none

Table of contents:

0.Compiling

1. How to make/prepare your pictures in c64 colors
 1.1 NUFLI
  1.1.1 Preparing IFLI/Drazlace
  1.1.2 FLIbug
  1.1.3 Rasterbars
 1.2 MUIFLI

2. Using the Converter
 2.1 General Stuff
 2.2 Converter GUI options
  2.2.1 NUIFLI
  2.2.2 MUIFLI
  2.2.3 Global

3. Postprocessing pictures and working in NUFLI/MUIFLI
 3.1 General stuff
 3.2 NUFLI
 3.3 MUIFLI
 3.4 Using the editors
  3.4.1 NUFLI-Editor
  3.4.2 MUIFLI-Editor


Preface
Hello and welcome to Mufflon, the High Quality Commodore 64 Imageconverter. If
you don't know what a Commodore 64 is, this is definately not for you. This
readme was written with Commodore 64 users in mind, so there will not be any
explanations as to what "Sprites", "Bitmap", "FLI" or "Rasterbars" are.

This converter was primarily designed for rendering pictures made in c64 colors.
You can feed it with normal truecolor images and select the auto-prepare-mode,
but this will look unsatisfactory in most cases. Auto-Prepare looks at the color
of the pixel and then chooses from seven c64-colorgradients (blue, red, grey,
green etc), it also generates a 4-step dithering.


0. Compiling

You may compile the CLI-sourcecode yourself if binaries for your platform are
not available. The c-source is platform agnostic and endian-safe and only
requires a POSIX-compliant CLI and gcc, although other compilers should work,
too. Simply do a "make" with an installed GCC-toolchain. The Mac GUI (which
includes the CLI-converter) is Mac-only and requires Xcode 3.2 with the MacOS X
10.4 SDK installed for compatibility.

1. How to make/prepare your pictures in c64 colors

You simply use the PC/Mac/Linux or even c64-pixelprogram of your choice
(Photoshop, Pixen, Promotion, Timanthes, Funpaint, Drazlace, Gunpaint, flick0r,
Project One etc) to make your picture, using the exact colors in the
gradient-GIFs when converting from PC-images. This is important, don't come
crying to me because your picture looks all wrong because you used a palette the
converter does not support! ;-)
Here are the Pepto and DeeKay Palettes in RGB format:

DeeKay:
0: 00/00/00 black
1: ff/ff/ff white
2: 85/1f/02 red
3: 65/cd/a8 cyan
4: a7/3b/9f purple
5: 4d/ab/19 green
6: 1a/0c/92 blue
7: eb/e3/53 yellow
8: a9/4b/02 orange
9: 44/1e/00 brown
10:d2/80/74 pink
11:46/46/46 dark grey
12:8b/8b/8b med. grey
13:8e/f6/8e lt. green
14:4d/91/d1 lt. blue
15:ba/ba/ba lt. grey

Pepto:
0: 00/00/00 black
1: FF/FF/FF white
2: 68/37/2B red
3: 70/A4/B2 cyan
4: 6F/3D/86 purple
5: 58/8D/43 green
6: 35/28/79 blue
7: B8/C7/6F yellow
8: 6F/4F/25 orange
9: 43/39/00 brown
10:9A/67/59 pink
11:44/44/44 dark grey
12:6C/6C/6C med. grey
13:9A/D2/84 lt. green
14:6C/5E/B5 lt. blue
15:95/95/95 lt. grey

I can't be assed to write down all the 79 interlaced colors now, just sample
them from the interlace gradient GIF or calculate them yourself by averaging the
two colors by hand! ;-)


1.1 NUFLI

1.1.1 Preparing IFLI/Drazlace

If you want to convert your Drazlace/IFLI images for NUFLI as BMP, make sure
they are in rastered form (16 colors only, square pixels like you see in the
zoom mode!), not blended. You can generate a rastered BMP either with tools such
as Comievju (Win) or View64 (Win/Linux) or you just make it yourself in your
gfx-program of choice (f.ex. if you don't have the original .drl or .fun file
anymore!): Make screengrabs of both pictures in the emulator, layer them on top
of another (with the same offset) and then generate a mask on the top half-image
layer. That mask needs to be filled with vertical 1-pixel-wide black and white
bars. Voila, rastered IFLI/Drazlace! There's a 50:50 chance you get the black
and white bars wrong, if it looks wrong, simply invert the mask. I made myself a
Photoshop-Action to ease the process, but it requires a 2x1 black/white
stripe-pattern i have predefined and I have no idea how to extract that from
Photoshop. The converter also supports direct import  of IFLIs in Gunpaint or
Funpaint 2 Format or Drazlace, and if you want to work on the picture before
conversion you can use the Save Pre-Convert button. See "Using the converter"
(2.) for more information.

Remove any of the tricks used to make IFLI/Drazlace less flickery, such as using
inbetween colors along hard vertical edges (e.g. black -> grey -> white), since
that is not necessary in static NUFLI and only adds unneccesary extra colors
horizontally, which you should always avoid. Remember: there is only a maximum
of 3 colors per char in NUFLI (Drazlace has four, max. in IFLI would be six, but
luckily that many colors are hardly ever neccesary!), with one color (sprites)
being fixed for 6 chars! So don't go overboard with the colors horizontally,
especially in NUFLI. Vertically, many colors aren't much of a problem, since
there is FLI every second line. Also make sure to align your picture to the
original cursor boundaries horizontally (vertically it does not matter thanks to
FLI!), since just a single pixel offset will generate a shitload of bugs in
colorful pictures, especially in Drazlace! Finally: Check that 320th
pixelcolumn. Depending on the editor the picture was made  in, it is often
black, and especially in the 40th colum there are no sprites, so just copy that
319th colum over and fix it a bit!

1.1.2 Preparing your own pictures

How you prepare your pictures is up to you, I've described how I do it for
MUIFLI in Photoshop in section 1.2, the only difference in static NUFLI would be
to use 8-color gradients instead of 15-color ones.

1.1.2 FLIbug

Remember NUFLI offers GFX in the FLIbug, so why not use this chance to extend
your IFLI-picture into those first three characters? ;-) In most cases, the
FLIbug is even more colorful than the rest of the picture, since there are 5
colors (4 plus light grey, 6 in the AFLI-lines) the converter can use (instead
of 3), and even two of them are hires pixels (instead of one in the picture
itself!).

1.1.3 Rasterbars

It didn't take me long to convince Crossbow of this feature, so since we now
cover the whole width of the screen, there is even the possibility of rasterbars
for the border in NUFLI! Check out Duce's Iskender and Peacemaker, the
end-NUIFLI-Logos in Crest Slide Story or Carrion's NUFLI pic in "Carrion's
Oldskool Pixels 100%" to see this in use. However, there are a few things that
need to be taken into consideration if you want to use this feature:

1) The converter does not render this. You need to set $d020 switches by hand in
the NUFLI editor.

2) color gets changed INSIDE the screen, so the left and right are offset one
pixel! Simply use something in the middle that covers the rasterbars to hide the
offset!

3) you can only change color every 2 lines.

4) you can only change color in odd lines (starting on the right side, remember,
so on the left side it shows first in even lines!), starting from line 3 (in
line 1 the register switches are taken for the initial FLIbug colors!), 5, 7, 9
up to line 199.

Section 3.4.1 has info on how to do $d020 Rasterbars in the editor.


1.2 MUIFLI

You can use whatever you like, I describe how I do it in Photoshop, but even if
you use something else you should read this since many of the basic principles
are program-independant. First, you need to keep in mind two things when
downscaling and cropping some picture into 296x200 (320x200 if you want to use
the limited MUIFLI FLIbug gfx!):
1) C64 hires pixels are NOT square! To verify this, simply draw a 25x25 square
with inverted spaces on your c64 and measure yourself! The pixel aspect ratio is
about 1:1.2 - ofcourse this also depends a lot on how the V/Hsize dials on the
back of your monitor are set. But 1:1.2 should look okay on the average setup.
So make sure to scale your picture not linear, but to squeeze it a bit
vertically!
2) Avoid sharpening the picture or using unsharp masks! Believe me: you do not
want this! Remember that bicubic interpolation automatically does sharpening, so
use bilinear interpolation instead!

Make sure you use the full brightness spectrum all the way from black to white
well (auto-levels/auto-contrast). With most pictures, it helps to do a little
level adjust before further processing, to get some more contrast without losing
any details in the white/black areas, like you would if you simply used the
contrast slider. To achieve this, it is a good idea to use a level adjust curve
that looks like a stretched-out s, this amplifies the bright and dark areas a
bit and makes sure your picture does not look like a bland grey mess with some
bright and dark spots on the real machine.
Remember that the c64 luminance gradient is not linear!

I prepared the MUIFLI example pictures by converting into 15 greys (In
Photoshop, do a convert to Greyscale and then back to RGB before converting to
indexed colors, simply desaturating the picture does NOT do greyscale with the
proper RGB weighting!) with either pattern or floyd steinberg dithering (floyd
works better in most cases, in Hires it looks quite okay on c64!) and then
assigning a fitting gradient by sampling colors from the interlace gradient GIF.
I always use 8-color-gradients, using only one color per gradient from the
green/pink/med. grey/lt. blue cluster, since gradients look smoother that way. 8
solid colors plus 7 inbetween colors = 15 luminance levels. You may opt for
non-linear gradients, wildly choosing from the palette whatever suits you, but
you need to remember that gradients that go mix-color -> solid color -> mix
color -> solid color generate WAY less bugs than gradients where mixed colors
follow one another. If you do a gradient that uses primarily  mixed colors, you
most definately must enable the frame-individual colorswitches when converting -
which means you will get more flicker.
When I have the picture in a single 15-color c64-gradient, I simply copy it over
to a new picture and add a transparency mask to it. This process is repeated for
however many color gradients I want for the picture. Then I just draw in the
transparency masks with the pencil tool (this is important, the mask needs to be
black/white only, no greyscale antialiasing such as with the brush tool, which
would add inbetween colors the converter does not understand!).
The bottom layer should be the most prominent gradient and does not need a mask,
that way you make sure every pixel is set in the resulting picture - It may not
be colored properly, but it's set! ;-)
I've enclosed a PSD so you can check out what this looks like!
Sometimes, depending on the picture, it helps to convert it into more grey
levels than you need in the end, and then assigning several levels the same c64
target color. For example if some picture needs more shades in darker or lighter
areas this usually helps.

Always remember: This is not $d016 interlace. It is totally up to you where and
how you use interlace (in the form of interlaced colors!). If you just want
smooth fades for your blurry background, you can have that, while the foreground
is static in 16 colors!

FLIbug gfx: As mentioned, there are three colors possible in the 24x200 area on
the left, the first three characters: Grey, a Spritecolor of your choice and an
interlaced mix-color of these two. You will not be able to work on this manually
in the editor later on, so make sure everything is fine there before you convert
your picture! Don't worry,  there won't be any conversion bugs in that area if
you just stick to these three possible colors.


2. Using the Converter


2.1 General stuff

Input file restrictions:
Thanks to Python's Imaging Libary, a lot of PC gfx can be loaded (BMP, PNG, GIF,
JPG), if they are not 320x200 pixels, they will be re-scaled (using
nearest-neighbor scaling) into this size, no matter what the aspect ratio is.

IFLI: Funpaint 2 (packed and unpacked) or Gunpaint file format, .fun/.fp2/gun
file extension
Drazlace: Drazlace file format, .drl file extension, 320x200
resolution-interlace only, no 160x200 color interlace! The $d021 byte is read
and interpreted, however $d021 rasterbars, such as Funpaint 2 offers them, are
not supported (since they've never been used to my knowledge)!

PC-Pictures in c64 colors must be 320x200 and use either the Pepto or Deekay
color palette for optimal results. For mixed colors, you may only use the 79
colors in the Palette-GIFs, the converter replaces any other unknown color with
one of these.

A word about the generated files: Every unpacked file (MUF, MUI, NUF - even NUI
for NUIFLI when that editor is released!) has its displayer and speedcode
generator built in, so you can always start them directly with SYS 12288 (JMP
$3000) If you want to load it into our editors on c64, make sure there are
exactly 12 characters before the .nuf/.mui file extension!

2.2 Converter GUI options

2.2.1 NUFLI (radiobutton): Use this if you don't want interlace, best choice for
converting old 16-colored IFLIs and Drazlace pictures! Full 320x200, FLI-bug has
colorful gfx, and there's the option of $d020 rasterbars!

Input File (text field): Restrictions see above. Use the "Select"-Button or type
in your Path and filename.

Output File (text field): This is generated automatically upon opening an input
file and already has the right file extension, but can be changed by hand or
through the "Select" button if desired. Respect the 12-character limit mentioned
above if you want to load the generated .nuf into our editors!

Source Palette (radio button): This is the palette your source image is in. Use
one of the following palettes: Pepto: Best palette choice. Measured on the real
thing with a Vectorscope: Deekay: Looked great when I made it back in '97.
Somehow looks way oversaturated now; Prepare truecolor: Renders
truecolor-picture in dithered c64-colorgradients, looks at the color of the
pixel and then chooses from seven c64-colorgradients (blue, red, grey, green
etc) and then does  a conversion with 16 static colors and 4-step dithering. Do
not use this if your picture is in c64-colors!
Luminance Min (slider): Adjust contrast for prepared pic. Higher values = more
black
Luminance Max (slider): Adjust contrast for prepared pic. Higher values = less
white

Destination Palette (radio button): This determines the colors the converter
works with internally and also the colors of all generated BMPs (including the
one in Pre-Convert). Use one of the following palettes: Pepto, Deekay.
Color-replacement works different with different palettes, so in some pictures
using my (DeeKay's) palette as Destination palette does deliver better results.

Render FLI-bug (checkbox): Renders the first 3 characters (FLI-bug) as well. May
be omitted if nothing is there, but then you'll need to clear the FLIbug
manually in the editor (don't worry, there's a command for it! ;-).

FLI-bug Multiple Passes (checkbox): Enables bruteforcing of all permutations for
spritecolor combinations in the FLIbug. Was 11x slower in our tests, but only
gave a 0.5% better result. Only recommended for that final convert of your
compopicture! ;-)

Store Result- & Errormap-BMPs (button): A click on this saves the conversion
result and errormap BMPs (displayed in the bottom right image well) into the
selected target folder, with the target name as base-filename. This is useful
for checking and comparing to the original to see where it still needs manual
fixing.

Original (radiobutton, image well): Your original picture. Stays empty if IFLIs
or Drazlace pictures are loaded (check it in the Pre-Convert view!)

Pre-Convert (radiobutton, image well): The picture just before conversion.
Truecolor Prepare (if selected) and destination palette are already applied
here.

Result (radiobutton, image well): This is what the c64 conversion looks like. 

Errormap (radiobutton, image well): This shows you the erroneous areas in your
picture. The whiter the pixel, the bigger the conversion error (difference to
the original) is. Should be as black as possible for optimal results!


2.2.2 MUIFLI (radiobutton): If you want details and ultra-smooth fades through
interlace, choose this. Supports 79 colors in total. Limited FLI-bug gfx only
(lt. grey and one more color, plus the mix color of these two, no matter how
much it flickers), which are NOT editable in the editor.

Input File (text field): Restrictions see above. Use the "Select"-Button or type
in your Path and filename.

Output File (text field): This is generated automatically upon opening an input
file and already has the right file extension, but can be changed by hand or
through the "Select" button if desired. Respect the 12-character limit mentioned
above if you want to load the generated .mui into our editors!

Source Palette (radio button): This is the palette your source image is in. Use
one of the following palettes: Pepto: Best palette choice. Measured on the real
thing with a Vectorscope: Deekay: Looked great when I made it back in '97.
Somehow looks way oversaturated now; Prepare truecolor: Renders
truecolor-picture in dithered c64-colorgradients, looks at the color of the
pixel and then chooses from seven c64-colorgradients (blue, red, grey, green
etc) and then does  a conversion with 79 interlaced colors and 4-step dithering.
Do not use this if your picture is in c64-colors!
Luminance Min (slider): Adjust contrast for prepared pic. Higher values = more
black
Luminance Max (slider): Adjust contrast for prepared pic. Higher values = less
white

Store Result-/Flicker-/Errormap-BMPs (button): A clickon this saves the
conversion result, flickermap and errormap BMPs (displayed in the bottom right
image well) into the selected target folder, with the target name as
base-filename. This is useful for checking and comparing to the original to see
where it still needs manual fixing.

Frame-individual colors: This determines what colors should be different in both
frames. The more checkboxes are selected, the longer rendering takes and the
more it flickers - but on the other hand conversion bugs are reduced. Also makes
it increasingly harder to do manual work on the picture afterwards, if you have
six different colors over both frames, it is quite hard to set the pixels right!

Different inks (Checkbox): Allows different ink-colors in both frames. Ink =
most important color. First choice to reduce bugs without causing too much
flicker. recommended if picture has many bugs!

Different Papers (Checkbox): Allows different paper-colors in both frames. If
bugs are still bad, check this as well. Adds even more flicker.

Different Sprites (Checkbox): Allows different sprite-colors in both frames.
Last resort. Rather heavy flickering, since deflickering hardly ever works
anymore.

Destination Palette (radio button): This determines the colors the converter
works with internally and also the colors of all generated BMPs (including the
one in Pre-Convert). Use one of the following palettes: Pepto, Deekay.
Color-replacement works different with different palettes, so in some pictures
using my (DeeKay's) palette as Destination palette does deliver better results.

FLI-bug color (pulldown): color of sprites covering the fli bug. The FLIbug in
MUIFLI only has 3 colors: This color, light grey and an interlace-mixed color of
both.

Deflicker (checkbox): Use DeeKay's deflickering method of distributing pixels
evenly across both frames. Should always be used! We are using double-wide
pixels for mixes with black, so the non-black pixels don't get "eaten" away.

Brute force spritecolor detection (checkbox): Search for the best sprite-colors
by brute force. recommended. Improves quality a bit, but makes rendering a
little slower.

Original (radiobutton, image well): Your original picture. Stays empty if IFLIs
or Drazlace pictures are loaded (check it in the Pre-Convert view!)

Pre-Convert (radiobutton, image well): The picture just before conversion.
Truecolor Prepare (if selected) and destination palette are already applied
here.

Result (radiobutton, image well): This is what the c64 conversion looks like. 

Flickermap (radiobutton, image well): This shows you how strong the flicker is.
The whiter the pixel, the more it flickers. Should be as black as possible for
optimal results!

Errormap (radiobutton, image well): This shows you the erroneous areas in your
picture. The whiter the pixel, the bigger the conversion error (difference to
the original) is. Should be as black as possible for optimal results!

2.2.3 Global: 

Input Interlace processing (pulldown): render IFLIs/Drazlaces either rastered
(16 solid colors, like in the zoom mode) or blended (like the actual IFLI looks
on c64, colorblending still through interlaced mix-colors, but with much less
flickering than before, since we use hires and better pixel distribution). Since
it uses interlace colors, "Blended" is not very useful for static NUFLI, only
use this with MUIFLI.
There's one thing about the "blended" mode: Like with PC-input-images using
mixed colors, the converter only supports those 79 colors (apart from the one
FLIbug mix color, which is lt. grey plus whatever FLIbug color is chosen!). So
even if theoretically an IFLI/Drazlace may have all 136 mixed colors that are
possible, the conversion result is always only in these 79 colors. In reality,
e.g. mixing black with white pretty much never happens though, since it simply
flickers too much. And even if it did, the converter would simply replace it
with regular medium gray, so you couldn't really tell a difference. When
converting to MUIFLI, colors are replaced anyway, it's just part of the
conversion process, but if you want 100% accurate PC-Bitmaps you should keep
this in mind.

Save Pre-Convert (button): This saves the picture in the "Pre-Convert" image
well as BMP. Use this together with the  source/destination palette switches for
easy palette remapping, which is very timesaving especially if your Image uses
mixed colors!  You can also use this to generate BMP-versions of your IFLIs and
Drazlace (or prepped truecolor) pictures for further editing/extending in PC
pixelprograms, or for general BMP conversion (e.g. screenshots for CSDB! ;-).
Note the 79-color-limitation mentioned above when using "blended" as Interlace
processing mode!

Convert (button): The big "Go!" button that processes and saves the conversion
results. GUI is unresponsive while it converts, which in the case of MUIFLI and
a lot of frame-individual colors can take quite some time.


3. Postprocessing pictures and working in NUFLI/MUIFLI

3.1 General stuff

Fear the dreaded "Black Bleed":
This is a problem you pretty much only encounter in Hires, so it's not an issue
in IFLI or Drazlace, since these formats are still multicolor. Black tends to
"eat" away other Hires pixels that it's rastered with. Emulators can't simulate
this effect right now (only Bero/fR's Micro64!). This looks WAY darker than in
your IFLI Drazlace, where the mixture of the colors is achieved through
interlace of solid lines. In MUIFLI, we try to account for this in the editor by
using a special interlace distribution pattern of double-wide pixels for
black-mixed colors, but it's not as good as if you do it manually. No chance to
do anything about this in static NUFLI, other than dithering black differently,
wider on one (the non-black) side than other colors. The "Black Bleed" effect
varies quite a bit over different VIC-, c64- and c128-revisions and also depends
on the monitor and how the dials are set, it's a bit like with the SID chip.
However, Crossbow's c128 and monitor are the only k nown setup that does not
show any black bleed at all, I've tested countless configurations and did not
find a single one that does not show any bleeding. If you want to check the
Black Bleed on your system, check CSDB for Crest's "emusuxX0r" test program.
In short: You should always check the areas where you dither with black on the
real machine after conversion - especially if the original is IFLI or Drazlace,
which looks perfectly fine with black dithering on the real machine, since it is
NOT real Hires!

If possible, fix your stuff on the real machine: Black bleed, dot-creep: these
are all effects you simply do not get in emulators. I've done most of the Crest
Slide Story fixing in VICE with scanlines and PAL-emulation activated, only to
find out very often it looks quite different on the real machine. Colors are
still somewhat different, and often PAL blending is less noticeable on the real
machine than in the emulator. Okay, the main reason to squeeze these editors
into the c64 memory was Crossbow's reluctance to make crossdev-tools, but why
not use this as an advantage? ;-) I also work much faster on the real machine
than on my iBook, where i constantly have to toggle joystick emulation on/off
and where pressing F-Keys is quite tedious.
With IFLI/Drazlace and Multicolor the difference isn't that big between real
hardware and an emulator. But with Hires and especially Hires Interlace it's a
completely different world.


3.2 NUFLI

Here's a few tricks to try out if you're starved of colors:
1) Replace colors of similar brightness. It's quite pointless to have e.g. cyan
and lt grey in the same block when you would desperately need some other colors
- Though for some reason that escapes me the converter still does this sometimes
when it chooses its colors!
2) Use dither insted of inbetween colors, e.g. you have dark and lt. grey, you
would need medium grey -> dither dark and lt grey!
3) Use inbetween colors instead of dithering. The reverse of 2), only useful in
cases where in this example the whole block would be filled with dithered
dark/lt grey -> use medium grey instead! If used wisely, this can enable you to
"free up" one color that you might need for something else. This trick is very
important vertically to get rid of those hard edges, since you can change color
every 2 lines! Got blue forming an ugly horizontal line with light blue? Just
use purple inbetween! You have half-FLI - use it!
4) Remove unneeded (mostly background) detail and focus on important stuff. No
one's gonna miss that crack that didn't really look like one in the first place!
;-)
5) Move stuff. Quite often, it already helps loads to just move stuff one pixel
to the left or right, to better match those cursor boundaries. No copy feature
in the NUFLI editor, sorry! <:-)
6) Avoid unneeded spritecolor changes. In some rare cases, it can be a boon to
have 2 different spritecolors in one block, but quite often in order to remove
unwanted blocks or lines, it also means you have to set one of the spritecolors
as bitmap color as well (or even both, I've had that, yes!) 
7) Use color brightness to smooth edges. Quite often it already helps to replace
a color with one that's just a wee bit darker or lighter to smooth some blocky
edge or something!
8) Check where sprites are really used and change the spritecolor if necessary.
It is always a hassle to change the spritecolor, since you have to fix every
block it's used in. However, quite often, Sprite color is only really used in
one or two bitmap-blocks, so it's fairly easy to change. The converter is quite
dumb after all, so quite often you can find a better suitable spritecolor
yourself!

Remember to make good use of that Nibbleswap(Star)-Key if the wide spritepixels
are aligned wrong!


3.3 MUIFLI

All the tricks for NUFLI also work in MUIFLI, but thanks to interlace, there's
some extra things to watch out for and more strategies to try.
The converter only supports 79 colors. We chose those 79 colors of the maximum
possible 136 colors to keep flickering to a minimum (check the GIF with the
interlaced colors! On the middle on each edge would be the most flickering
colors, the nearer it is to the "X" in the picture, the less it flickers!). This
means the converter simply never uses ANY color that is more than 2 brightness
levels apart. However, if you fix your picture manually, you can -and should-
use whatever interlaced color you like in smaller areas, such as antialiased
edges. If it's merely a single pixel, even black can be interlaced with white
without causing too much pain on the eyes.

Keep in mind that you can have different colors in both pictures. That way you
not only have smoother fades (thanks to interlace), but also denser fades than
in static NUFLI, because you can have three or even four colors in a single char
(without sprites!). An Example: Your fade goes from green to cyan to lt. green,
so you have green and cyan in one picture and cyan and lt. green in the other,
all in a single char and excluding the spritecolor. This adds a bit more
flicker, since green and lt. green are only in one frame each, so you can't do
deflickering by even distribution across both frames, but as long as you don't
do it in larger areas you should be okay. Having different colors in both frames
also helps a lot with antialiasing, which is always quite hard to achieve in
static NUFLI. Check out the NUIFLI Logo in Crest Slide Story to see what level
of antialiasing you can achieve thanks to interlace. 

Speaking of flicker: If possible, try to distribute the interlace evenly across
both frames. Worst flicker of all is a solid color each in every frame, least
flicker is alternating checkerboards (or vertical (hires) columns) in both
pictures. When checkerboard mixing a solid color with an interlaced one, try to
go for line-wise switching. This is what the "deflicker" switch tries to do, but
you should also keep it in mind when postprocessing your picture!

There's a very cheap and simple way to use unconventional colors in (M)UIFLI,
and you should rather do this by hand (rather quickly) in the editor than before
in the PC-Image: Since there is zero difference in luminance (which is what
causes flicker!) you can simply interlace the color-pairs with solid colors in
each frame, without hardly any noticeable flicker and requiring zero bugfixing. 
So you can have some sort of very dark grey instead of brown or blue, or some
dirty grey instead of pink or green. I found f.ex. that for naked skin it works
much better to use a mixture of dark grey and red instead of either color, since
dark grey is just too grey and red looks like a severe sunburn. It also works
quite well for skin to interlace med. grey with pink, since medium grey is a
little bit darker, and there is quite a big luma-difference between orange and
pink, so the interlace with medium grey alleviates that effect a bit. Also, a
mixture of lt. green and yellow works better for skin colors than the respective
solid colors, since it looks less saturated. Check out the "tittenmarie" and
"klumwater" pictures on samplepictures.d64, in which I've done just that!

Priorities: As with UIFLI before, a very neat trick to combat the double-wide
spritepixels is having different priorities in both pictures (use the Star-key
to swap!). So f.ex. you have brown as ink and pink as paper in one picture and
the other way around in the other picture. This allows you VERY often to smooth
out those wide sprite-pixels just nicely, to achieve that perfect edge etc!

Multicolor: Even though the converter does not support/generate multicolor, that
does not mean you can't when postprocessing your picture. So in some cases
hitting that return key just might solve your problems. Be aware though that
multicolor is not supported in NUIFLI, so once that editor is released and you
import your MUIFLI pictures, all parts where multicolor sprites are used WILL be
buggy!


3.4 Using the editors

Both editors have a help-page built in, however with the MUIFLI editor this page
is only displayed once in the beginning, so memorize it well (or make a
screenshot in an Emulator! ;-)

3.4.1 NUFLI-Editor

This has the same keymapping as the MUFLI editor, in which we structured it a
bit more logically than in the U(I)FLI editors before. Memorize the F-Keys,
they're essential. Most of the time, while fixing bugs, you'll be working with
F3 (toggle bitmap, no coloring), F2 (color ink only), F4 (color paper only) and
F7 (toggle Spritepixel). NUFLI also has the MUFLI editor's multiple Undo/Redo,
which is quite convenient if you changed a block by accident and don't remember
the original colors (wrong F-Key!) or if you want to try something out. Remember
to use the Star nibbleswap and be not afraid to change the sprite color if it's
not used too much (see the hints in 3.2). The whole editor is actually two
editors: The picture editor and the FLIbug editor, which is quite different. The
picture editor works exactly like the MUFLI editor, so I won't cover that in
detail and focus on the FLIbug-editor instead.
The first time you see the FLIbug editor after hitting Return you probably just
think "what the fuck?". Don't worry, that is a perfectly normal reaction! ;-)
Fear not, you'll get the hang of it eventually, I'll try to explain it a bit:

First, a few words about the structure of the GFX in the FLIbug: As you might
know, both colors are usually light grey there. But thanks to Crossbow's awesome
timing skills we managed to squeeze out 2 lines in every char that are regular
AFLI with freely selectable ink and paper colors (those are the lines with the
white vertical divider and the Fs!). Yes, it's a bit weird, but you don't HAVE
to use this, feel free to also use light grey there for consistency! ;-) When
fixing Drazlace pictures for Crest Slide Story I was quite often rather happy to
have these 2 lines of AFLI though. Just draw in them and color them like you
would in the regular editor. The help-pages forget to mention that you can also
set ink color with simply A-P instead of Commodore-A-P when you're in
bitmap-edit-mode.

Then there's the spritelayers, which are below the set pixels (that way you can
also use the Hires bitmap for colors, that means lt. grey mostly!) There's one
Multicolor sprite at the bottom (colors 1-3, Keys 1-3) and a hires sprite over
it (color 4, Key 4). All sprite colors can be changed, but here's the thing:
it's all progressive, starting on the top and going down, so every color change
means this color is active until the next colorchange for that color (if there
is any). Hence the four color-columns right of the middle, so you can see what
spritecolors are usable in that very line. 

Now what's that stuff on the right? That's the Spritecolor-Changes for both the
picture and the FLIbug. We tried to get the maximum out of the FLIbug without
sacrificing anything inside the picture. So what we do is we re-use those unused
color-switches inside the picture, the -quite usual - cases where a spritecolor
remains unchanged, to switch spritecolors in the FLIbug. Having checked out many
many pictures I can assure you that changing all six spritecolors in the same
line pretty much never happens, usually you have anything from 2 to 4 switches
at your disposal - which is more than enough! The six colums on the right
represent the six sprites in the picture, with one (the one on the left) being
offset one line due to timing issues. A "-" means you can set your own
colorchange there, which you do by navigating there with the cursorkeys,
selecting the color you want to change (0-4) and then pressing the color's key
(A-P). You will see the "-" changes to the number of the spritecolor you had
chosen when pressing the color key, you will also see the color change in the
respective color-column. An "x" in means that this color-switch is already
occupied by a colorchange inside the picture, a  "*" denotes a
spritepointer-change, which we need to do eight times in the lower half of the
picture. As a bonus, we added the option of making $d020 Rasterbars in the
border by pressing key 0, for the limitations see 1.1.3. There is no
color-column for this, only a "0" in the area on the right denotes a
$d020-colorswitch. You can clear any FLIbug-colorswitch (0-4) by pressing the
"-" key with the cursor on it, "x" and "*" cannot be deleted.
It is quite tedious to draw something by hand in that, that's why we made the
converter for you! ;-) But if you understand how it works, it is certainly
possible to fix/improve things within a reasonable time frame in it. You will
notice the FLIbug can even be more colorful than the rest of the picture,
because you can choose from five or even six (AFLI lines) colors instead of
merely three.
With key 5 you clear the Multicolor spritepixels. You will only need this in the
two AFLI lines, because in the lt. grey area there's only light grey paper below
the sprites, which you already have as (hires!) ink.
Undo is "U" here, not +/-  like in the regular editor, and there's only a single
undo step.

3.4.2 MUIFLI-Editor

There is very little difference to the NUFLI editor (even less to the MUFLI
editor). In MUIFLI, there's still that Multicolor option, although the converter
does not use that (use Return). Each half-picture is edited separately, like in
the old UIFLI editor. Sorry. We just couldn't think of a better/easier way with
all the layers etc, and memory is kinda limited, too. Switch pictures with
Ins/Del, preview the interlace in zoom mode with '+' (takes a second or so to
start since we need to copy stuff around!). Apart from that, the only difference
to NUFLI is that there is no undo and no FLIbug gfx. You can't even edit the
stuff the converter rendered there, so make sure it's fine before conversion!

Closing words

Hope you all have fun making gfx in our weird modes! Remember: The converter is
good, but only if you master the editors and learn how to get rid of all the
bugs and use NUFLI/MUIFLI to its full extent, you get that extra edge of
perfection! ;-) Especially mastering MUIFLI is quite a task, but in return you
get graphical quality that is absolutely stunning and opens up completely new
levels in c64 graphics. There's vast areas of new territory to be explored, so
leave the old flicker-hell of Drazlace and IFLI to die, it's been done to death
anyway! ;-)

DeeKay/Crest on Aug 4th, 2010 - Thumbs up!
"""
  help=tkinter.Toplevel()
  s = tkinter.Scrollbar(help)
  T = tkinter.Text(help,width=80)
  T.focus_set()
  #s.pack(side=Tkinter.RIGHT, fill=Tkinter.Y)
  #T.pack(side=Tkinter.LEFT, fill=Tkinter.Y)
  s.grid(row=0,column=1,sticky='nes')
  T.grid(row=0,column=0,sticky='we')
  s.config(command=T.yview)
  T.config(yscrollcommand=s.set)
  T.insert(tkinter.END, whole_readme)
  help_button= tkinter.Button(help, text="Close", command=help.destroy)
  help_button.grid(row=1,column=0,sticky='e')

#*****************************************************
#menu is out of notebook
menu = tkinter.Menu(a)
a.config(menu=menu)

filemenu = tkinter.Menu(menu)
helpmenu = tkinter.Menu(menu)
menu.add_cascade(label="File", menu=filemenu)
menu.add_cascade(label="Help", menu=helpmenu)

filemenu.add_command(label="Exit", command=a.quit)

helpmenu.add_command(label='About',command=About)
helpmenu.add_separator()
helpmenu.add_command(label='Help',command=Help)

#root.mainloop()
n.add_screen(evil, "MUIFLI (interlaced)")
n.add_screen(root, "NUFLI (static)")

if __name__ == "__main__":
        a.mainloop()

#now clean tmp dir
print('removing "%s" again' % tmp_path)
try:
  shutil.rmtree(tmp_path)
except:
  tkinter.messagebox.showwarning('Ooops!','Something went wrong')
  infobox.destroy()


