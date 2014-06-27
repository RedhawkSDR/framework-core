import os,sys,inspect
dir = os.path.abspath(os.path.dirname(inspect.getfile(inspect.currentframe())))
sys.path.append(os.path.join(dir, "../_unitTestHelper"))
sys.path.append(os.path.join(dir, "../../base/framework/python"))
os.environ['PYTHONPATH'] = os.path.join(dir, "../../base/framework/python")
from _unitTestHelpers import scatest
os.environ['OSSIEUNITTESTSLOGCONFIG'] = os.path.join(dir, "../runtest.props")
os.environ['SDRROOT'] = os.path.join(dir, "../sdr")
# Point the SDR cache to a different location so that it's easy to clean/ignore
os.environ['SDRCACHE'] = os.path.join(os.environ['SDRROOT'], "cache")

def appendClassPath(path):
    classpath = os.environ.get('CLASSPATH', '').split(':')
    classpath.append(os.path.abspath(path))
    os.environ['CLASSPATH'] = ':'.join(classpath)

# Add Java libraries to CLASSPATH so that test components can find them
# regardless of where they run.
appendClassPath(os.path.join(dir, "../../base/framework/java/CFInterfaces.jar"))
appendClassPath(os.path.join(dir, "../../base/framework/java/apache-commons-lang-2.4.jar"))
appendClassPath(os.path.join(dir, "../../base/framework/java/log4j-1.2.15.jar"))
appendClassPath(os.path.join(dir, "../../base/framework/java/ossie/ossie.jar"))
scatest.createTestDomain()
