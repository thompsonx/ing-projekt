# ing-projekt
Parallel synchronization of tracelogs generated by applications developed in Kaira (verif.cs.vsb.cz/kaira). School project.

Requirements:
- C++11 compiler

Development notes:
- Support tracelog with pointer size 8 only

Kaira tracelog chart display bug (Ubuntu 16.04):
/usr/lib/python2.7/dist-packages/numpy/ma/core.py:2914: FutureWarning: Numpy has detected that you (may be) writing to an array returned
by numpy.diagonal or by selecting multiple fields in a structured
array. This code will likely break in a future numpy release --
see numpy.diagonal or arrays.indexing reference docs for details.
The quick fix is to make an explicit copy (e.g., do
arr.diagonal().copy() or arr[['f0','f1']].copy()).
  if (obj.__array_interface__["data"][0]
/usr/lib/python2.7/dist-packages/numpy/ma/core.py:2915: FutureWarning: Numpy has detected that you (may be) writing to an array returned
by numpy.diagonal or by selecting multiple fields in a structured
array. This code will likely break in a future numpy release --
see numpy.diagonal or arrays.indexing reference docs for details.
The quick fix is to make an explicit copy (e.g., do
arr.diagonal().copy() or arr[['f0','f1']].copy()).
  != self.__array_interface__["data"][0]):
Traceback (most recent call last):
  File "/home/tom/Projekt/kaira-main/kaira/gui/extensions.py", line 152, in <lambda>
    "clicked", lambda w: self._cb_show())
  File "/home/tom/Projekt/kaira-main/kaira/gui/extensions.py", line 200, in _cb_show
    view = type.get_view(self.source.data, self.app)
  File "/home/tom/Projekt/kaira-main/kaira/gui/datatypes.py", line 168, in tracelog_view
    return runview.RunView(app, data)
  File "/home/tom/Projekt/kaira-main/kaira/gui/runview.py", line 51, in __init__
    self.views.append(process_utilization(table, processes))
  File "/home/tom/Projekt/kaira-main/kaira/gui/runview.py", line 176, in process_utilization
    "Utilization of processes", "Time", "Process", idles))
  File "/home/tom/Projekt/kaira-main/kaira/gui/charts.py", line 862, in utilization_chart
    edgecolor='face', facecolor='#EAA769')
  File "/usr/lib/python2.7/dist-packages/matplotlib/__init__.py", line 1814, in inner
    return func(ax, *args, **kwargs)
  File "/usr/lib/python2.7/dist-packages/matplotlib/axes/_axes.py", line 2343, in broken_barh
    kwargs=kwargs)
  File "/usr/lib/python2.7/dist-packages/matplotlib/axes/_base.py", line 1910, in _process_unit_info
    self.xaxis.update_units(xdata)
  File "/usr/lib/python2.7/dist-packages/matplotlib/axis.py", line 1383, in update_units
    converter = munits.registry.get_converter(data)
  File "/usr/lib/python2.7/dist-packages/matplotlib/units.py", line 141, in get_converter
    xravel = x.ravel()
  File "/usr/lib/python2.7/dist-packages/numpy/ma/core.py", line 4326, in ravel
    r = ndarray.ravel(self._data, order=order).view(type(self))
TypeError: descriptor 'ravel' requires a 'numpy.ndarray' object but received a 'numpy.void'
