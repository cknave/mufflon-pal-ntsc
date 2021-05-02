#!/bin/bash
# Mufflon PythonGUI Launcherscript (c) DeeKay/Crest
if [ -z "`type -p python`" ]; then echo 'ERROR: Python not found! Plz install! kthxbye..'; exit 1; fi
if ! python -c "from PIL import Image" 2>/dev/null; then echo 'ERROR: Python Imaging Library (PIL) not found! Please install! MacOS 10.5-Users see here: http://tinyurl.com/mac-pil MacOS 10.6: http://tinyurl.com/mac-pil-10-6. Windows (Cygwin): http://tinyurl.com/win-pil. Linux-users please install package python-imaging-tk'; exit 1; fi
if ! python -c "from PIL import ImageTk" 2>/dev/null; then echo 'ERROR: Python-TK-bindings for PIL not found! Please install! Linux-users please install package python-imaging-tk'; exit 1; fi
if ! python -c "import _tkinter" 2>/dev/null; then echo 'ERROR: TK-bindings for Python not found! Please install! Linux-users please install package python-tk'; exit 1; fi
if [ -z "`type -p mufflongui.py`" -a ! -e mufflongui.py ]; then echo 'ERROR: Mufflon Python-GUI not found! Please put it in this dir or somewhere within your $PATH!'; exit 1; fi
if [ -z "`type -p mufflon`" -a ! -e mufflon ]; then echo 'ERROR: Mufflon binary not found! Please put it in this dir or somewhere in your $PATH!'; exit 1; fi

echo "Launching MufflonGUI...."
python mufflongui.py
