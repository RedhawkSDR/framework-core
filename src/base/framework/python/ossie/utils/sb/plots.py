import logging
import threading
import time

from PyQt4 import QtCore, QtGui

import matplotlib
matplotlib.use('Qt4Agg')
from matplotlib import pyplot
from matplotlib import mlab
from matplotlib.backends.backend_agg import FigureCanvasAgg
import numpy

from bulkio.bulkioInterfaces import BULKIO__POA
from ossie.utils.bulkio import bulkio_data_helpers
from ossie.utils.model import PortSupplier
from ossie.utils.model.connect import PortEndpoint

from io_helpers import helperBase

__all__ = ('LinePlot', 'LinePSD', 'RasterPlot', 'RasterPSD', 'XYPlot')

log = logging.getLogger(__name__)

class PlotSink(bulkio_data_helpers.ArraySink):
    """
    Helper sink subclass that discards data when it is not started, so that
    plots do not increase memory use unbounded if they are not running.
    """
    def __init__(self, porttype):
        super(PlotSink,self).__init__(porttype)
        self._started = False

    def start(self):
        self._started = True
        super(PlotSink,self).start()

    def stop(self):
        super(PlotSink,self).stop()
        self._started = False
        self.data = []

    def pushPacket(self, data, ts, EOS, stream_id):
        if not self._started:
            return
        super(PlotSink,self).pushPacket(data, ts, EOS, stream_id)


class PlotBase(helperBase, PortSupplier):
    """
    Abstract base class for all matplotlib-based plots. Manages the provides
    port dictionary, the matplotlib figure, and the plot update thread.
    """
    def __init__(self):
        helperBase.__init__(self)
        PortSupplier.__init__(self)

        # Create provides port dictionary.
        self._providesPortDict = {}
        for interface in ('Char', 'Short', 'Long', 'Float', 'Ulong',
                          'Double', 'LongLong', 'Octet', 'UlongLong',
                          'Ushort'):
            name = '%sIn' % interface.lower()
            self._providesPortDict[name] = {
                'Port Interface': 'IDL:BULKIO/data%s:1.0' % interface,
                'Port Name': name
                }

        self.breakBlock = False
        self._thread = None

        # Create a new figure and axes.
        self._figure = pyplot.figure()
        self._plot = self._figure.add_subplot(1, 1, 1)
        self._canvas = self._figure.canvas
        self._figure.show(False)

        # Let subclasses set up the axes.
        self._configureAxes()

        # Matplotlib 0.99 does not give an easy way to listen for window close
        # events; as a result, we have to get the underlying backend-specific
        # window. This is course, somewhat fragile. Furthermore, QMainWindow
        # does not provide a signal on close that would allow us to stop the
        # rendering loop before the objects are destroyed, so we dynamically
        # override the closeEvent method on this particular instance.
        window = self._getWindow()
        oldClose = window.closeEvent
        def closeEvent(event):
            self.stop()
            oldClose(event)
        window.closeEvent = closeEvent

        # Add thread synchronization to the draw and paintEvent methods on
        # FigureCanvasQTAgg, so that we can do drawing updates in the reader
        # thread at the same time that the main thread might traw to redraw.
        # Without synchronization, this leads to exceptions and flickering.
        lock = threading.Lock()
        oldDraw = self._canvas.draw
        def draw():
            lock.acquire()
            try:
                oldDraw()
            finally:
                lock.release()
        self._canvas.draw = draw
        oldPaint = self._canvas.paintEvent
        def paintEvent(e):
            lock.acquire()
            try:
                oldPaint(e)
            finally:
                lock.release()
        self._canvas.paintEvent = paintEvent
        self._renderLock = lock

    def _getWindow(self):
        for manager in pyplot._pylab_helpers.Gcf.get_all_fig_managers():
            if manager.num == self._figure.number:
                return manager.window
        return None

    def _createSink(self, interface):
        # Create the type-specific sink servant.
        interface = interface.split(':')[1]
        namespace, interface = interface.split('/')
        skeleton = getattr(BULKIO__POA, interface)
        sink = PlotSink(skeleton)
        if self._thread:
            # Plot is started; start sink.
            sink.start()
        return sink

    def _redraw(self):
        """
        Redraw the plot and ask the widget to display the update.
        """
        # Perform the redraw with the render lock held, preventing the graphics
        # thread from trying to do a redraw at the same time.
        self._renderLock.acquire()
        try:
            # NB: In matplotlib 0.99, if you call the canvas' draw() method
            #     directly, it immediately redraws the canvas, but sets replot
            #     to true, leading to another redraw when the update is handled
            #     by the graphics thread.
            FigureCanvasAgg.draw(self._canvas)
            self._canvas.replot = False
            self._canvas.update()
        finally:
            self._renderLock.release()

    def _run(self):
        """
        Main method for plot update thread; do not call directly.
        """
        while not self.breakBlock:
            # Continually update the plot, unless there is nothing to do (i.e.,
            # not connected to a data source).
            if self._update():
                self._redraw()
            else:
                time.sleep(0.1)

    def start(self):
        """
        Start the plot, if it is not already started. The plot will continually
        read input data and refresh the display until it is stopped.
        """
        self.breakBlock = False
        if self._thread:
            return
        self._thread = threading.Thread(target=self._run)
        self._thread.setDaemon(True)
        self._thread.start()

    def stop(self):
        """
        Stop the plot. The plot will discontinue reading input data and updating
        the display.
        """
        self.breakBlock = True
        if self._thread:
            self._thread.join()
            self._thread = None


