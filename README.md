# Mufflon with PAL/NTSC player

This is a fork of the [mufflon-1.0] Commodore 64 image converter with some changes:

1. Replace the PAL-only displayer code with the PAL/NTSC displayer from [NUFLI Editor 1.12]
2. Update python GUI to python 3
3. Remove GUI wrappers (shell/win32)

[mufflon-1.0]: https://twinbirds.com/mufflon/
[NUFLI Editor 1.12]: https://csdb.dk/release/?id=95473&show=notes


## Building

```
make
```

## GUI

Set up a virtualenv:
```
python3 -mvenv .venv
.venv/bin/pip install -r requirements.txt
```

Use it to run the gui:
```
.venv/bin/python mufflongui.py
```
