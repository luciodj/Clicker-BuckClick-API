
from setuptools import setup

APP = ['BuckGUI.py']
DATA_FILES = []
OPTIONS = {'includes':['tcl8.6']}
# OPTIONS = {}

setup(
	app = APP,
	data_files = DATA_FILES,
	options = {'p2app': OPTIONS},
	setup_requires = ['py2app'],
)