class PlotEndpoint(PortEndpoint):
    def __init__(self, plot, port, connectionId):
        super(PlotEndpoint,self).__init__(plot, port)
        self.connectionId = connectionId

    def getReference(self):
        try:
            self.supplier._addTrace(self.port, self.connectionId)
        except KeyError:
            pass
        portname = '%s-%s' % (self.port['Port Name'], self.connectionId)
        return self.supplier.getPort(portname)


class LineBase(PlotBase):
    """
    Abstract base class for line plots, supporting multiple traces on the same
    plot. Each connection creates a new port to enable this behavior, although
    this behavior should be transparent to the sandbox user.
    """
    def __init__(self, frameSize, ymin, ymax):
        PlotBase.__init__(self)
        self._frameSize = frameSize
        self._ymin = ymin
        self._ymax = ymax
        self._plot.set_ylim(self._ymin, self._ymax)

        # Line state
        self._lines = {}
        self._linesLock = threading.Lock()

    def _getEndpoint(self, port, connectionId):
        if not port['Port Name'] in self._providesPortDict:
            raise RuntimeError, "Line plot '%s' has no provides port '%s'", (self._instanceName, name)
        return PlotEndpoint(self, port, connectionId)

    def _disconnected(self, connectionId):
        self._linesLock.acquire()
        try:
            for name, trace in self._lines.iteritems():
                if trace['id'] == connectionId:
                    trace['sink'].stop()
                    line = trace['line']
                    line.remove()
                    del self._lines[name]
                    return
        finally:
            self._linesLock.release()

    def getPort(self, name):
        self._linesLock.acquire()
        try:
            sink = self._lines[name]['sink']
            return sink.getPort()
        finally:
            self._linesLock.release()

    def _lineOptions(self):
        return {}

    def _addTrace(self, port, name):
        self._linesLock.acquire()
        try:
            traceName = '%s-%s' % (port['Port Name'], name)
            if traceName in self._lines:
                raise KeyError, "Trace '%s' already exists" % traceName

            sink = self._createSink(port['Port Interface'])
            options = self._lineOptions()
            line, = self._plot.plot([], [], label=name, scalex=False, scaley=False, **options)
            trace = { 'sink':   sink,
                      'xdelta': None,
                      'line':   line,
                      'id':     name }
            self._lines[traceName] = trace
        finally:
            self._linesLock.release()
 
    def _updateTrace(self, trace):
        sink = trace['sink']
        line = trace['line']

        # Read next frame.
        data, timestamps = sink.retrieveData(length=self._frameSize)
        x_data, y_data = self._formatData(data, sink.sri)
        line.set_data(x_data, y_data)

        # Check for new xdelta and update canvas if necessary.
        if sink.sri.xdelta != trace['xdelta']:
            trace['xdelta'] = sink.sri.xdelta
            xmin, xmax = self._getXRange(sink.sri.xdelta)
            self._plot.set_xlim(xmin, xmax)

    def _update(self):
        # Get a copy of the current set of lines to update, then release the
        # lock to do the reads. This allows the read to be interrupted (e.g.,
        # if a source is disconnected) without deadlock.
        self._linesLock.acquire()
        try:
            traces = self._lines.values()
        finally:
            self._linesLock.release()
        for trace in traces:
            self._updateTrace(trace)

        if self._ymin is None or self._ymax is None:
            self._plot.relim()
            if self._ymin is not None:
                ymin = self._ymin
            else:
                ymin = self._plot.dataLim.y0
            if self._ymax is not None:
                ymax = self._ymax
            else:
                ymax = self._plot.dataLim.y1
            # Update plot scale.
            self._plot.set_ylim(ymin, ymax)

        return True

    def start(self):
        log.debug("Starting line plot '%s'", self._instanceName)
        super(LineBase,self).start()
        # Start all associated sinks.
        self._linesLock.acquire()
        try:
            for trace in self._lines.itervalues():
                trace['sink'].start()
        finally:
            self._linesLock.release()
    start.__doc__ = PlotBase.start.__doc__

    def stop(self):
        log.debug("Stopping line plot '%s'", self._instanceName)
        # Stop all associated sinks.
        self._linesLock.acquire()
        try:
            for trace in self._lines.itervalues():
                trace['sink'].stop()
        finally:
            self._linesLock.release()
        super(LineBase,self).stop()
    stop.__doc__ = PlotBase.stop.__doc__

    # Plot settings
    def get_ymin(self):
        """
        The lower bound of the Y-axis. If set to None, the lower bound will be
        determined automatically per-frame based on the data.
        """
        return self._ymin

    def set_ymin(self, ymin):
        self._check_yrange(ymin, self._ymax)
        self._ymin = ymin
        self._plot.set_ylim(self._ymin, self._ymax)

    ymin = property(get_ymin, set_ymin)
    del get_ymin, set_ymin

    def get_ymax(self):
        """
        The upper bound of the Y-axis. If set to None, the upper bound will be
        determined automatically per-frame based on the data.
        """
        return self._ymax

    def _check_yrange(self, ymin, ymax):
        if ymin is None or ymax is None:
            return
        if ymax < ymin:
            raise ValueError, 'Y-axis bounds cannot overlap (%d > %d)' % (ymin, ymax)

    def set_ymax(self, ymax):
        self._check_yrange(self._ymin, ymax)
        self._ymax = ymax
        self._plot.set_ylim(self._ymin, self._ymax)

    ymax = property(get_ymax, set_ymax)
    del get_ymax, set_ymax


