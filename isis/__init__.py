__version__ = '0.7.0'
VERSION = tuple(map(int, __version__.split('.')))

__all__=["util","data","tools"]

def getVersion():
	return VERSION