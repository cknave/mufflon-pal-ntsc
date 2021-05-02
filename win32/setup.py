from distutils.core import setup
import py2exe

#data_files=["mufflon.exe","pythongui-about.png","cygwin1.dll"]
data_files=["mufflon.exe","pythongui-about.png","mufflon.ico"]
setup(data_files=data_files,
	windows=['mufflongui.py'],
	options = {'py2exe': {'bundle_files': 3}},
	zipfile = None,
)