class LinePlot(LineBase):
    """
    Line plot for time data, supporting multiple input sources. Each connection
    is drawn as its own colored line, and lines can be dynamically created or
    removed.
    """
    def __init__(self, frameSize=1024, ymin=None, ymax=None):
        """
        Create a new line plot.

        Arguments:
          frameSize  - Number of elements to draw per frame.
          ymin, ymax - Y-axis constraints. If not given, constraint will be
                       automatically calculated per-frame based on the data.
        """
        super(LinePlot,self).__init__(frameSize, ymin, ymax)

    def _configureAxes(self):
        self._plot.xaxis.set_label_text('Time (s)')

    def _formatData(self, data, sri):
        xdelta = sri.xdelta
        times = numpy.arange(len(data)) * xdelta
        return times, data

    def _getXRange(self, xdelta):
        return 0.0, (self._frameSize-1)*xdelta


class PSDBase(object):
    """
    Mix-in class for plots that the power spectral density (PSD) of input
    signal(s).
    """
    def __init__(self, nfft):
        self._nfft = nfft

    def _psd(self, data):
        return mlab.psd(data, NFFT=self._nfft)


class LinePSD(LineBase, PSDBase):
    """
    Line plot for displaying the power spectral density (PSD) of one or more
    input signals. Input data is padded to the FFT size, if necessary, and
    a Hanning window is applied before calculating the PSD.

    The Y-axis (magnitude) is displayed using a logarithmic scale.
    """
    def __init__(self, nfft=1024, frameSize=None, ymin=None, ymax=None):
        """
        Create a new line PSD plot.

        Arguments:

          nfft       - FFT size
          frameSize  - Number of elements to process per frame; if not given,
                       defaults to the FFT size. Must be less than or equal to
                       FFT size.
          ymin, ymax - Y-axis constraints. If not given, constraint will be
                       automatically calculated per-frame based on the data.
        """
        if not frameSize:
            frameSize = nfft
        LineBase.__init__(self, frameSize, ymin, ymax)
        PSDBase.__init__(self, nfft)

    def _configureAxes(self):
        self._plot.xaxis.set_label_text('Frequency (Hz)')
        self._plot.set_yscale('log')

    def _formatData(self, data, sri):
        # Calculate PSD of input data.
        data, freqs = self._psd(data)

        # Convert frequencies from [0.0, 1.0] interval to Hz.
        nyquist = 0.5/sri.xdelta
        freqs = freqs * nyquist

        # Return x data (frequencies) and y data (magnitudes)
        return freqs, data.reshape(len(data))

    def _getXRange(self, xdelta):
        # Round Nyquist frequency to an integral value to account for the fact
        # that there is typically some rounding error in the xdelta value.
        nyquist = round(0.5/xdelta)
        return 0.0, nyquist


class RasterBase(PlotBase):
    """
    Abstract base class for falling raster plots. Time is displayed on the
    Y-axis, while magnitude (Z-axis) is displayed using a color map. The
    meaning of the X-axis varies depending on the data being displayed.
    """
    def __init__(self, frameSize=1024, imageWidth=1024, imageHeight=1024, zmin=-1, zmax=1, defaultValue=0.0):
        PlotBase.__init__(self)

        self._frameSize = frameSize
        self._sink = None

        # Raster state
        self._imageData = numpy.ones((imageHeight, imageWidth)) * defaultValue
        self._image = self._plot.imshow(self._imageData, extent=(0, 1, 1, 0))
        self._zmin = zmin
        self._zmax = zmax
        norm = self._getNorm(self._zmin, self._zmax)
        self._image.set_norm(norm)
        self._row = 0
        self._xdelta = None

        # Maintain aspect ratio of image
        self._aspect = float(imageHeight)/imageWidth
        self._plot.set_aspect(self._aspect)

        # Add a colorbar
        self._colorbar = self._figure.colorbar(self._image)

    def _formatData(self, data):
        return data

    def getPort(self, name):
        if self._sink:
            if name != self._sinkName:
                raise RuntimeError, "Raster plot only supports one port at a time (using '%s')" % self._sinkName
        else:
            port = self._providesPortDict[name]
            self._sink = self._createSink(port['Port Interface'])
            self._sinkName = name
        return self._sink.getPort()

    def _update(self):
        if not self._sink:
            return False

        # Read and format data.
        data, timestamps = self._sink.retrieveData(length=self._frameSize)
        data = self._formatData(data)

        # If xdelta changes, update the X and Y ranges.
        sri = self._sink.sri
        if sri.xdelta != self._xdelta:
            x_range, y_range = self._sriChanged(sri)
            self._image.set_extent((0, y_range, x_range, 0))
            self._plot.set_aspect(y_range/x_range*self._aspect)
            self._xdelta = sri.xdelta

        # Resample data from frame size to image size.
        height, width = self._imageData.shape
        indices_out = numpy.linspace(0, len(data)-1, width)
        indices_in = numpy.arange(len(data))
        data = numpy.interp(indices_out, indices_in, data)

        # Store the new row and update the image data.
        self._imageData[self._row] = data
        self._image.set_array(self._imageData)

        # Advance row pointer
        self._row = (self._row + 1) % height

        return True

    def start(self):
        log.debug("Starting raster plot '%s'", self._instanceName)
        if self._sink:
            self._sink.start()
        super(RasterBase,self).start()
    start.__doc__ = PlotBase.start.__doc__

    def stop(self):
        log.debug("Stopping raster plot '%s'", self._instanceName)
        if self._sink:
            self._sink.stop()
        super(RasterBase,self).stop()
    stop.__doc__ = PlotBase.stop.__doc__

    # Plot settings
    def _check_zrange(self, zmin, zmax):
        if zmax < zmin:
            raise ValueError, 'Z-axis bounds cannot overlap (%d > %d)' % (zmin, zmax)

    def get_zmin(self):
        """
        The lower bound of the Z-axis.
        """
        return self._zmin

    def set_zmin(self, zmin):
        self._check_zrange(zmin, self._zmax)
        self._zmin = zmin
        norm = self._getNorm(self._zmin, self._zmax)
        self._image.set_norm(norm)

    zmin = property(get_zmin, set_zmin)
    del get_zmin, set_zmin

    def get_zmax(self):
        """
        The upper bound of the Z-axis.
        """
        return self._zmax

    def set_zmax(self, zmax):
        self._check_zrange(self._zmin, zmax)
        self._zmax = zmax
        norm = self._getNorm(self._zmin, self._zmax)
        self._image.set_norm(norm)

    zmax = property(get_zmax, set_zmax)
    del get_zmax, set_zmax


class RasterPlot(RasterBase):
    """
    Falling raster plot of time data. The Y-axis represents inter-frame time,
    while the X-axis represents intra-frame time. The Z-axis, mapped to a color
    range, represents the magnitude of each sample.
    """
    def __init__(self, frameSize=1024, imageWidth=1024, imageHeight=1024, zmin=-1.0, zmax=1.0):
        """
        Create a new raster plot.

        Arguments:

          frameSize   - Number of elements to draw per line
          imageWidth  - Width of the backing image in pixels
          imageHeight - Height of the backing image in pixels
          zmin, zmax  - Z-axis (magnitude) constraints. Data is clamped to the
                        range [zmin, zmax].

        If the frame size is not equal to the image width, the input line will
        be linearly resampled to the image width.
        """
        super(RasterPlot,self).__init__(frameSize, imageWidth, imageHeight, zmin, zmax, defaultValue=zmin)

    def _getNorm(self, zmin, zmax):
        return matplotlib.colors.Normalize(zmin, zmax)

    def _configureAxes(self):
        self._plot.xaxis.set_label_text('Time offset (s)')
        self._plot.yaxis.set_label_text('Time (s)')

    def _sriChanged(self, sri):
        # Y range is time per line.
        y_range = sri.xdelta * self._frameSize

        # X range is the total time across all lines.
        height, width = self._imageData.shape
        x_range = height*y_range

        return x_range, y_range


class RasterPSD(RasterBase, PSDBase):
    """
    Falling raster plot for displaying the power spectral density (PSD) of an
    input signal. Input data is padded to the FFT size, if necessary, and a
    Hanning window is applied before calculating the PSD.

    The Y-axis represents inter-frame time,  while the X-axis represents
    frequency.  The Z-axis, mapped to a color range, represents the magnitude
    of each frequency.

    The Z-axis (magnitude) is displayed using a logarithmic scale.
    """
    def __init__(self, nfft=1024, frameSize=None, imageWidth=1024, imageHeight=1024, zmin=1.0e-16, zmax=1.0):
        """
        Create a new raster PSD plot.

        Arguments:

          nfft        - FFT size
          frameSize   - Number of elements to process per line; if not given,
                        defaults to the FFT size. Must be less than or equal to
                        FFT size.
          imageWidth  - Width of the backing image in pixels
          imageHeight - Height of the backing image in pixels
          zmin, zmax  - Z-axis (magnitude) constraints. Data is clamped to the
                        range [zmin, zmax].

        If the size of the PSD output (nfft/2+1) is not equal to the image
        width, the PSD output will be linearly resampled to the image width.
        """
        if not frameSize:
            frameSize = nfft
        RasterBase.__init__(self, frameSize, imageWidth, imageHeight, zmin, zmax, defaultValue=zmin)
        PSDBase.__init__(self, nfft)

    def _getNorm(self, zmin, zmax):
        return matplotlib.colors.LogNorm(zmin, zmax)

    def _configureAxes(self):
        self._plot.xaxis.set_label_text('Frequency (Hz)')
        self._plot.yaxis.set_label_text('Time (s)')

    def _sriChanged(self, sri):
        # X range is the total time across all lines.
        dtime = sri.xdelta * self._nfft
        height, width = self._imageData.shape
        x_range = height*dtime

        # Y range is [0, nyquist).
        if sri.xdelta > 0.0:
            y_range = round(0.5 / sri.xdelta)
        else:
            # Bad SRI xdelta, use [0, 1]
            y_range = 1.0
        
        return x_range, y_range

    def _formatData(self, data):
        # Calculate PSD of input data.
        data, freqs = self._psd(data)
        return data.reshape(len(data))


class XYPlot(LineBase):
    """
    Scatter plot for displaying complex pairs as X/Y coordinates. The real
    component is plotted on the X-axis, while the imaginary component is
    plotted on the Y-axis.
    """
    def __init__(self, frameSize=1024, xmin=-1.0, xmax=1.0, ymin=-1.0, ymax=1.0):
        """
        Create a new X/Y plot.

        Arguments:

          frameSize  - Number of complex values to draw per frame.
          xmin, xmax - X-axis constraints.
          ymin, ymax - Y-axis constraints.
        """
        LineBase.__init__(self, frameSize*2, ymin, ymax)
        self._xmin = xmin
        self._xmax = xmax
        self._plot.set_xlim(self._xmin, self._xmax)

        # Enable gridlines and draw X-axis and Y-axis through the origin
        self._plot.grid(True)
        self._plot.axhline(0, color='black')
        self._plot.axvline(0, color='black')

    def _configureAxes(self):
        self._plot.xaxis.set_label_text('Real')
        self._plot.yaxis.set_label_text('Imaginary')

    def _formatData(self, data, sri):
        return data[::2], data[1::2]

    def _getXRange(self, xdelta):
        return self._xmin, self._xmax

    def _lineOptions(self):
        return {'marker': 'o', 'linestyle': 'None'}

    def _check_xrange(self, xmin, xmax):
        if xmin is None or xmax is None:
            return
        if xmax < xmin:
            raise ValueError, 'X-axis bounds cannot overlap (%f > %f)' % (xmin, xmax)

    # Plot properties
    def get_xmin(self):
        """
        The lower bound of the X-axis.
        """
        return self._xmin

    def set_xmin(self, xmin):
        self._check_yrange(xmin, self._xmax)
        self._xmin = xmin
        self._plot.set_xlim(self._xmin, self._xmax)

    xmin = property(get_xmin, set_xmin)
    del get_xmin, set_xmin

    def get_xmax(self):
        """
        The upper bound of the X-axis.
        """
        return self._xmax

    def set_xmax(self, xmax):
        self._check_xrange(self._xmin, xmax)
        self._xmax = xmax
        self._plot.set_xlim(self._xmin, self._xmax)

    xmax = property(get_xmax, set_xmax)
    del get_xmax, set_xmax
